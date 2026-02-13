#ifndef CANTA_RENDERGRAPHV2_H
#define CANTA_RENDERGRAPHV2_H

#include <Ende/platform.h>
#include <Ende/graph/graph.h>
#include <expected>
#include <Canta/Device.h>
#include <Canta/RenderGraphV2.h>

namespace canta::V2 {

    enum class RenderGraphError {
        NONE,
        IS_CYCLICAL,
        NO_ROOT,
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


        struct PushData {
            std::array<u8, 128> data = {};
            u32 size = 0;
        };

        auto clone() -> RenderPass;

        template <typename T, typename U, typename... Args>
        void unpack(PushData& dst, i32& i, const T& t, const U& u, const Args&... args) {
            unpack(dst, i, t);
            unpack(dst, i, u, args...);
        }

        template <typename T>
        void unpack(PushData& dst, i32& i, const T& t) {
            auto* data = reinterpret_cast<const u8*>(&t);
            for (auto j = 0; j < sizeof(T); j++) {
                dst.data[i + j] = data[j];
            }
            i += sizeof(T);
            dst.size += sizeof(T);
        }

        void unpack(PushData& dst, i32& i, const BufferIndex& index);

        void unpack(PushData& dst, i32& i, const ImageIndex& index);

        void unpack(PushData& dst, i32& i, const BufferHandle& handle);

        void unpack(PushData& dst, i32& i, const ImageHandle& handle);

        template <typename... Args>
        auto pushConstants(const Args&... args) -> RenderPass& {
            i32 i = 0;
            unpack(_pushData, i, args...);
            return *this;
        }

        auto setCallback(const std::function<void(CommandBuffer&, RenderGraph&, const PushData&)>& callback) -> RenderPass&;

    protected:
        friend RenderGraph;
        friend PassBuilder;
        friend ComputePass;

        Type _type = Type::NONE;

        struct DeferredPushConstant {
            i32 type = 0;
            Edge value;
            i32 offset = 0;
        };
        static auto resolvePushConstants(RenderGraph& graph, PushData data, std::span<DeferredPushConstant> deferredConstants) -> PushData;

        std::vector<DeferredPushConstant> _deferredPushConstants;
        PushData _pushData = {};
        u32 _pushSize = 0;
        std::function<void(CommandBuffer&, RenderGraph&, const PushData&)> _callback = {};
        std::vector<ResourceAccess> _accesses = {};

        std::string _name = {};

    };

    class PassBuilder {
    public:

        PassBuilder(RenderGraph* graph, u32 index);

        auto pass() -> RenderPass&;

        auto read(BufferIndex index, Access access, PipelineStage stage) -> bool;
        auto read(ImageIndex index, Access access, PipelineStage stage, ImageLayout layout) -> bool;

        auto write(BufferIndex index, Access access, PipelineStage stage) -> bool;
        auto write(ImageIndex index, Access access, PipelineStage stage, ImageLayout layout) -> bool;

        template <typename... Args>
        auto pushConstants(Args&&... args) -> PassBuilder& {
            pass().pushConstants(std::forward<Args>(args)...);
            return *this;
        }

        template <typename T>
        auto output(const u32 index = 0) -> std::expected<T, ende::graph::Error> {
            return pass().output<T>(index);
        }

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

        auto addSampledRead(ImageIndex index) -> ComputePass&;

        template <typename... Args>
        auto pushConstants(Args&&... args) -> ComputePass& {
            pass().pushConstants(std::forward<Args>(args)...);
            return *this;
        }

        auto dispatchThreads(u32 x = 1, u32 y = 1, u32 z = 1) -> ComputePass&;
        auto dispatchWorkgroups(u32 x = 1, u32 y = 1, u32 z = 1) -> ComputePass&;
        auto dispatchIndirect(BufferHandle commandBuffer, u32 offset) -> ComputePass&;

    };

    class GraphicsPass : public PassBuilder {
    public:

        GraphicsPass(RenderGraph* graph, u32 index);

        auto addColourRead(ImageIndex index) -> GraphicsPass&;
        auto addColourWrite(ImageIndex index, const ClearValue& clearColor = std::to_array({ 0.f, 0.f, 0.f, 1.f })) -> GraphicsPass&;

        auto addDepthRead(ImageIndex index) -> GraphicsPass&;
        auto addDepthWrite(ImageIndex index, const ClearValue& clearColor = DepthClearValue{ .depth = 1.f, .stencil = 0 }) -> GraphicsPass&;

        auto addStorageImageRead(ImageIndex index, PipelineStage stage) -> GraphicsPass&;
        auto addStorageImageWrite(ImageIndex index, PipelineStage stage) -> GraphicsPass&;

        auto addStorageBufferRead(BufferIndex index, PipelineStage stage) -> GraphicsPass&;
        auto addStorageBufferWrite(BufferIndex index, PipelineStage stage) -> GraphicsPass&;

