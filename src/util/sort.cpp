#include <Canta/util/sort.h>
#include "embeded_shaders_Canta.h"

auto canta::sort(RenderGraph &renderGraph, const BufferIndex keys, const BufferIndex values, const u32 count, const u32 typeSize) -> SortOutput {
    assert(typeSize % sizeof(u32) == 0);

    const auto tmpKeys = renderGraph.duplicate(keys);
    const auto tmpValues = renderGraph.duplicate(values);

    const auto pipeline = renderGraph.device()->singleSortPipeline();

    const auto outputs = renderGraph.addPass({.name = "sort"})
        .addStorageBufferRead(keys)
        .addStorageBufferRead(values)
        .setPipeline(pipeline)
        .pushConstants(canta::Write(keys), canta::Write(tmpKeys), canta::Write(values), canta::Write(tmpValues), count, typeSize)
        .dispatchThreads(count).aliasBufferOutputs();

    return {
        .keys = outputs[0],
        .values = outputs[2],
    };
}
