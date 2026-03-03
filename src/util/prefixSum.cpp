#include <Canta/util/prefixSum.h>

auto canta::prefixSumExclusive(V2::RenderGraph &graph, V2::BufferIndex values, const u32 count) -> V2::BufferIndex {

    const auto vectorizedSize = ((count + 4) / 4 * 4) / 4;
    constexpr auto partitionSize = 3072;
    const auto partitions = (vectorizedSize + partitionSize - 1) / partitionSize;

    const auto threadBlockReductionCount = count / graph.device()->prefixSumExclusive()->localSize(ShaderStage::COMPUTE)->x() + 1;
    const auto threadBlockReductions = graph.addBuffer({
        .size = static_cast<u32>(threadBlockReductionCount * sizeof(u32)),
        .name = "thread_block_reduction"
    });

    const auto scanBump = graph.addBuffer({
        .size = sizeof(u32),
        .name = "scan_bump"
    });

    const auto sums = graph.addBuffer({
        .size = static_cast<u32>(count * sizeof(u32)),
        .name = "sums"
    });

    const auto clearedScanBump = graph.transfer("clear_scan_bump").clear(scanBump);
    const auto clearedThreadBlockReductions = graph.transfer("clear_thread_block_reductions").clear(threadBlockReductions);

    const auto summed = graph.compute("sum", graph.device()->prefixSumExclusive())
        .addStorageBufferRead(*clearedScanBump)
        .addStorageBufferRead(*clearedThreadBlockReductions)
        .pushConstants(V2::Read(values), V2::Write(sums), V2::Write(*clearedScanBump), V2::Write(*clearedThreadBlockReductions), vectorizedSize, partitions)
        .dispatchThreads(count).output<V2::BufferIndex>();

    return *summed;
}

auto canta::prefixSumInclusive(V2::RenderGraph &graph, V2::BufferIndex values, const u32 count) -> V2::BufferIndex {

    const auto vectorizedSize = ((count + 4) / 4 * 4) / 4;
    constexpr auto partitionSize = 3072;
    const auto partitions = (vectorizedSize + partitionSize - 1) / partitionSize;

    const auto threadBlockReductionCount = count / graph.device()->prefixSumExclusive()->localSize(ShaderStage::COMPUTE)->x() + 1;
    const auto threadBlockReductions = graph.addBuffer({
        .size = static_cast<u32>(threadBlockReductionCount * sizeof(u32)),
        .name = "thread_block_reduction"
    });

    const auto scanBump = graph.addBuffer({
        .size = sizeof(u32),
        .name = "scan_bump"
    });

    const auto sums = graph.addBuffer({
        .size = static_cast<u32>(count * sizeof(u32)),
        .name = "sums"
    });

    const auto clearedScanBump = graph.transfer("clear_scan_bump").clear(scanBump);
    const auto clearedThreadBlockReductions = graph.transfer("clear_thread_block_reductions").clear(threadBlockReductions);

    const auto summed = graph.compute("sum", graph.device()->prefixSumInclusive())
        .addStorageBufferRead(*clearedScanBump)
        .addStorageBufferRead(*clearedThreadBlockReductions)
        .pushConstants(V2::Read(values), V2::Write(sums), V2::Write(*clearedScanBump), V2::Write(*clearedThreadBlockReductions), vectorizedSize, partitions)
        .dispatchThreads(count).output<V2::BufferIndex>();

    return *summed;
}
