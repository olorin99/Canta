#ifndef CANTA_SORT_H
#define CANTA_SORT_H

#include <Canta/RenderGraph.h>

namespace canta {

    struct SortOutput {
        BufferIndex keys;
        BufferIndex values;
    };

    auto sort(PipelineManager& manager, RenderGraph& renderGraph, BufferIndex keys, BufferIndex values, u32 count, u32 typeSize) -> SortOutput;

    template <typename T>
    auto sort(PipelineManager& manager, RenderGraph& renderGraph, BufferIndex keys, BufferIndex values, u32 count) -> SortOutput {
        return sort(manager, renderGraph, keys, values, count, sizeof(T));
    }

}

#endif //CANTA_SORT_H