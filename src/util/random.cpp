#include <Ende/math/random.h>
#include <Canta/util/random.h>

auto canta::randomList(RenderGraph &graph, const f32 min, const f32 max, const u32 count, const u32 offset, BufferIndex buffer) -> BufferIndex {
    if (buffer.id < 0) {
        buffer = graph.addBuffer({
            .size = static_cast<u32>(count * sizeof(f32)),
            .name = "random_list"
        });
    }

    const auto seed = ende::math::rand<f32>(0, count);

    assert(max > min);
    const auto scale = max - min;
    const auto scaleOffset = min;

    const auto output = graph.addPass({.name = "generate_random_list"})
        .setPipeline(graph.device()->randomListGenerator())
        .pushConstants(Write(buffer), count, offset, scale, scaleOffset, seed)
        .dispatchThreads(count).aliasBufferOutput(0);

    return *output;
}
