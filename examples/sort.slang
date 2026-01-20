// translated into slang from https://github.com/MircoWerner/VkRadixSort/blob/main/singleradixsort/resources/shaders/single_radixsort.comp

import canta;

#define WORKGROUP_SIZE 256
GROUP_SIZE(WORKGROUP_SIZE, 1, 1)

#define radixSortBins 256
#define subGroupSize 64
#define iterations 4

#define ELEMENT_IN(index, iteration) (iteration % 2 == 0 ? elementsIn[index] : elementsOut[index])

groupshared uint[radixSortBins] histogram;
groupshared uint[radixSortBins / subGroupSize] sums;
groupshared uint[radixSortBins] localOffsets;
groupshared uint[radixSortBins] globalOffsets;

struct BinFlags {
    uint flags[WORKGROUP_SIZE / 32];
};
groupshared BinFlags[radixSortBins] binFlags;

[shader("compute")]
NUM_THREADS
void main(
    uint3 groupThread : SV_GroupThreadID,
    uint3 groupId : SV_GroupID,
    uniform uint* elementsIn,
    uniform uint* elementsOut,
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
            const uint bin = uint(ELEMENT_IN(ID, iteration) >> shift) & uint(radixSortBins - 1);
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
            for (uint i = waveId; i < radixSortBins; i += subGroupSize) {
                globalOffsets[i] = offset + localOffsets[i];
                offset += sums[i / subGroupSize];
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

            uint elementIn = 0;
            uint binId = 0;
            uint binOffset = 0;

            if (ID < numElements) {
                elementIn = ELEMENT_IN(ID, iteration);
                binId = uint(elementIn >> shift) & uint(radixSortBins - 1);
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

                if (iteration % 2 == 0) {
                    if (binOffset + prefix < numElements) {
                        elementsOut[binOffset + prefix] = elementIn;
                    }
                } else {
                    if (binOffset + prefix < numElements) {
                        elementsIn[binOffset + prefix] = elementIn;
                    }
                }
                if (prefix == count - 1) {
                    InterlockedAdd(globalOffsets[binId], count);
                }
            }
        }
    }
}