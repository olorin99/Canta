#ifndef CANTA_RANDOM_H
#define CANTA_RANDOM_H

#include <Canta/RenderGraph.h>

namespace canta {

    auto randomListUint(RenderGraph& graph, u32 min, u32 max, u32 count, u32 offset = 0, BufferIndex buffer = {}) -> BufferIndex;

    auto randomListFloat(RenderGraph& graph, f32 min, f32 max, u32 count, u32 offset = 0, BufferIndex buffer = {}) -> BufferIndex;

    auto generateRandomNoise(RenderGraph& graph, u32 width, u32 height, u32 seed = 0, ImageIndex image = {}) -> ImageIndex;

    struct PerlinOptions{f32 time = 0; u32 octaves = 6; f32 persistence = 0.5; f32 lacunarity = 2; u32 seed = 0;};
    auto generatePerlinNoise(RenderGraph& graph, u32 width, u32 height, PerlinOptions options = {}, ImageIndex image = {}) -> ImageIndex;

}

#endif //CANTA_RANDOM_H