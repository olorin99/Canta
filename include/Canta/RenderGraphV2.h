#ifndef CANTA_RENDERGRAPHV2_H
#define CANTA_RENDERGRAPHV2_H

#include <Ende/platform.h>
#include <Ende/graph/graph.h>
#include <expected>
#include <Canta/Device.h>

#include "RenderGraphV2.h"

namespace canta::V2 {

    enum class RenderGraphError {

    };

    struct ImageIndex : ende::graph::Edge {
        i32 index = -1;
    };

    struct BufferIndex : ende::graph::Edge {
        i32 index = -1;
    };

    struct ResourceAccess {
        i32 id = -1;
        i32 index = -1;
        Access access = Access::NONE;
        PipelineStage stage = PipelineStage::NONE;
        ImageLayout layout = ImageLayout::UNDEFINED;
    };

    class RenderGraph;
    class PassBuilder;
    class ComputePass;
    class RenderPass : public ende::graph::Vertex<BufferIndex, ImageIndex> {
    public:

        enum class Type {
            NONE,
            COMPUTE,
            GRAPHICS,
            TRANSFER,
            PRESENT,
            HOST,
        };

        auto clone() -> RenderPass;

        auto setCallback(const std::function<void(CommandBuffer&, RenderGraph&)>& callback) -> RenderPass&;

    protected:
        friend RenderGraph;
        friend PassBuilder;
        friend ComputePass;

        Type _type = Type::NONE;
        std::array<u8, 192> _pushData = {};
        std::function<void(CommandBuffer&, RenderGraph&)> _callback = {};
        std::vector<ResourceAccess> _accesses = {};

    };

    class PassBuilder {
    public:

        PassBuilder(RenderGraph* graph, u32 index);

        auto pass() -> RenderPass&;

        auto read(BufferIndex index, Access access, PipelineStage stage) -> bool;
        auto read(ImageIndex index, Access access, PipelineStage stage, ImageLayout layout) -> bool;

        auto write(BufferIndex index, Access access, PipelineStage stage) -> bool;
        auto write(ImageIndex index, Access access, PipelineStage stage, ImageLayout layout) -> bool;

    protected:

        auto addColourRead(ImageIndex index) -> PassBuilder&;
        auto addColourWrite(ImageIndex index, const ClearValue& clearColor = std::to_array({ 0.f, 0.f, 0.f, 1.f })) -> PassBuilder&;

        auto addDepthRead(ImageIndex index) -> PassBuilder&;
        auto addDepthWrite(ImageIndex index, const ClearValue& clearColor = DepthClearValue{ .depth = 1.f, .stencil = 0 }) -> PassBuilder&;

        auto addStorageImageRead(ImageIndex index, PipelineStage stage = PipelineStage::NONE) -> PassBuilder&;
        auto addStorageImageWrite(ImageIndex index, PipelineStage stage = PipelineStage::NONE) -> PassBuilder&;

        auto addStorageBufferRead(BufferIndex index, PipelineStage stage = PipelineStage::NONE) -> PassBuilder&;
        auto addStorageBufferWrite(BufferIndex index, PipelineStage stage = PipelineStage::NONE) -> PassBuilder&;

        auto addSampledRead(ImageIndex index, PipelineStage stage = PipelineStage::NONE) -> PassBuilder&;

        auto addBlitRead(ImageIndex index) -> PassBuilder&;
        auto addBlitWrite(ImageIndex index) -> PassBuilder&;

        auto addTransferRead(ImageIndex index) -> PassBuilder&;
        auto addTransferWrite(ImageIndex index) -> PassBuilder&;
        auto addTransferRead(BufferIndex index) -> PassBuilder&;
        auto addTransferWrite(BufferIndex index) -> PassBuilder&;

        auto addIndirectRead(BufferIndex index) -> PassBuilder&;

        auto addDummyRead(ImageIndex index) -> PassBuilder&;
        auto addDummyWrite(ImageIndex index) -> PassBuilder&;
        auto addDummyRead(BufferIndex index) -> PassBuilder&;
        auto addDummyWrite(BufferIndex index) -> PassBuilder&;

    private:
        friend RenderGraph;

        RenderGraph* _graph = nullptr;
        u32 _vertexIndex = 0;

    };

    class ComputePass : public PassBuilder {
    public:

        ComputePass(RenderGraph* graph, u32 index);

        auto addStorageImageRead(ImageIndex index) -> ComputePass&;
        auto addStorageImageWrite(ImageIndex index) -> ComputePass&;

        auto addStorageBufferRead(BufferIndex index) -> ComputePass&;
        auto addStorageBufferWrite(BufferIndex index) -> ComputePass&;


        auto dispatchThreads(u32 x = 1, u32 y = 1, u32 z = 1) -> ComputePass&;


    };



    class RenderGraph : public ende::graph::Graph<RenderPass> {
    public:

        auto compute() -> ComputePass;

    private:

    };


}


#endif //CANTA_RENDERGRAPHV2_H