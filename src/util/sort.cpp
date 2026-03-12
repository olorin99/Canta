#include <Canta/util/sort.h>

#include "embedded_shaders_Canta.h"

auto canta::singleSort(RenderGraph &renderGraph, const BufferIndex keys, const BufferIndex values, u32 count, const u32 typeSize) -> std::expected<SortOutput, RenderGraphError> {
    assert(typeSize % sizeof(u32) == 0);

    if (count == 0) {
        const auto keysInfo = TRY(renderGraph.getBufferInfo(keys));
        count = keysInfo.size / sizeof(u32);
    }

    const auto tmpKeys = TRY(renderGraph.duplicate(keys));
    const auto tmpValues = TRY(renderGraph.duplicate(values));

    auto pass = TRY(single_sort()(renderGraph, ReadWrite(keys), Write(tmpKeys), ReadWrite(values), Write(tmpValues), count, typeSize));

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

    auto sortGroup = graph.addGroup("sort", { 0, 0, 1, 1 });

    auto index = keys;
    for (u32 iteration = 0; iteration < 4; iteration++) {
        const u32 shift = 8 * iteration;

        const auto iterationKeys = iteration % 2 == 0 ? keys : tmpKeys;
        const auto iterationTmpKeys = iteration % 2 == 0 ? tmpKeys : keys;
        const auto iterationValues = iteration % 2 == 0 ? values : tmpValues;
        const auto iterationTmpValues = iteration % 2 == 0 ? tmpValues : values;

        auto histogramOutput = TRY(TRY(multi_sort_histograms(totalThreads)(graph, Read(iterationKeys), ReadWrite(histogram), count, shift, numWorkgroups, numBlocksPerWorkgroup))
            .addStorageBufferRead(index)
            .addStorageBufferRead(values)
            .output<BufferIndex>());

        auto sortPass = TRY(multi_sort(totalThreads)(graph, ReadWrite(iterationKeys), ReadWrite(iterationTmpKeys), ReadWrite(iterationValues), ReadWrite(iterationTmpValues), Read(histogramOutput), count, shift, numWorkgroups, numBlocksPerWorkgroup, typeSize));

        index = TRY(sortPass.output<BufferIndex>());
    }

    return SortOutput{
        .keys = index,
        .values = values,
    };
}