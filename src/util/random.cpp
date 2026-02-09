#include <Ende/math/random.h>
#include <Canta/util/random.h>

auto canta::randomListUint(RenderGraph &graph, const u32 min, const u32 max, const u32 count, const u32 offset, BufferIndex buffer) -> BufferIndex {
    if (buffer.id < 0) {
        buffer = graph.addBuffer({
            .size = static_cast<u32>(count * sizeof(f32)),
            .name = "random_list"
        });
    }

    const auto seed = ende::math::rand<u32>(0, count);

    assert(max > min);

    const auto output = graph.addPass({.name = "generate_random_list"})
        .setPipeline(graph.device()->randomListGenerator())
        .pushConstants(Write(buffer), count, offset, max, min, seed)
        .dispatchThreads(count).aliasBufferOutput(0);

    return *output;
}

auto canta::randomListFloat(RenderGraph &graph, const f32 min, const f32 max, const u32 count, const u32 offset, BufferIndex buffer) -> BufferIndex {
    if (buffer.id < 0) {
        buffer = graph.addBuffer({
            .size = static_cast<u32>(count * sizeof(f32)),
            .name = "random_list"
        });
    }

    const auto seed = ende::math::rand<u32>(0, count);

    assert(max > min);

    const auto output = graph.addPass({.name = "generate_random_list"})
        .setPipeline(graph.device()->randomListGeneratorFloat())
        .pushConstants(Write(buffer), count, offset, max, min, seed)
        .dispatchThreads(count).aliasBufferOutput(0);

    return *output;
}
