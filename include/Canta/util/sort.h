#ifndef CANTA_SORT_H
#define CANTA_SORT_H

#include <Canta/RenderGraph.h>

namespace canta {

    struct SortOutput {
        BufferIndex keys;
        BufferIndex values;
    };

    auto sort(RenderGraph& renderGraph, BufferIndex keys, BufferIndex values, u32 count, u32 typeSize) -> SortOutput;

    template <typename T>
    auto sort(RenderGraph& renderGraph, const BufferIndex keys, const BufferIndex values, const u32 count = 0) -> SortOutput {
        return sort(renderGraph, keys, values, count, sizeof(T));
    }

}

#endif //CANTA_SORT_H