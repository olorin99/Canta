#include <Canta/util/sort.h>

auto canta::singleSort(RenderGraph &renderGraph, const BufferIndex keys, const BufferIndex values, u32 count, const u32 typeSize) -> std::expected<SortOutput, RenderGraphError> {
    assert(typeSize % sizeof(u32) == 0);

    if (count == 0) {
        const auto keysInfo = TRY(renderGraph.getBufferInfo(keys));
        count = keysInfo.size / sizeof(u32);
    }

    const auto tmpKeys = TRY(renderGraph.duplicate(keys));
    const auto tmpValues = TRY(renderGraph.duplicate(values));

    const auto pipeline = renderGraph.device()->singleSortPipeline();

    auto pass = renderGraph.compute("sort", pipeline)
        .addStorageBufferRead(keys)
        .addStorageBufferRead(values)
        .addStorageBufferWrite(keys)
        .addStorageBufferWrite(tmpKeys)
        .addStorageBufferWrite(values)
        .addStorageBufferWrite(tmpValues)
        .pushConstants(keys, tmpKeys, values, tmpValues, count, typeSize)
        .dispatchWorkgroups(1);

    return SortOutput{
        .keys = TRY(pass.output<BufferIndex>(0)),
        .values = TRY(pass.output<BufferIndex>(2)),
    };
}

auto canta::multiSort(RenderGraph& graph, const BufferIndex keys, const BufferIndex values, u32 count, u32 numBlocksPerWorkgroup, const u32 typeSize) -> std::expected<SortOutput, RenderGraphError> {
    assert(typeSize % sizeof(u32) == 0);

    if (count == 0) {
        const auto keysInfo = TRY(graph.getBufferInfo(keys));
        count = keysInfo.size / sizeof(u32);
    }
    if (numBlocksPerWorkgroup == 0) {

        // From testing on a rx 6800 xt targeting about 40 workgroups seems to give the best performance across different input counts.
        // Will probably be different for different hardware.
        constexpr auto workgroupCount = 40;
        numBlocksPerWorkgroup = std::max(1u, count / ((256 * workgroupCount) - 255));
        // numBlocksPerWorkgroup = 32;
    }

    u32 totalThreads = count / numBlocksPerWorkgroup;
    const u32 remainder = count % numBlocksPerWorkgroup;
    totalThreads += remainder > 0 ? 1 : 0;
    const u32 numWorkgroups = (totalThreads + 256 - 1) / 256;

    const auto tmpKeys = TRY(graph.duplicate(keys));
    const auto tmpValues = TRY(graph.duplicate(values));
    const auto histogram = graph.addBuffer({
        .size = static_cast<u32>(numWorkgroups * 265 * sizeof(u32)),
        .name = "histogram",
    });

    // auto sortGroup = graph.getGroup("sort");

    auto histogramsPipeline = graph.device()->multiSortHistogramsPipeline();
    auto sortPipeline = graph.device()->multiSortPipeline();

    auto index = keys;
    for (u32 iteration = 0; iteration < 4; iteration++) {
        const u32 shift = 8 * iteration;

        const auto iterationKeys = iteration % 2 == 0 ? keys : tmpKeys;
        const auto iterationTmpKeys = iteration % 2 == 0 ? tmpKeys : keys;
        const auto iterationValues = iteration % 2 == 0 ? values : tmpValues;
        const auto iterationTmpValues = iteration % 2 == 0 ? tmpValues : values;

        auto histogramOutput = TRY(graph.compute("histogram_pass", histogramsPipeline)
            .addStorageBufferRead(index)
            .addStorageBufferRead(values)
            .addStorageBufferRead(iterationKeys)
            .addStorageBufferWrite(histogram)
            .pushConstants(iterationKeys, histogram, count, shift, numWorkgroups, numBlocksPerWorkgroup)
            .dispatchThreads(totalThreads)
            .output<BufferIndex>());

        auto sortPass = graph.compute("sort_pass", sortPipeline)
            .addStorageBufferRead(iterationKeys)
            .addStorageBufferRead(iterationValues)
            .addStorageBufferRead(iterationTmpKeys)
            .addStorageBufferRead(iterationTmpValues)
            .addStorageBufferRead(histogramOutput)
            .addStorageBufferWrite(iterationKeys)
            .addStorageBufferWrite(iterationValues)
            .addStorageBufferWrite(iterationTmpKeys)
            .addStorageBufferWrite(iterationTmpValues)
            .pushConstants(iterationKeys, iterationTmpKeys, iterationValues, iterationTmpValues, histogramOutput, count, shift, numWorkgroups, numBlocksPerWorkgroup, typeSize)
            .dispatchThreads(totalThreads);

        index = TRY(sortPass.output<BufferIndex>());
    }

    return SortOutput{
        .keys = index,
        .values = values,
    };
}