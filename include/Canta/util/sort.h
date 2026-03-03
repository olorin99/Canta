#ifndef CANTA_SORT_H
#define CANTA_SORT_H

#include <Canta/RenderGraphV2.h>

namespace canta::V2 {

    struct SortOutput {
        BufferIndex keys;
        BufferIndex values;
    };

    auto singleSort(RenderGraph& renderGraph, BufferIndex keys, BufferIndex values, u32 count, u32 typeSize) -> std::expected<SortOutput, RenderGraphError>;

    template <typename T>
    auto singleSort(RenderGraph& renderGraph, const BufferIndex keys, const BufferIndex values, const u32 count = 0) -> std::expected<SortOutput, RenderGraphError> {
        return singleSort(renderGraph, keys, values, count, sizeof(T));
    }

    auto multiSort(RenderGraph& renderGraph, BufferIndex keys, BufferIndex values, u32 count, u32 numBlocksPerWorkgroup, u32 typeSize) -> std::expected<SortOutput, RenderGraphError>;

    template <typename T>
    auto multiSort(RenderGraph& renderGraph, const BufferIndex keys, const BufferIndex values, const u32 count = 0, const u32 numBlocksPerWorkgroup = 0) -> std::expected<SortOutput, RenderGraphError> {
        return multiSort(renderGraph, keys, values, count, numBlocksPerWorkgroup, sizeof(T));
    }

    template <typename T>
    auto sort(RenderGraph& renderGraph, const BufferIndex keys, const BufferIndex values, const u32 count = 0, const u32 numBlocksPerWorkgroup = 0) -> std::expected<SortOutput, RenderGraphError> {
        return multiSort(renderGraph, keys, values, count, numBlocksPerWorkgroup, sizeof(T));
    }

}

#endif //CANTA_SORT_H