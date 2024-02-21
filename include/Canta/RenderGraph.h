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
        ImageLayout initialLayout = ImageLayout::UNDEFINED;
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

    struct RenderGroup {
        i32 id = -1;
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
        ImageLayout initialLayout = ImageLayout::UNDEFINED;
        std::string_view name = {};
    };

    struct BufferDescription {
        u32 size = 0;
        BufferUsage usage = BufferUsage::STORAGE;
        BufferHandle handle = {};
        std::string_view name = {};
    };

    class RenderGraph;

    enum class PassType {
        GRAPHICS,
        COMPUTE,
        TRANSFER
    };
    class RenderPass {
    public:

        void addColourWrite(ImageIndex index, const std::array<f32, 4>& clearColor = { 0, 0, 0, 1 });
        void addColourRead(ImageIndex index);

        void addDepthWrite(ImageIndex index, const std::array<f32, 4>& clearColor = { 1, 0, 0, 0 });
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

        void addIndirectRead(BufferIndex index);

        void setExecuteFunction(std::function<void(CommandBuffer&, RenderGraph&)> execute) {
            _execute = execute;
        }

        void setDimensions(u32 width, u32 height) {
            _width = width;
            _height = height;
        }

        void setGroup(RenderGroup group) {
            _group = group;
        }

        auto getGroup() -> RenderGroup { return _group; }

        void setDebugColour(const std::array<f32, 4>& colour = { 0, 1, 0, 1 }) {
            _debugColour = colour;
        }

    private:
        friend RenderGraph;

        auto writes(ImageIndex index, Access access, PipelineStage stage, ImageLayout layout) -> Resource*;
        auto reads(ImageIndex index, Access access, PipelineStage stage, ImageLayout layout) -> Resource*;

        auto writes(BufferIndex index, Access access, PipelineStage stage) -> Resource*;
        auto reads(BufferIndex index, Access access, PipelineStage stage) -> Resource*;

        RenderGraph* _graph = nullptr;

        std::string _name = {};
        PassType _type = PassType::COMPUTE;

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

        RenderGroup _group = {};
        std::array<f32, 4> _debugColour = { 0, 1, 0, 1 };

    };

    class RenderGraph {
    public:

        enum class TimingMode {
            PER_PASS,
            PER_GROUP,
            SINGLE
        };

        struct CreateInfo {
            Device* device = nullptr;
            bool enableTiming = true;
            TimingMode timingMode = TimingMode::PER_PASS;
            bool enablePipelineStatistics = true;
            bool individualPipelineStatistics = false;
            std::string_view name = {};
        };

        static auto create(CreateInfo info) -> RenderGraph;

        RenderGraph() = default;

        auto addPass(std::string_view name, PassType type = PassType::COMPUTE, RenderGroup group = {}) -> RenderPass&;
        auto addClearPass(std::string_view name, ImageIndex index, const std::array<f32, 4>& value = { 0, 0, 0, 1 }, RenderGroup group = {}) -> RenderPass&;
        auto addBlitPass(std::string_view name, ImageIndex src, ImageIndex dst, Filter filter = Filter::LINEAR, RenderGroup group = {}) -> RenderPass&;

        auto getGroup(std::string_view name, const std::array<f32, 4>& colour = { 0, 1, 0, 1 }) -> RenderGroup;
        auto getGroupName(RenderGroup) -> std::string;
        auto getGroupColour(RenderGroup) -> std::array<f32, 4>;

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
        auto execute(std::span<Semaphore::Pair> waits, std::span<Semaphore::Pair> signals, bool backbufferIsSwapchain = false, std::span<ImageBarrier> imagesToAcquire = {}) -> std::expected<bool, RenderGraphError>;

        auto timers() -> std::span<std::pair<std::string, Timer>> {
            std::size_t count = _timerCount;
            if (!_timingEnabled) count = 0;
            if (_timingEnabled && _timingMode == TimingMode::SINGLE) count = 1;
            count = std::min(_timers[_device->flyingIndex()].size(), count);
            return { _timers[_device->flyingIndex()].data(), count };
        }
        auto pipelineStatistics() -> std::span<std::pair<std::string, PipelineStatistics>> {
            auto count = _orderedPasses.size();
            if (!_pipelineStatisticsEnabled) count = 0;
            if (_pipelineStatisticsEnabled && !_individualPipelineStatistics) count = 1;
            count = std::min(_pipelineStats[_device->flyingIndex()].size(), count);
            return std::span(_pipelineStats[_device->flyingIndex()].data(), count);
        }

        auto timingEnabled() const -> bool { return _timingEnabled; }
        auto pipelineStatisticsEnabled() const -> bool { return _pipelineStatisticsEnabled; }

        auto timingMode() const -> TimingMode { return _timingMode; }
        auto individualPipelineStatistics() const -> bool { return _individualPipelineStatistics; }

        void setTimingEnabled(bool enabled) { _timingEnabled = enabled; }
        void setPipelineStatisticsEnabled(bool enabled) { _pipelineStatisticsEnabled = enabled; }

        void setTimingMode(TimingMode mode) { _timingMode = mode; }
        void setIndividualPipelineStatistics(bool individual) { _individualPipelineStatistics = individual; }

        struct Statistics {
            u32 passes = 0;
            u32 resources = 0;
            u32 images = 0;
            u32 buffers = 0;
            u32 commandBuffers = 0;
        };
        auto statistics() const -> Statistics;

    private:
        friend RenderPass;

        void buildBarriers();
        void buildResources();
        void buildRenderAttachments();

        Device* _device = nullptr;
        std::string _name = {};

        i32 _backbufferId = -1;
        i32 _backbufferIndex = -1;

        std::vector<RenderPass> _passes = {};
        std::vector<RenderPass*> _orderedPasses = {};

        bool _timingEnabled = true;
        TimingMode _timingMode = TimingMode::PER_PASS;
        std::array<std::vector<std::pair<std::string, Timer>>, FRAMES_IN_FLIGHT> _timers = {};
        u32 _timerCount = 0;
        bool _pipelineStatisticsEnabled = true;
        bool _individualPipelineStatistics = true;
        std::array<std::vector<std::pair<std::string, PipelineStatistics>>, FRAMES_IN_FLIGHT> _pipelineStats = {};

        tsl::robin_map<std::string_view, std::pair<i32, std::array<f32, 4>>> _renderGroups;
        u32 _groupId = 0;
        tsl::robin_map<const char*, u32> _nameToIndex;

        std::vector<std::unique_ptr<Resource>> _resources = {};
        u32 _resourceId = 0;

        std::vector<ImageHandle> _images = {};
        std::vector<BufferHandle> _buffers = {};

        std::array<CommandPool, FRAMES_IN_FLIGHT> _commandPools = {};

    };

}

#endif //CANTA_RENDERGRAPH_H
