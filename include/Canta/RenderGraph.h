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

        auto addColourWrite(const ImageIndex index, const ClearValue& clearColor = std::to_array({ 0.f, 0.f, 0.f, 1.f })) -> RenderPass&;
        auto addColourRead(const ImageIndex index) -> RenderPass&;

        auto addDepthWrite(const ImageIndex index, const ClearValue& clearColor = DepthClearValue{ .depth = 1.f, .stencil = 0 }) -> RenderPass&;
        auto addDepthRead(const ImageIndex index) -> RenderPass&;

        auto addStorageImageWrite(const ImageIndex index, const PipelineStage stage) -> RenderPass&;
        auto addStorageImageRead(const ImageIndex index, const PipelineStage stage) -> RenderPass&;

        auto addStorageBufferWrite(const BufferIndex index, const PipelineStage stage) -> RenderPass&;
        auto addStorageBufferRead(const BufferIndex index, const PipelineStage stage) -> RenderPass&;

        auto addSampledRead(const ImageIndex index, const PipelineStage stage) -> RenderPass&;

        auto addBlitWrite(const ImageIndex index) -> RenderPass&;
        auto addBlitRead(const ImageIndex index) -> RenderPass&;

        auto addTransferWrite(const ImageIndex index) -> RenderPass&;
        auto addTransferRead(const ImageIndex index) -> RenderPass&;
        auto addTransferWrite(const BufferIndex index) -> RenderPass&;
        auto addTransferRead(const BufferIndex index) -> RenderPass&;

        auto addIndirectRead(const BufferIndex index) -> RenderPass&;

        auto setPipeline(const PipelineHandle &handle) -> RenderPass& {
            _pipeline = handle;
            return *this;
        }

        auto setManualPipeline(const bool manual = false) -> RenderPass& {
            _manualPipeline = manual;
            return *this;
        }

        template <typename T, typename U, typename... Args>
        void unpack(std::array<u8, 192>& dst, i32& i, const T& t, const U& u, const Args&... args) {
            unpack(dst, i, t);
            unpack(dst, i, u, args...);
        }

        template <typename T>
        void unpack(std::array<u8, 192>& dst, i32& i, const T& arg) {
            auto* data = reinterpret_cast<const u8*>(&arg);
            for (auto j = 0; j < sizeof(arg); j++) {
                dst[i + j] = data[j];
            }
            i += sizeof(arg);
            _pushConstantSize += sizeof(arg);
            assert(_pushConstantSize <= 128);
        }

        void unpack(std::array<u8, 192>& dst, i32&, const ImageIndex& image);

        void unpack(std::array<u8, 192>& dst, i32&, const BufferIndex& image);

        void unpack(std::array<u8, 192>& dst, i32&, const ImageHandle& image);

        void unpack(std::array<u8, 192>& dst, i32&, const BufferHandle& image);

        template <typename... Args>
        auto pushConstants(const Args&... args) -> RenderPass& {
            _pushConstantSize = 0;
            i32 i = 0;
            unpack(_pushConstants, i, args...);
            return *this;
        }

        auto dispatchWorkgroups(u32 x = 1, u32 y = 1, u32 z = 1) -> RenderPass&;
        auto dispatchThreads(u32 x = 1, u32 y = 1, u32 z = 1) -> RenderPass&;

        auto setExecuteFunction(const std::function<void(CommandBuffer&, RenderGraph&)> &execute) -> RenderPass& {
            _execute = execute;
            return *this;
        }

        auto setDimensions(const u32 width, const u32 height) -> RenderPass& {
            _width = width;
            _height = height;
            return *this;
        }

        auto setQueue(const QueueType queue) -> RenderPass& {
            _queue = queue;
            return *this;
        }

        auto getQueue() const -> QueueType { return _queue; }

        auto getType() const -> PassType { return _type; }

        auto setGroup(const RenderGroup group) -> RenderPass& {
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

        struct Barrier {
            u32 index = 0;
            i32 passIndex = 0;
            PipelineStage srcStage = PipelineStage::TOP;
            PipelineStage dstStage = PipelineStage::BOTTOM;
            Access srcAccess = Access::NONE;
            Access dstAccess = Access::NONE;
            ImageLayout srcLayout = ImageLayout::UNDEFINED;
            ImageLayout dstLayout = ImageLayout::UNDEFINED;
            QueueType srcQueue = QueueType::NONE;
            QueueType dstQueue = QueueType::NONE;
        };
        auto barriers() const -> std::span<const Barrier> { return _barriers; }

    private:
        friend RenderGraph;

        auto writes(const ImageIndex index, const Access access, const PipelineStage stage, const ImageLayout layout) -> Resource*;
        auto reads(const ImageIndex index, const Access access, const PipelineStage stage, const ImageLayout layout) -> Resource*;

        auto writes(const BufferIndex index, const Access access, const PipelineStage stage) -> Resource*;
        auto reads(const BufferIndex index, const Access access, const PipelineStage stage) -> Resource*;

        RenderGraph* _graph = nullptr;

        std::string _name = {};
        PassType _type = PassType::COMPUTE;
        QueueType _queue = QueueType::GRAPHICS;
        std::vector<std::pair<u32, QueueType>> _waits = {};
        bool _signal = false;

        PipelineHandle _pipeline = {};
        bool _manualPipeline = false;
        std::array<u8, 192> _pushConstants = {};
        u32 _pushConstantSize = 0;

        struct DeferredPushConstant {
            i32 type = 0; // 0 == image, 1 == buffer
            i32 id = -1;
            u32 index = 0;
            i32 offset = 0;
        };
        std::vector<DeferredPushConstant> _deferredPushConstants = {};

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

        static auto create(const CreateInfo &info) -> RenderGraph;

        RenderGraph() = default;

        struct PassInfo {
            std::string_view name;
            PassType type = PassType::COMPUTE;
            QueueType queue = QueueType::GRAPHICS;
            RenderGroup group = {};
            bool manualPipeline = false;
        };
        auto addPass(PassInfo info) -> RenderPass&;
        auto addPass(RenderPass&& pass) -> RenderPass&;
        auto addClearPass(const std::string_view name, const ImageIndex index, const ClearValue& value = std::to_array({ 0.f, 0.f, 0.f, 1.f }), const RenderGroup group = {}) -> RenderPass&;
        auto addBlitPass(const std::string_view name, const ImageIndex src, const ImageIndex dst, const Filter filter = Filter::LINEAR, const RenderGroup group = {}) -> RenderPass&;

        auto getGroup(const std::string_view name, const std::array<f32, 4>& colour = { 0, 1, 0, 1 }) -> RenderGroup;
        auto getGroupName(const RenderGroup) -> std::string;
        auto getGroupColour(const RenderGroup) -> std::array<f32, 4>;

        auto addImage(const ImageDescription &description) -> ImageIndex;
        auto addBuffer(const BufferDescription &description) -> BufferIndex;

        auto addAlias(const ImageIndex index) -> ImageIndex;
        auto addAlias(const BufferIndex index) -> BufferIndex;

        auto getImage(const ImageIndex index) -> ImageHandle;
        auto getBuffer(const BufferIndex index) -> BufferHandle;

        void setBackbuffer(const ImageIndex index, const ImageLayout finalLayout = ImageLayout::UNDEFINED);
        void setBackbuffer(const BufferIndex index);

        void reset();
        auto compile() -> std::expected<bool, RenderGraphError>;
        auto execute(std::span<SemaphorePair> waits, std::span<SemaphorePair> signals, std::span<ImageBarrier> imagesToAcquire = {}, bool synchronous = false) -> std::expected<bool, RenderGraphError>;

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
        void setMultiQueueEnabled(const bool enabled) { _multiQueue = enabled; }

        auto timingEnabled() const -> bool { return _timingEnabled; }
        auto pipelineStatisticsEnabled() const -> bool { return _pipelineStatisticsEnabled; }

        auto timingMode() const -> TimingMode { return _timingMode; }
        auto individualPipelineStatistics() const -> bool { return _individualPipelineStatistics; }

        void setTimingEnabled(const bool enabled) { _timingEnabled = enabled; }
        void setPipelineStatisticsEnabled(const bool enabled) { _pipelineStatisticsEnabled = enabled; }

        void setTimingMode(const TimingMode mode) { _timingMode = mode; }
        void setIndividualPipelineStatistics(const bool individual) { _individualPipelineStatistics = individual; }

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
            const auto& resource = _graph->_resources[output.index];
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
            const auto& resource = _graph->_resources[output.index];
            if (resource->type == ResourceType::BUFFER)
                aliases[i++] = _graph->addAlias(BufferIndex{ .id = output.id, .index = output.index });
        }
        return aliases;
    }

}

#endif //CANTA_RENDERGRAPH_H
