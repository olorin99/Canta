#ifndef CANTA_PREFIXSUM_H
#define CANTA_PREFIXSUM_H

#include <Canta/RenderGraphV2.h>

namespace canta {

    auto prefixSumExclusive(V2::RenderGraph& graph, V2::BufferIndex values, u32 count) -> V2::BufferIndex;

    auto prefixSumInclusive(V2::RenderGraph& graph, V2::BufferIndex values, u32 count) -> V2::BufferIndex;

}

#endif //CANTA_PREFIXSUM_H