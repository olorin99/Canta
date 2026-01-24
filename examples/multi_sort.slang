import canta;

#define WORKGROUP_SIZE 256
GROUP_SIZE(WORKGROUP_SIZE, 1, 1)

#define radixSortBins 256
#define subGroupSize 64
#define iterations 4

groupshared uint[radixSortBins / subGroupSize] sums;
groupshared uint[radixSortBins] globalOffsets;

struct BinFlags {
    uint flags[WORKGROUP_SIZE / 32];
};
groupshared BinFlags[radixSortBins] binFlags;

[shader("compute")]
NUM_THREADS
void main(
    uint3 threadId : SV_DispatchThreadID,
    uint3 groupThread : SV_GroupThreadID,
    uint3 groupId : SV_GroupID,
    uniform uint* elementsIn,
    uniform uint* elementsOut,
    uniform uint* histograms,
    uniform uint numElements,
    uniform uint shift,
    uniform uint numWorkgroups,
    uniform uint numBlocksPerWorkgroup
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
        uint elementId = wid * numBlocksPerWorkgroup * WORKGROUP_SIZE + index * WORKGROUP_SIZE + lid;

        if (lid < radixSortBins) {
            for (int i = 0; i < WORKGROUP_SIZE / 32; i++) {
                binFlags[lid].flags[i] = 0;
            }
        }
        AllMemoryBarrierWithGroupSync();

        uint elementIn = 0;
        uint binId = 0;
        uint binOffset = 0;
        if (elementId < numElements) {
            elementIn = elementsIn[elementId];
            binId = uint(elementIn >> shift) & uint(radixSortBins - 1);
            binOffset = globalOffsets[binId];
            InterlockedAdd(binFlags[binId].flags[flagsBin], flagsBit);
        }
        AllMemoryBarrierWithGroupSync();

        if (elementId < numElements) {
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
            //elementsOut[gid] = localHistogram;
            elementsOut[binOffset + prefix] = elementIn;
            if (prefix == count - 1) {
                InterlockedAdd(globalOffsets[binId], count);
            }
        }
        AllMemoryBarrierWithGroupSync();
    }
}