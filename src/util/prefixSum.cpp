#include <Canta/util/prefixSum.h>

#include "embedded_shaders_Canta.h"

auto canta::prefixSumExclusive(RenderGraph &graph, BufferIndex values, const u32 count) -> BufferIndex {

    const auto vectorizedSize = ((count + 4) / 4 * 4) / 4;
    constexpr auto partitionSize = 3072;
    const auto partitions = (vectorizedSize + partitionSize - 1) / partitionSize;

    // const auto threadBlockReductionCount = count / graph.device()->prefixSumExclusive()->localSize(ShaderStage::COMPUTE)->x() + 1;
    const auto threadBlockReductionCount = count / 256 + 1;
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

    const auto summed = prefix_sum(count).entry("exclusive")(graph, Read(values), Write(sums), ReadWrite(*clearedScanBump), ReadWrite(*clearedThreadBlockReductions), vectorizedSize, partitions)->output<BufferIndex>();

    return *summed;
}

auto canta::prefixSumInclusive(RenderGraph &graph, BufferIndex values, const u32 count) -> BufferIndex {

    const auto vectorizedSize = ((count + 4) / 4 * 4) / 4;
    constexpr auto partitionSize = 3072;
    const auto partitions = (vectorizedSize + partitionSize - 1) / partitionSize;

    // const auto threadBlockReductionCount = count / graph.device()->prefixSumExclusive()->localSize(ShaderStage::COMPUTE)->x() + 1;
    const auto threadBlockReductionCount = count / 256 + 1;
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

    const auto summed = prefix_sum(count).entry("inclusive")(graph, Read(values), Write(sums), ReadWrite(*clearedScanBump), ReadWrite(*clearedThreadBlockReductions), vectorizedSize, partitions)->output<BufferIndex>();

    return *summed;
}
