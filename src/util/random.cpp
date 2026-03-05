#include <Ende/math/random.h>
#include <Canta/util/random.h>

#include "Canta/RenderGraph.h"

auto canta::randomListUint(RenderGraph &graph, const u32 min, const u32 max, const u32 count, const u32 offset, BufferIndex buffer) -> BufferIndex {
    if (buffer.id < 0) {
        buffer = graph.addBuffer({
            .size = static_cast<u32>(count * sizeof(f32)),
            .name = "random_list"
        });
    }

    const auto seed = ende::math::rand<u32>(0, count);

    assert(max > min);

    const auto output = graph.compute("generate_random_list", graph.device()->randomListGenerator())
        .pushConstants(Write(buffer), count, offset, max, min, seed)
        .dispatchThreads(count).output<BufferIndex>();

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

    const auto output = graph.compute("generate_random_list", graph.device()->randomListGeneratorFloat())
        .pushConstants(Write(buffer), count, offset, max, min, seed)
        .dispatchThreads(count).output<BufferIndex>();

    return *output;
}

auto canta::generateRandomNoise(RenderGraph &graph, const u32 width, const u32 height, u32 seed, ImageIndex image) -> ImageIndex {
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
        .pushConstants(Write(image), seed, seed)
        .dispatchThreads(width, height)
        .output<ImageIndex>();

    return *output;
}

auto canta::generatePerlinNoise(RenderGraph &graph, const u32 width, const u32 height, const PerlinOptions options, ImageIndex image) -> ImageIndex {
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
        .pushConstants(Write(image), options.time, options.octaves, options.persistence, options.lacunarity, seed)
        .dispatchThreads(width, height)
        .output<ImageIndex>();

    return *output;
}
