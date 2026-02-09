#include <Canta/util/prefixSum.h>

auto canta::prefixSumExclusive(RenderGraph &graph, BufferIndex values, const u32 count) -> BufferIndex {

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

    const auto clearedScanBump = graph.addClearPass("clear_scan_bump", scanBump, 0, 0, count).aliasBufferOutput(0);
    const auto clearedThreadBlockReductions = graph.addClearPass("clear_thread_block_reductions", threadBlockReductions, 0, 0, count).aliasBufferOutput(0);

    const auto summed = graph.addPass({.name = "sum"})
        .addStorageBufferRead(*clearedScanBump)
        .addStorageBufferRead(*clearedThreadBlockReductions)
        .setPipeline(graph.device()->prefixSumExclusive())
        .pushConstants(Read(values), Write(sums), Write(*clearedScanBump), Write(*clearedThreadBlockReductions), vectorizedSize, partitions)
        .dispatchThreads(count).aliasBufferOutputs();

    return summed.front();
}

auto canta::prefixSumInclusive(RenderGraph &graph, BufferIndex values, const u32 count) -> BufferIndex {

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

    const auto clearedScanBump = graph.addClearPass("clear_scan_bump", scanBump, 0, 0, count).aliasBufferOutput(0);
    const auto clearedThreadBlockReductions = graph.addClearPass("clear_thread_block_reductions", threadBlockReductions, 0, 0, count).aliasBufferOutput(0);

    const auto summed = graph.addPass({.name = "sum"})
        .addStorageBufferRead(*clearedScanBump)
        .addStorageBufferRead(*clearedThreadBlockReductions)
        .setPipeline(graph.device()->prefixSumInclusive())
        .pushConstants(Read(values), Write(sums), Write(*clearedScanBump), Write(*clearedThreadBlockReductions), vectorizedSize, partitions)
        .dispatchThreads(count).aliasBufferOutputs();

    return summed.front();
}
