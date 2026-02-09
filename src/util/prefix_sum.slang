// derived from https://github.com/b0nes164/GPUPrefixSums/blob/main/GPUPrefixSumsD3D12/Shaders/ChainedScanDecoupledLookback.hlsl

import canta;

#define BLOCK_SIZE 256
[vk::constant_id(0)] const uint x_size = BLOCK_SIZE;
[vk::constant_id(1)] const uint y_size = 1;
[vk::constant_id(2)] const uint z_size = 1;

#define UINT4_PART_SIZE     768U
#define UINT4_PER_THREAD    3U
#define MIN_WAVE_SIZE       4U


#define FLAG_NOT_READY  0
#define FLAG_REDUCTION  1
#define FLAG_INCLUSIVE  2
#define FLAG_MASK       3

groupshared uint reduction[BLOCK_SIZE / MIN_WAVE_SIZE];
groupshared uint broadcast;

struct scan {
    uint4 t[UINT4_PER_THREAD];
};

void acquirePartitionIndex(uint lid, uint* scanBump) {
    if (lid == 0)
        InterlockedAdd(scanBump[0], 1, broadcast);
}

void deviceBroadcast(uint lid, uint partIndex, uint* threadBlockReduction) {
    if (lid == 0)
    {
        uint t;
        InterlockedExchange(threadBlockReduction[partIndex],
            (partIndex != 0 ? FLAG_REDUCTION : FLAG_INCLUSIVE) |
            reduction[BLOCK_SIZE / WaveGetLaneCount() - 1] << 2, t);
    }
}

uint getWaveIndex(uint lid) {
    return lid / WaveGetLaneCount();
}

uint partStart(uint partIndex) {
    return partIndex * UINT4_PART_SIZE;
}

uint wavePartSize() {
    return UINT4_PER_THREAD * WaveGetLaneCount();
}

uint wavePartStart(uint lid) {
    return getWaveIndex(lid) * wavePartSize();
}

uint4 setXAddYZW(uint t, uint4 val) {
    return uint4(t, val.yzw + t);
}

void scanInclusiveFull(uint lid, uint partIndex, uint4* scanIn, inout scan s) {
    let laneMask = WaveGetLaneCount() - 1;
    let circularShift = WaveGetLaneIndex() + laneMask & laneMask;
    uint waveReduction = 0;

    [unroll]
    for (uint i = WaveGetLaneIndex() + wavePartStart(lid) + partStart(partIndex), k = 0;
        k < UINT4_PER_THREAD;
        i += WaveGetLaneCount(), ++k)
    {
        s.t[k] = scanIn[i];
        s.t[k].y += s.t[k].x;
        s.t[k].z += s.t[k].y;
        s.t[k].w += s.t[k].z;

        let t = WaveReadLaneAt(s.t[k].w + WavePrefixSum(s.t[k].w), circularShift);
        s.t[k] += (WaveGetLaneIndex() != 0 ? t : 0) + waveReduction;
        waveReduction += WaveReadLaneAt(t, 0);
    }

    if (WaveGetLaneIndex() == 0)
        reduction[getWaveIndex(lid)] = waveReduction;
}

void scanInclusivePartial(uint lid, uint partIndex, uint vectorizedSize, uint4* scanIn, inout scan t_s) {
    let laneMask = WaveGetLaneCount() - 1;
    let circularShift = WaveGetLaneIndex() + laneMask & laneMask;
    uint waveReduction = 0;

    let isFirstWaveIndex = WaveGetLaneIndex() == 0;

    [unroll]
    for (uint i = WaveGetLaneIndex() + wavePartStart(lid) + partStart(partIndex), k = 0;
        k < UINT4_PER_THREAD;
        i += WaveGetLaneCount(), ++k)
    {
        t_s.t[k] = i < vectorizedSize ? scanIn[i] : 0;
        t_s.t[k].y += t_s.t[k].x;
        t_s.t[k].z += t_s.t[k].y;
        t_s.t[k].w += t_s.t[k].z;

        let t = WaveReadLaneAt(t_s.t[k].w + WavePrefixSum(t_s.t[k].w), circularShift);
        t_s.t[k] += (!isFirstWaveIndex ? t : 0) + waveReduction;
        waveReduction += WaveReadLaneAt(t, 0);
    }

    if (isFirstWaveIndex)
        reduction[getWaveIndex(lid)] = waveReduction;
}

