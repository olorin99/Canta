#include <Canta/util/sort.h>

auto canta::singleSort(RenderGraph &renderGraph, const BufferIndex keys, const BufferIndex values, u32 count, const u32 typeSize) -> SortOutput {
    assert(typeSize % sizeof(u32) == 0);

    if (count == 0) {
        const auto keysInfo = renderGraph.getBufferInfo(keys);
        count = keysInfo.size / sizeof(u32);
    }

    const auto tmpKeys = renderGraph.duplicate(keys);
    const auto tmpValues = renderGraph.duplicate(values);

    const auto pipeline = renderGraph.device()->singleSortPipeline();

    const auto outputs = renderGraph.addPass({.name = "sort"})
        .addStorageBufferRead(keys)
        .addStorageBufferRead(values)
        .setPipeline(pipeline)
        .pushConstants(canta::Write(keys), canta::Write(tmpKeys), canta::Write(values), canta::Write(tmpValues), count, typeSize)
        .dispatchWorkgroups(1).aliasBufferOutputs();

    return {
        .keys = outputs[0],
        .values = outputs[2],
    };
}

auto canta::multiSort(RenderGraph& graph, const BufferIndex keys, const BufferIndex values, u32 count, const u32 typeSize) -> SortOutput {
    assert(typeSize % sizeof(u32) == 0);

    if (count == 0) {
        const auto keysInfo = graph.getBufferInfo(keys);
        count = keysInfo.size / sizeof(u32);
    }

    constexpr u32 numBlocksPerWorkgroup = 32;
    u32 totalThreads = count / numBlocksPerWorkgroup;
    const u32 remainder = count % numBlocksPerWorkgroup;
    totalThreads += remainder > 0 ? 1 : 0;
    const u32 numWorkgroups = (totalThreads + 256 - 1) / 256;

    const auto tmpKeys = graph.duplicate(keys);
    const auto tmpValues = graph.duplicate(values);
    const auto histogram = graph.addBuffer({
        .size = static_cast<u32>(numWorkgroups * 265 * sizeof(u32)),
        .name = "histogram",
    });

    auto sortGroup = graph.getGroup("sort");

    auto histogramsPipeline = graph.device()->multiSortHistogramsPipeline();
    auto sortPipeline = graph.device()->multiSortPipeline();

    auto index = keys;
    for (u32 iteration = 0; iteration < 4; iteration++) {
        const u32 shift = 8 * iteration;

        const auto iterationKeys = iteration % 2 == 0 ? keys : tmpKeys;
        const auto iterationTmpKeys = iteration % 2 == 0 ? tmpKeys : keys;
        const auto iterationValues = iteration % 2 == 0 ? values : tmpValues;
        const auto iterationTmpValues = iteration % 2 == 0 ? tmpValues : values;

        auto histogramOutput = graph.addPass({.name = "histogram_pass", .group = sortGroup})
            .addStorageBufferRead(index)
            .addStorageBufferRead(values)
            .setPipeline(histogramsPipeline)
            .pushConstants(Read(iterationKeys), Write(histogram), count, shift, numWorkgroups, numBlocksPerWorkgroup)
            .dispatchThreads(totalThreads)
            .aliasBufferOutputs();

        auto sortOutput = graph.addPass({.name = "sort_pass", .group = sortGroup})
            .setPipeline(sortPipeline)
            .pushConstants(Write(iterationKeys), Write(iterationTmpKeys), Write(iterationValues), Write(iterationTmpValues), Read(histogramOutput[0]), count, shift, numWorkgroups, numBlocksPerWorkgroup, typeSize)
        .dispatchThreads(totalThreads).aliasBufferOutputs();
        index = sortOutput[0];
    }

    return {
        .keys = index,
        .values = values,
    };
}