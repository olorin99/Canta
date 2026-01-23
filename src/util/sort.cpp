#include <Canta/util/sort.h>

auto canta::sort(PipelineManager& manager, RenderGraph &renderGraph, BufferIndex keys, BufferIndex values, u32 count, u32 typeSize) -> SortOutput {
    assert(typeSize % sizeof(u32) == 0);

    const auto tmpKeys = renderGraph.duplicate(keys);
    const auto tmpValues = renderGraph.duplicate(values);

    auto outputs = renderGraph.addPass({.name = "sort"})
        .addStorageBufferRead(keys)
        .addStorageBufferRead(values)
        .setPipeline(manager.getPipeline({
            .compute = {
                .path = CANTA_SRC_DIR"/src/util/sort.slang"
            },
            .specializationConstants = {SpecializationConstant{.id = 0, .name = "typeSize", .value = typeSize}}
        }).value())
        .pushConstants(canta::Write(keys), canta::Write(tmpKeys), canta::Write(values), canta::Write(tmpValues), count)
        .dispatchThreads(count).aliasBufferOutputs();

    return {
        .keys = outputs[0],
        .values = outputs[2],
    };
}