void scanExclusiveFull(uint lid, uint partIndex, uint4* scanIn, inout scan s) {
    let laneMask = WaveGetLaneCount() - 1;
    let circularShift = WaveGetLaneIndex() + laneMask & laneMask;
    uint waveReduction = 0;

    let isFirstWaveIndex = WaveGetLaneIndex() == 0;

    [unroll]
    for (uint i = WaveGetLaneIndex() + wavePartStart(lid) + partStart(partIndex), k = 0;
        k < UINT4_PER_THREAD;
        i += WaveGetLaneCount(), ++k) {
        s.t[k] = scanIn[i];

        uint t0 = s.t[k].x;
        s.t[k].x += s.t[k].y;
        s.t[k].y = t0;

        t0 = s.t[k].x;
        s.t[k].x += s.t[k].z;
        s.t[k].z = t0;

        t0 = s.t[k].x;
        s.t[k].x += s.t[k].w;
        s.t[k].w = t0;

        let t1 = WaveReadLaneAt(s.t[k].x + WavePrefixSum(s.t[k].x), circularShift);
        s.t[k] = setXAddYZW((!isFirstWaveIndex ? t1 : 0) + waveReduction, s.t[k]);
        waveReduction += WaveReadLaneAt(t1, 0);
    }

    if (isFirstWaveIndex)
        reduction[getWaveIndex(lid)] = waveReduction;
}

void scanExclusivePartial(uint lid, uint partIndex, uint vectorizedSize, uint4* scanIn, inout scan t_s) {
    let laneMask = WaveGetLaneCount() - 1;
    let circularShift = WaveGetLaneIndex() + laneMask & laneMask;
    var waveReduction = 0;

    let isFirstWaveIndex = WaveGetLaneIndex() == 0;

    [unroll]
    for (uint i = WaveGetLaneIndex() + wavePartStart(lid) + partStart(partIndex), k = 0;
        k < UINT4_PER_THREAD;
        i += WaveGetLaneCount(), ++k)
    {
        t_s.t[k] = i < vectorizedSize ? scanIn[i] : 0;

        var t0 = t_s.t[k].x;
        t_s.t[k].x += t_s.t[k].y;
        t_s.t[k].y = t0;

        t0 = t_s.t[k].x;
        t_s.t[k].x += t_s.t[k].z;
        t_s.t[k].z = t0;

        t0 = t_s.t[k].x;
        t_s.t[k].x += t_s.t[k].w;
        t_s.t[k].w = t0;

        let t1 = WaveReadLaneAt(t_s.t[k].x + WavePrefixSum(t_s.t[k].x), circularShift);
        t_s.t[k] = setXAddYZW((!isFirstWaveIndex ? t1 : 0) + waveReduction, t_s.t[k]);
        waveReduction += WaveReadLaneAt(t1, 0);
    }

    if (isFirstWaveIndex)
        reduction[getWaveIndex(lid)] = waveReduction;
}

void spineScan(uint lid)
{
    let laneLog = countbits(WaveGetLaneCount() - 1);
    let spineSize = BLOCK_SIZE >> laneLog;
    let alignedSize = 1 << (countbits(spineSize - 1) + laneLog - 1) / laneLog * laneLog;
    uint offset = 0;
    for (uint j = WaveGetLaneCount(); j <= alignedSize; j <<= laneLog)
    {
        let t0 = j != WaveGetLaneCount() ? 1 : 0;
        let i0 = (lid + t0 << offset) - t0;
        let pred0 = i0 < spineSize;
        let t1 = pred0 ? reduction[i0] : 0;
        let t2 = t1 + WavePrefixSum(t1);
        if (pred0)
            reduction[i0] = t2;
        GroupMemoryBarrierWithGroupSync();

        if (j != WaveGetLaneCount())
        {
            let rshift = j >> laneLog;
            let i1 = lid + rshift;
            if ((i1 & j - 1) >= rshift)
            {
                let pred1 = i1 < spineSize;
                let t3 = pred1 ? reduction[((i1 >> offset) << offset) - 1] : 0;
                if (pred1 && (i1 + 1 & rshift - 1) != 0)
                    reduction[i1] += t3;
            }
        }
        offset += laneLog;
    }
}

void propagateFull(uint lid, uint partIndex, uint prevReduction, uint4* scanOut, scan t_s)
{
    [unroll]
    for (uint i = WaveGetLaneIndex() + wavePartStart(lid) + partStart(partIndex), k = 0;
        k < UINT4_PER_THREAD;
        i += WaveGetLaneCount(), ++k)
    {
        scanOut[i] = t_s.t[k] + prevReduction;
    }
}

