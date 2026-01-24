import canta;

#define WORKGROUP_SIZE 256
GROUP_SIZE(WORKGROUP_SIZE, 1, 1)

#define radixSortBins 256

groupshared uint[radixSortBins] histogram;

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

    if (lid < radixSortBins) {
        histogram[lid] = 0;
    }
    AllMemoryBarrierWithGroupSync();

    for (uint index = 0; index < numBlocksPerWorkgroup; index++) {
        uint elementId = wid * numBlocksPerWorkgroup * WORKGROUP_SIZE + index * WORKGROUP_SIZE + lid;
        if (elementId < numElements) {
            const uint bin = uint(elementsIn[elementId] >> shift) & uint(radixSortBins - 1);
            InterlockedAdd(histogram[bin], 1);
        }
    }
    AllMemoryBarrierWithGroupSync();

    if (lid < radixSortBins) {
        histograms[radixSortBins * wid + lid] = histogram[lid];
    }
}