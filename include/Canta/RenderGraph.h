#ifndef CANTA_RENDERGRAPH_H
#define CANTA_RENDERGRAPH_H

#include <Ende/platform.h>
#include <expected>
#include <string>
#include <vector>
#include <Canta/Device.h>

namespace canta {

    enum class RenderGraphError {
        CYCLICAL_GRAPH,
    };

    enum ResourceType {
        IMAGE = 0,
        BUFFER = 1
    };

    struct Resource {
        virtual ~Resource() = default;

        u32 index = 0;
        ResourceType type = ResourceType::IMAGE;
        std::string name = {};
    };

    struct ImageResource : public Resource {
        bool matchesBackbuffer = true;
        u32 imageIndex = 0;
        u32 width = 1;
        u32 height = 1;
        u32 depth = 1;
        u32 mipLevels = 1;
        Format format = Format::RGBA8_UNORM;
        ImageUsage usage = ImageUsage::SAMPLED | ImageUsage::TRANSFER_SRC;
    };

    struct BufferResource : public Resource {
        u32 bufferIndex = 0;
        u32 size = 0;
        BufferUsage usage = BufferUsage::STORAGE;
    };

    struct ImageIndex {
        u32 id = 0;
        u32 index = 0;
    };

    struct BufferIndex {
        u32 id = 0;
        u32 index = 0;
    };

    struct ImageDescription {
        bool matchesBackbuffer = true;
        u32 width = 1;
        u32 height = 1;
        u32 depth = 1;
        u32 mipLevels = 1;
        Format format = Format::RGBA8_UNORM;
        ImageUsage usage = ImageUsage::SAMPLED | ImageUsage::TRANSFER_SRC;
        ImageHandle handle = {};
        std::string_view name = {};
    };

    struct BufferDescription {
        u32 size = 0;
        BufferUsage usage = BufferUsage::STORAGE;
        BufferHandle handle = {};
        std::string_view name = {};
    };

    class RenderGraph;

    class RenderPass {
    public:

        enum class Type {
            GRAPHICS,
            COMPUTE,
            TRANSFER
        };

        void addColourWrite(ImageIndex index, const std::array<f32, 4>& clearColor = { 0, 0, 0, 1 });
        void addColourRead(ImageIndex index);

        void addDepthWrite(ImageIndex index);
        void addDepthRead(ImageIndex index);

        void addStorageImageWrite(ImageIndex index, PipelineStage stage);
        void addStorageImageRead(ImageIndex index, PipelineStage stage);

        void addStorageBufferWrite(BufferIndex index, PipelineStage stage);
        void addStorageBufferRead(BufferIndex index, PipelineStage stage);

        void addSampledRead(ImageIndex index, PipelineStage stage);

        void addBlitWrite(ImageIndex index);
        void addBlitRead(ImageIndex index);

        void addTransferWrite(ImageIndex index);
        void addTransferRead(ImageIndex index);
        void addTransferWrite(BufferIndex index);
        void addTransferRead(BufferIndex index);

        void setExecuteFunction(std::function<void(CommandBuffer&, RenderGraph&)> execute) {
            _execute = execute;
        }

        void setDimensions(u32 width, u32 height) {
            _width = width;
            _height = height;
        }

    private:
        friend RenderGraph;

        auto writes(ImageIndex index, Access access, PipelineStage stage, ImageLayout layout) -> Resource*;
        auto reads(ImageIndex index, Access access, PipelineStage stage, ImageLayout layout) -> Resource*;

        auto writes(BufferIndex index, Access access, PipelineStage stage) -> Resource*;
        auto reads(BufferIndex index, Access access, PipelineStage stage) -> Resource*;

        RenderGraph* _graph = nullptr;

        std::string _name = {};
        Type _type = Type::COMPUTE;

        std::function<void(CommandBuffer&, RenderGraph&)> _execute = {};

        struct ResourceAccess {
            u32 id = 0;
            u32 index = 0;
            Access access = Access::NONE;
            PipelineStage stage = PipelineStage::NONE;
            ImageLayout layout = ImageLayout::UNDEFINED;
        };

        std::vector<ResourceAccess> _inputs = {};
        std::vector<ResourceAccess> _outputs = {};

        struct Attachment {
            i32 index = -1;
            ImageLayout layout = ImageLayout::UNDEFINED;
            std::array<f32, 4> clearColor = { 0, 0, 0, 1 };
        };
        std::vector<Attachment> _colourAttachments = {};
        Attachment _depthAttachment = {};

        std::vector<canta::Attachment> _renderingColourAttachments = {};
        canta::Attachment _renderingDepthAttachment = {};

        i32 _width = -1;
        i32 _height = -1;

        struct Barrier {
            u32 index = 0;
            PipelineStage srcStage = PipelineStage::TOP;
            PipelineStage dstStage = PipelineStage::BOTTOM;
            Access srcAccess = Access::NONE;
            Access dstAccess = Access::NONE;
            ImageLayout srcLayout = ImageLayout::UNDEFINED;
            ImageLayout dstLayout = ImageLayout::UNDEFINED;
        };
        std::vector<Barrier> _barriers = {};

    };

    class RenderGraph {
    public:

        struct CreateInfo {
            Device* device = nullptr;
        };

        static auto create(CreateInfo info) -> RenderGraph;

        auto addPass(std::string_view name, RenderPass::Type type = RenderPass::Type::COMPUTE) -> RenderPass&;
        auto addClearPass(std::string_view name, ImageIndex index) -> RenderPass&;
        auto addBlitPass(std::string_view name, ImageIndex src, ImageIndex dst) -> RenderPass&;

        auto addImage(ImageDescription description) -> ImageIndex;
        auto addBuffer(BufferDescription description) -> BufferIndex;

        auto addAlias(ImageIndex index) -> ImageIndex;
        auto addAlias(BufferIndex index) -> BufferIndex;

        auto getImage(ImageIndex index) -> ImageHandle;
        auto getBuffer(BufferIndex index) -> BufferHandle;

        void setBackbuffer(ImageIndex index);
        void setBackbuffer(BufferIndex index);

        void reset();
        auto compile() -> std::expected<bool, RenderGraphError>;
        auto execute(CommandBuffer& cmd) -> std::expected<bool, RenderGraphError>;

    private:
        friend RenderPass;

        void buildBarriers();
        void buildResources();
        void buildRenderAttachments();

        Device* _device = nullptr;

        i32 _backbufferId = -1;
        i32 _backbufferIndex = -1;

        std::vector<RenderPass> _passes = {};
        std::vector<RenderPass*> _orderedPasses = {};

        tsl::robin_map<const char*, u32> _nameToIndex;

        std::vector<std::unique_ptr<Resource>> _resources = {};
        u32 _resourceId = 0;

        std::vector<ImageHandle> _images = {};
        std::vector<BufferHandle> _buffers = {};

    };

}

#endif //CANTA_RENDERGRAPH_H
