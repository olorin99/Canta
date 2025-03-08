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
        INVALID_PIPELINE,
        INVALID_SUBMISSION,
        INVALID_PASS
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
        i32 id = -1;
        u32 index = 0;
    };

    struct BufferIndex {
        i32 id = -1;
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

    struct ResourceAccess {
        i32 id = 0;
        u32 index = 0;
        Access access = Access::NONE;
        PipelineStage stage = PipelineStage::NONE;
        ImageLayout layout = ImageLayout::UNDEFINED;
    };

    class RenderGraph;

    enum class PassType {
        GRAPHICS,
        COMPUTE,
        TRANSFER,
        HOST
    };
    class RenderPass {
    public:

        auto addColourWrite(ImageIndex index, const ClearValue& clearColor = std::to_array({ 0.f, 0.f, 0.f, 1.f })) -> RenderPass&;
        auto addColourRead(ImageIndex index) -> RenderPass&;

        auto addDepthWrite(ImageIndex index, const ClearValue& clearColor = DepthClearValue{ .depth = 1.f, .stencil = 0 }) -> RenderPass&;
        auto addDepthRead(ImageIndex index) -> RenderPass&;

        auto addStorageImageWrite(ImageIndex index, PipelineStage stage) -> RenderPass&;
        auto addStorageImageRead(ImageIndex index, PipelineStage stage) -> RenderPass&;

        auto addStorageBufferWrite(BufferIndex index, PipelineStage stage) -> RenderPass&;
        auto addStorageBufferRead(BufferIndex index, PipelineStage stage) -> RenderPass&;

        auto addSampledRead(ImageIndex index, PipelineStage stage) -> RenderPass&;

        auto addBlitWrite(ImageIndex index) -> RenderPass&;
        auto addBlitRead(ImageIndex index) -> RenderPass&;

        auto addTransferWrite(ImageIndex index) -> RenderPass&;
        auto addTransferRead(ImageIndex index) -> RenderPass&;
        auto addTransferWrite(BufferIndex index) -> RenderPass&;
        auto addTransferRead(BufferIndex index) -> RenderPass&;

        auto addIndirectRead(BufferIndex index) -> RenderPass&;

        auto setPipeline(PipelineHandle handle) -> RenderPass& {
            _pipeline = handle;
            return *this;
        }

        auto setManualPipeline(bool manual = false) -> RenderPass& {
            _manualPipeline = manual;
            return *this;
        }

        auto setExecuteFunction(std::function<void(CommandBuffer&, RenderGraph&)> execute) -> RenderPass& {
            _execute = execute;
            return *this;
        }

        auto setDimensions(u32 width, u32 height) -> RenderPass& {
            _width = width;
            _height = height;
            return *this;
        }

        auto setQueue(QueueType queue) -> RenderPass& {
            _queue = queue;
            return *this;
        }

        auto getQueue() const -> QueueType { return _queue; }

        auto getType() const -> PassType { return _type; }

        auto setGroup(RenderGroup group) -> RenderPass& {
            _group = group;
            return *this;
        }

        auto getGroup() const -> RenderGroup { return _group; }

        auto setDebugColour(const std::array<f32, 4>& colour = { 0, 1, 0, 1 }) -> RenderPass& {
            _debugColour = colour;
            return *this;
        }

        auto aliasImageOutputs() const -> std::vector<ImageIndex>;
        auto aliasBufferOutputs() const -> std::vector<BufferIndex>;

        template <u8 N>
        auto aliasImageOutputs() const -> std::array<ImageIndex, N>;
        template <u8 N>
        auto aliasBufferOutputs() const -> std::array<BufferIndex, N>;

        auto name() const -> std::string_view { return _name; }
        auto inputs() const -> std::span<const ResourceAccess> { return _inputs; }
        auto output() const -> std::span<const ResourceAccess> { return _outputs; }

    private:
        friend RenderGraph;

        auto writes(ImageIndex index, Access access, PipelineStage stage, ImageLayout layout) -> Resource*;
        auto reads(ImageIndex index, Access access, PipelineStage stage, ImageLayout layout) -> Resource*;

        auto writes(BufferIndex index, Access access, PipelineStage stage) -> Resource*;
        auto reads(BufferIndex index, Access access, PipelineStage stage) -> Resource*;

        RenderGraph* _graph = nullptr;

        std::string _name = {};
        PassType _type = PassType::COMPUTE;
        QueueType _queue = QueueType::GRAPHICS;
        std::vector<std::pair<u32, QueueType>> _waits = {};
        bool _signal = false;

        PipelineHandle _pipeline = {};
        bool _manualPipeline = false;

        std::function<void(CommandBuffer&, RenderGraph&)> _execute = {};

        std::vector<ResourceAccess> _inputs = {};
        std::vector<ResourceAccess> _outputs = {};

        struct Attachment {
            i32 index = -1;
            ImageLayout layout = ImageLayout::UNDEFINED;
            ClearValue clearColor = std::to_array({ 0.f, 0.f, 0.f, 1.f });
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
            QueueType srcQueue = QueueType::NONE;
            QueueType dstQueue = QueueType::NONE;
        };
        std::vector<Barrier> _barriers = {};
        std::vector<Barrier> _releaseBarriers = {};

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
            bool multiQueue = true;
            bool allowHostPasses = true;
            std::string_view name = {};
        };

        static auto create(CreateInfo info) -> RenderGraph;

        RenderGraph() = default;

        auto addPass(std::string_view name, PassType type = PassType::COMPUTE, RenderGroup group = {}, bool manualPipeline = false) -> RenderPass&;
        auto addPass(RenderPass&& pass) -> RenderPass&;
        auto addClearPass(std::string_view name, ImageIndex index, const ClearValue& value = std::to_array({ 0.f, 0.f, 0.f, 1.f }), RenderGroup group = {}) -> RenderPass&;
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

        void setBackbuffer(ImageIndex index, ImageLayout finalLayout = ImageLayout::UNDEFINED);
        void setBackbuffer(BufferIndex index);

        void reset();
        auto compile() -> std::expected<bool, RenderGraphError>;
        auto execute(std::span<SemaphorePair> waits, std::span<SemaphorePair> signals, std::span<ImageBarrier> imagesToAcquire = {}) -> std::expected<bool, RenderGraphError>;

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

        auto multiQueueEnabled() const -> bool { return _multiQueue; }
        void setMultiQueueEnabled(bool enabled) { _multiQueue = enabled; }

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

        auto resources() const -> std::span<const std::unique_ptr<Resource>> { return _resources; }
        auto buffers() const -> std::span<const BufferHandle> { return _buffers; }
        auto images() const -> std::span<const ImageHandle> { return _images; }

        auto orderedPasses() -> std::span<RenderPass*> { return _orderedPasses; }

    private:
        friend RenderPass;

        // helpers for execution function
//        auto selectCommandBuffer(RenderPass& pass) -> CommandBuffer*;
        void submitBarriers(CommandBuffer& commandBuffer, const std::vector<RenderPass::Barrier>& barriers);
        void startQueries(CommandBuffer& commandBuffer, u32 passIndex, RenderPass& pass, RenderGroup& currentGroup);
        void endQueries(CommandBuffer& commandBuffer, u32 passIndex, RenderPass& pass);

        // helpers for compile function
        void buildBarriers();
        void buildResources();
        void buildRenderAttachments();

        Device* _device = nullptr;
        std::string _name = {};
        bool _multiQueue = true;
        bool _allowHostPasses = true;

        i32 _backbufferId = -1;
        i32 _backbufferIndex = -1;
        ImageLayout _backbufferFinalLayout = ImageLayout::UNDEFINED;
        RenderPass::Barrier _backbufferBarrier = {};

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
        i32 _resourceId = 0;

        std::vector<ImageHandle> _images = {};
        std::vector<BufferHandle> _buffers = {};

        std::array<std::array<CommandPool, 2>, FRAMES_IN_FLIGHT> _commandPools = {};
        SemaphoreHandle _cpuTimeline = {};

    };

    template <u8 N>
    auto RenderPass::aliasImageOutputs() const -> std::array<ImageIndex, N> {
        std::array<ImageIndex, N> aliases = {};
        for (u32 i = 0; auto& output : _outputs) {
            if (i >= N) break;
            auto& resource = _graph->_resources[output.index];
            if (resource->type == ResourceType::IMAGE)
                aliases[i++] = _graph->addAlias(ImageIndex{ .id = output.id, .index = output.index });
        }
        return aliases;
    }

    template <u8 N>
    auto RenderPass::aliasBufferOutputs() const -> std::array<BufferIndex, N> {
        std::array<BufferIndex, N> aliases = {};
        for (u32 i = 0; auto& output : _outputs) {
            if (i >= N) break;
            auto& resource = _graph->_resources[output.index];
            if (resource->type == ResourceType::BUFFER)
                aliases[i++] = _graph->addAlias(BufferIndex{ .id = output.id, .index = output.index });
        }
        return aliases;
    }

}

#endif //CANTA_RENDERGRAPH_H
