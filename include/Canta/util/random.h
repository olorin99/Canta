#ifndef CANTA_RANDOM_H
#define CANTA_RANDOM_H

#include <Canta/RenderGraph.h>

namespace canta {

    auto randomList(RenderGraph& graph, f32 min, f32 max, u32 count, u32 offset = 0, BufferIndex buffer = {}) -> BufferIndex;

}

#endif //CANTA_RANDOM_H