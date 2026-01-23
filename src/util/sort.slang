// translated into slang from https://github.com/MircoWerner/VkRadixSort/blob/main/singleradixsort/resources/shaders/single_radixsort.comp

import canta;

#define WORKGROUP_SIZE 256
GROUP_SIZE(WORKGROUP_SIZE, 1, 1)
[vk::constant_id(SPEC_CONSTANT_INDEX(0))] const uint typeSize = sizeof(uint);

#define radixSortBins 256
#define subGroupSize 64 // 32 for nvidia and 64 for amd
#define iterations 4

groupshared uint[radixSortBins] histogram;
groupshared uint[radixSortBins / subGroupSize] sums;
groupshared uint[radixSortBins] localOffsets;
groupshared uint[radixSortBins] globalOffsets;

struct BinFlags {
    uint flags[WORKGROUP_SIZE / 32];
};
groupshared BinFlags[radixSortBins] binFlags;

void setValue(uint* dst, uint* src, uint srcIndex, uint dstIndex) {
    let count = typeSize / sizeof(uint);
    for (uint i = 0; i < count; i++) {
        let d = dstIndex * count + i;
        let s = srcIndex * count + i;
        dst[d] = src[s];
    }
}

void swapValue(uint* values, uint* tmpValues, uint srcIndex, uint dstIndex, uint iteration) {
    if (iteration % 2 == 0) {
        setValue(tmpValues, values, srcIndex, dstIndex);
    } else {
        setValue(values, tmpValues, srcIndex, dstIndex);
    }
}

uint getKey(uint* keys, uint* tmpKeys, uint index, uint iteration) {
    if (iteration % 2 == 0) {
        return keys[index];
    } else {
        return tmpKeys[index];
    }
}

void setKey(uint* keys, uint* tmpKeys, uint index, uint iteration, uint key) {
    if (iteration % 2 == 0) {
        tmpKeys[index] = key;
    } else {
        keys[index] = key;
    }
}

[shader("compute")]
NUM_THREADS
void main(
    uint3 groupThread : SV_GroupThreadID,
    uint3 groupId : SV_GroupID,
    uniform uint* keys,
    uniform uint* tmpKeys,
    uniform uint* values,
    uniform uint* tmpValues,
    uniform uint numElements,
) {
    let waveIndex = groupThread.x / WaveGetLaneCount();
    let waveId = WaveGetLaneIndex();
    let id = groupThread.x;

    for (uint iteration = 0; iteration < iterations; iteration++) {
        uint shift = 8 * iteration;
        if (id < radixSortBins) {
            histogram[id] = 0;
        }
        AllMemoryBarrierWithGroupSync();

        for (uint ID = id; ID < numElements; ID += WORKGROUP_SIZE) {
            const uint bin = uint(getKey(keys, tmpKeys, ID, iteration) >> shift) & uint(radixSortBins - 1);
            InterlockedAdd(histogram[bin], 1);
        }
        AllMemoryBarrierWithGroupSync();

        if (id < radixSortBins) {
            uint histogramCount = histogram[id];
            uint sum = WaveActiveSum(histogramCount);
            uint prefixSum = WavePrefixSum(histogramCount);
            localOffsets[id] = prefixSum;
            if (WaveIsFirstLane()) {
                sums[waveIndex] = sum;
            }
        }
        AllMemoryBarrierWithGroupSync();

        if (waveIndex == 0) {
            uint offset = 0;
            for (uint i = waveId; i < radixSortBins; i += WaveGetLaneCount()) {
                globalOffsets[i] = offset + localOffsets[i];
                offset += sums[i / WaveGetLaneCount()];
            }
        }
        AllMemoryBarrierWithGroupSync();

        const uint flagsBin = id / 32;
        const uint flagsBit = 1 << (id % 32);

        for (uint blockId = 0; blockId < numElements; blockId += WORKGROUP_SIZE) {
            AllMemoryBarrierWithGroupSync();

            const uint ID = blockId + id;

            if (id < radixSortBins) {
                for (int i = 0; i < WORKGROUP_SIZE / 32; i++) {
                    binFlags[id].flags[i] = 0;
                }
            }
            AllMemoryBarrierWithGroupSync();

            uint key = 0;
            uint value = 0;
            uint binId = 0;
            uint binOffset = 0;

            if (ID < numElements) {
                key = getKey(keys, tmpKeys, ID, iteration);
                binId = uint(key >> shift) & uint(radixSortBins - 1);
                binOffset = globalOffsets[binId];
                InterlockedAdd(binFlags[binId].flags[flagsBin], flagsBit);
            }
            AllMemoryBarrierWithGroupSync();

            if (ID < numElements) {
                uint prefix = 0;
                uint count = 0;
                for (uint i = 0; i < WORKGROUP_SIZE / 32; i++) {
                    const uint bits = binFlags[binId].flags[i];
                    const uint fullCount = countbits(bits);
                    const uint partialCount = countbits(bits & (flagsBit - 1));
                    prefix += (i < flagsBin) ? fullCount : 0;
                    prefix += (i == flagsBin) ? partialCount : 0;
                    count += fullCount;
                }

                if (binOffset + prefix < numElements) {
                    setKey(keys, tmpKeys, binOffset + prefix, iteration, key);
                    swapValue(values, tmpValues, ID, binOffset + prefix, iteration);
                }
                if (prefix == count - 1) {
                    InterlockedAdd(globalOffsets[binId], count);
                }
            }
        }
    }
}