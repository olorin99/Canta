import canta;

#define WORKGROUP_SIZE 256
[vk::constant_id(0)] const uint x_size = WORKGROUP_SIZE;
[vk::constant_id(1)] const uint y_size = 1;
[vk::constant_id(2)] const uint z_size = 1;
static const uint3 groupSize = uint3(x_size, y_size, z_size);

#define radixSortBins 256
#define subGroupSize 64

groupshared uint[radixSortBins / subGroupSize] sums;
groupshared uint[radixSortBins] globalOffsets;

struct BinFlags {
    uint flags[WORKGROUP_SIZE / 32];
};
groupshared BinFlags[radixSortBins] binFlags;

void setValue(uint* dst, uint* src, uint srcIndex, uint dstIndex, uint typeSize) {
    let count = typeSize / sizeof(uint);
    for (uint i = 0; i < count; i++) {
        let d = dstIndex * count + i;
        let s = srcIndex * count + i;
        dst[d] = src[s];
    }
};

[shader("compute")]
[numthreads(x_size, y_size, z_size)]
void main(
    uint3 threadId : SV_DispatchThreadID,
    uint3 groupThread : SV_GroupThreadID,
    uint3 groupId : SV_GroupID,
    uniform uint* keys,
    uniform uint* tmpKeys,
    uniform uint* values,
    uniform uint* tmpValues,
    uniform uint* histograms,
    uniform uint numElements,
    uniform uint shift,
    uniform uint numWorkgroups,
    uniform uint numBlocksPerWorkgroup,
    uniform uint typeSize,
) {
    uint gid = threadId.x;
    uint lid = groupThread.x;
    uint wid = groupId.x;
    uint sid = groupThread.x / WaveGetLaneCount();
    uint lsid = WaveGetLaneIndex();

    uint localHistogram = 0;
    uint prefixSum = 0;
    uint histogramCount = 0;

    if (lid < radixSortBins) {
        uint count = 0;
        for (uint i = 0; i < numWorkgroups; i++) {
            const uint t = histograms[radixSortBins * i + lid];
            localHistogram = (i == wid) ? count : localHistogram;
            count += t;
        }
        histogramCount = count;
        const uint sum = WaveActiveSum(histogramCount);
        prefixSum = WavePrefixSum(histogramCount);
        if (WaveIsFirstLane()) {
            sums[sid] = sum;
        }
    }
    AllMemoryBarrierWithGroupSync();

    if (lid < radixSortBins) {
        const uint sumsPrefixSum = WaveReadLaneAt(WavePrefixSum(sums[lsid]), sid);
        const uint globalHistogram = sumsPrefixSum + prefixSum;
        globalOffsets[lid] = globalHistogram + localHistogram;
    }

    const uint flagsBin = lid / 32;
    const uint flagsBit = 1 << (lid % 32);

    for (uint index = 0; index < numBlocksPerWorkgroup; index++) {
        uint keyId = wid * numBlocksPerWorkgroup * WORKGROUP_SIZE + index * WORKGROUP_SIZE + lid;

        if (lid < radixSortBins) {
            for (int i = 0; i < WORKGROUP_SIZE / 32; i++) {
                binFlags[lid].flags[i] = 0;
            }
        }
        AllMemoryBarrierWithGroupSync();

        uint key = 0;
        uint binId = 0;
        uint binOffset = 0;
        if (keyId < numElements) {
            key = keys[keyId];
            binId = uint(key >> shift) & uint(radixSortBins - 1);
            binOffset = globalOffsets[binId];
            InterlockedAdd(binFlags[binId].flags[flagsBin], flagsBit);
        }
        AllMemoryBarrierWithGroupSync();

        if (keyId < numElements) {
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
            tmpKeys[binOffset + prefix] = key;
            setValue(tmpValues, values, keyId, binOffset + prefix, typeSize);
            if (prefix == count - 1) {
                InterlockedAdd(globalOffsets[binId], count);
            }
        }
        AllMemoryBarrierWithGroupSync();
    }
}