#include <Ende/math/random.h>
#include <Canta/util/random.h>

#include "Canta/RenderGraphV2.h"

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

auto canta::generateRandomNoise(V2::RenderGraph &graph, const u32 width, const u32 height, u32 seed, V2::ImageIndex image) -> V2::ImageIndex {
    if (image.id < 0) {
        image = graph.addImage({
            .width = width,
            .height = height,
            .name = "random_noise"
        });
    }

    if (seed == 0) {
        seed = ende::math::rand<u32>(0, 1e9);
    }
    auto output = graph.compute("generate_random_noise", graph.device()->generateRandomNoise())
        .pushConstants(V2::Write(image), seed, seed)
        .dispatchThreads(width, height)
        .output<V2::ImageIndex>();

    return *output;
}

auto canta::generatePerlinNoise(V2::RenderGraph &graph, const u32 width, const u32 height, const PerlinOptions options, V2::ImageIndex image) -> V2::ImageIndex {
    if (image.id < 0) {
        image = graph.addImage({
            .width = width,
            .height = height,
            .format = Format::RGBA32_SFLOAT,
            .name = "perlin_noise"
        });
    }

    const auto seed = options.seed == 0 ? ende::math::rand<u32>(0, 100) : options.seed;

    auto output = graph.compute("generate_perlin_noise", graph.device()->generatePerlinNoise())
        .pushConstants(V2::Write(image), options.time, options.octaves, options.persistence, options.lacunarity, seed)
        .dispatchThreads(width, height)
        .output<V2::ImageIndex>();

    return *output;
}
