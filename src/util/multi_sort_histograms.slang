import canta;

#define WORKGROUP_SIZE 256
[vk::constant_id(0)] const uint x_size = WORKGROUP_SIZE;
[vk::constant_id(1)] const uint y_size = 1;
[vk::constant_id(2)] const uint z_size = 1;
static const uint3 groupSize = uint3(x_size, y_size, z_size);

#define radixSortBins 256

groupshared uint[radixSortBins] histogram;

[shader("compute")]
[numthreads(x_size, y_size, z_size)]
void main(
    uint3 threadId : SV_DispatchThreadID,
    uint3 groupThread : SV_GroupThreadID,
    uint3 groupId : SV_GroupID,
    uniform uint* keys,
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
        uint keyId = wid * numBlocksPerWorkgroup * WORKGROUP_SIZE + index * WORKGROUP_SIZE + lid;
        if (keyId < numElements) {
            const uint bin = uint(keys[keyId] >> shift) & uint(radixSortBins - 1);
            InterlockedAdd(histogram[bin], 1);
        }
    }
    AllMemoryBarrierWithGroupSync();

    if (lid < radixSortBins) {
        histograms[radixSortBins * wid + lid] = histogram[lid];
    }
}