void propagatePartial(uint lid, uint partIndex, uint prevReduction, uint vectorizedSize, uint4* scanOut, scan t_s)
{
    for (uint i = WaveGetLaneIndex() + wavePartStart(lid) + partStart(partIndex), k = 0;
        k < UINT4_PER_THREAD && i < vectorizedSize;
        i += WaveGetLaneCount(), ++k)
    {
        scanOut[i] = t_s.t[k] + prevReduction;
    }
}

void lookbackSingle(uint partIndex, uint* threadBlockReduction) {
    uint prevReduction = 0;
    uint lookBackIndex = partIndex - 1;

    while (true)
    {
        let flagPayload = threadBlockReduction[lookBackIndex];

        if ((flagPayload & FLAG_MASK) > FLAG_NOT_READY)
        {
            prevReduction += flagPayload >> 2;
            if ((flagPayload & FLAG_MASK) == FLAG_INCLUSIVE)
            {
                uint t;
                InterlockedExchange(threadBlockReduction[partIndex], FLAG_INCLUSIVE |
                    prevReduction + reduction[BLOCK_SIZE / WaveGetLaneCount() - 1] << 2, t);
                broadcast = prevReduction;
                break;
            }
            else
            {
                lookBackIndex--;
            }
        }
    }
}

[shader("compute")]
[numthreads(x_size, y_size, z_size)]
void inclusive(
    uint3 groupThread : SV_GroupThreadID,
    uniform uint4* scanIn,
    uniform uint4* scanOut,
    uniform uint* scanBump,
    uniform uint* threadBlockReduction,
    uniform const uint vectorizedSize,
    uniform const uint threadBlocks,
) {
    let lid = groupThread.x;

    acquirePartitionIndex(lid, scanBump);
    GroupMemoryBarrierWithGroupSync();
    let partitionIndex = broadcast;

    scan s;
    if (partitionIndex < threadBlocks - 1)
        scanInclusiveFull(lid, partitionIndex, scanIn, s);

    if (partitionIndex == threadBlocks - 1)
        scanInclusivePartial(lid, partitionIndex, vectorizedSize, scanIn, s);
    GroupMemoryBarrierWithGroupSync();

    spineScan(lid);
    GroupMemoryBarrierWithGroupSync();

    deviceBroadcast(lid, partitionIndex, threadBlockReduction);

    if (partitionIndex != 0 && lid == 0)
        lookbackSingle(partitionIndex, threadBlockReduction);
    GroupMemoryBarrierWithGroupSync();

    let prevReduction = broadcast + (lid >= WaveGetLaneCount() ? reduction[getWaveIndex(lid) - 1] : 0);

    if (partitionIndex < threadBlocks - 1)
        propagateFull(lid, partitionIndex, prevReduction, scanOut, s);
    if (partitionIndex == threadBlocks - 1)
        propagatePartial(lid, partitionIndex, prevReduction, vectorizedSize, scanOut, s);
}

[shader("compute")]
[numthreads(x_size, y_size, z_size)]
void exclusive(
    uint3 groupThread : SV_GroupThreadID,
    uniform uint4* scanIn,
    uniform uint4* scanOut,
    uniform uint* scanBump,
    uniform uint* threadBlockReduction,
    uniform const uint vectorizedSize,
    uniform const uint threadBlocks,
) {
    let lid = groupThread.x;

    acquirePartitionIndex(lid, scanBump);
    GroupMemoryBarrierWithGroupSync();
    let partitionIndex = broadcast;

    scan s;
    if (partitionIndex < threadBlocks - 1)
        scanExclusiveFull(lid, partitionIndex, scanIn, s);

    if (partitionIndex == threadBlocks - 1)
        scanExclusivePartial(lid, partitionIndex, vectorizedSize, scanIn, s);
    GroupMemoryBarrierWithGroupSync();

    spineScan(lid);
    GroupMemoryBarrierWithGroupSync();

    deviceBroadcast(lid, partitionIndex, threadBlockReduction);

    if (partitionIndex != 0 && lid == 0)
        lookbackSingle(partitionIndex, threadBlockReduction);
    GroupMemoryBarrierWithGroupSync();

    let prevReduction = broadcast + (lid >= WaveGetLaneCount() ? reduction[getWaveIndex(lid) - 1] : 0);

    if (partitionIndex < threadBlocks - 1)
            propagateFull(lid, partitionIndex, prevReduction, scanOut, s);
    if (partitionIndex == threadBlocks - 1)
        propagatePartial(lid, partitionIndex, prevReduction, vectorizedSize, scanOut, s);
}