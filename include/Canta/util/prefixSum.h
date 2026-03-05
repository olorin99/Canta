#ifndef CANTA_PREFIXSUM_H
#define CANTA_PREFIXSUM_H

#include <Canta/RenderGraph.h>

namespace canta {

    auto prefixSumExclusive(RenderGraph& graph, BufferIndex values, u32 count) -> BufferIndex;

    auto prefixSumInclusive(RenderGraph& graph, BufferIndex values, u32 count) -> BufferIndex;

}

#endif //CANTA_PREFIXSUM_H