        auto addSampledRead(ImageIndex index, PipelineStage stage = PipelineStage::NONE) -> GraphicsPass&;

        template <typename... Args>
        auto pushConstants(Args&&... args) -> GraphicsPass& {
            pass().pushConstants(std::forward<Args>(args)...);
            return *this;
        }

        auto draw(u32 count, u32 instanceCount = 1, u32 firstVertex = 0, u32 firstIndex = 0, u32 firstInstance = 0, bool indexed = false) -> GraphicsPass&;
        auto drawIndirect(BufferHandle commands, u32 offset, u32 drawCount, bool indexed = false, u32 stride = 0) -> GraphicsPass&;
        auto drawIndirectCount(BufferHandle commands, u32 offset, BufferHandle countBuffer, u32 countOffset, bool indexed = false, u32 stride = 0) -> GraphicsPass&;

        auto drawMeshTasksThreads(u32 x = 1, u32 y = 1, u32 z = 1) -> GraphicsPass&;
        auto drawMeshTasksWorkgroups(u32 x = 1, u32 y = 1, u32 z = 1) -> GraphicsPass&;
        auto drawMeshTasksIndirect(BufferHandle commands, u32 offset, u32 drawCount, u32 stride = sizeof(VkDrawMeshTasksIndirectCommandEXT)) -> GraphicsPass&;
        auto drawMeshTasksIndirectCount(BufferHandle commands, u32 offset, BufferHandle countBuffer, u32 countOffset, u32 stride = sizeof(VkDrawMeshTasksIndirectCommandEXT)) -> GraphicsPass&;

    };

    class TransferPass : public PassBuilder {
    public:

        TransferPass(RenderGraph* graph, u32 index);

        auto addTransferRead(BufferIndex index) -> TransferPass&;
        auto addTransferWrite(BufferIndex index) -> TransferPass&;
        auto addTransferRead(ImageIndex index) -> TransferPass&;
        auto addTransferWrite(ImageIndex index) -> TransferPass&;

        auto blit(ImageIndex src, ImageIndex dst, Filter filter = Filter::LINEAR) -> TransferPass&;

        auto copy(BufferIndex src, BufferIndex dst, u32 srcOffset = 0, u32 dstOffset = 0, u32 size = 0) -> TransferPass&;
        struct ImageCopy {
            ImageLayout layout = ImageLayout::TRANSFER_DST;
            ende::math::Vec<3, u32> dimensions = { 0, 0, 0 };
            ende::math::Vec<3, u32> offsets = { 0, 0, 0 };
            u32 mipLevel = 0;
            u32 layer = 0;
            u32 layerCount = 1;
            u32 size = 0;
            u32 offset;
        };
        auto copy(BufferIndex src, ImageIndex dst, ImageCopy info) -> TransferPass&;
        auto copy(ImageIndex src, BufferIndex dst, ImageCopy info) -> TransferPass&;

        auto clear(ImageIndex index, const ClearValue& value = std::to_array({0.f, 0.f, 0.f, 1.f})) -> TransferPass&;
        auto clear(BufferIndex index, u32 value = 0, u32 offset = 0, u32 size = 0) -> TransferPass&;

    };

    class HostPass : public PassBuilder {
    public:

        HostPass(RenderGraph* graph, u32 index);

        auto read(BufferIndex index) -> HostPass&;
        auto read(ImageIndex index) -> HostPass&;

        auto write(BufferIndex index) -> HostPass&;
        auto write(ImageIndex index) -> HostPass&;

        auto setCallback(const std::function<void(RenderGraph&)>& callback) -> HostPass&;

    };

    class PresentPass : public PassBuilder {
    public:

        PresentPass(RenderGraph* graph, u32 index);

        auto present(Swapchain* swapchain, ImageIndex index) -> PresentPass&;

    };



    class RenderGraph : public ende::graph::Graph<RenderPass> {
    public:

        // resource management
        auto addBuffer() -> BufferIndex;

        auto addImage() -> ImageIndex;

        auto alias(BufferIndex index) -> BufferIndex;
        auto alias(ImageIndex index) -> ImageIndex;

        auto getBuffer(BufferIndex index) -> BufferHandle;

        auto getImage(ImageIndex index) -> ImageHandle;

        // pass management
        auto compute(std::string_view name) -> ComputePass;

        auto graphics(std::string_view name) -> GraphicsPass;

        auto transfer(std::string_view name) -> TransferPass;

        auto host(std::string_view name) -> HostPass;

        auto present(Swapchain* swapchain, ImageIndex index) -> PresentPass;


        // finalisation

        void setRoot(BufferIndex index);
        void setRoot(ImageIndex index);

        auto compile() -> std::expected<bool, RenderGraphError>;

    private:

        i32 _rootEdge = -1;
        i32 _rootPass = -1;

    };


}


#endif //CANTA_RENDERGRAPHV2_H