#ifndef CANTA_RANDOM_H
#define CANTA_RANDOM_H

#include <Canta/RenderGraphV2.h>

namespace canta {

    auto randomListUint(V2::RenderGraph& graph, u32 min, u32 max, u32 count, u32 offset = 0, V2::BufferIndex buffer = {}) -> V2::BufferIndex;

    auto randomListFloat(V2::RenderGraph& graph, f32 min, f32 max, u32 count, u32 offset = 0, V2::BufferIndex buffer = {}) -> V2::BufferIndex;

    auto generateRandomNoise(V2::RenderGraph& graph, u32 width, u32 height, u32 seed = 0, V2::ImageIndex image = {}) -> V2::ImageIndex;

    struct PerlinOptions{f32 time = 0; u32 octaves = 6; f32 persistence = 0.5; f32 lacunarity = 2; u32 seed = 0;};
    auto generatePerlinNoise(V2::RenderGraph& graph, u32 width, u32 height, PerlinOptions options = {}, V2::ImageIndex image = {}) -> V2::ImageIndex;

}

#endif //CANTA_RANDOM_H