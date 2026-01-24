#include <Canta/util/sort.h>

auto canta::sort(RenderGraph &renderGraph, const BufferIndex keys, const BufferIndex values, u32 count, const u32 typeSize) -> SortOutput {
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
