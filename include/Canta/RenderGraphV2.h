#ifndef CANTA_RENDERGRAPHV2_H
#define CANTA_RENDERGRAPHV2_H

#include <Ende/platform.h>
#include <Ende/graph/graph.h>
#include <expected>
#include <Canta/Device.h>
#include <Canta/RenderGraphV2.h>
#include <Ende/thread/ThreadPool.h>

namespace canta {
    class ImGuiContext;
}

namespace canta::V2 {

    enum class RenderGraphError {
        NONE,
        IS_CYCLICAL,
        NO_ROOT,
        INVALID_PASS,
        INVALID_PASS_COUNT,
        INVALID_RESOURCE,
        INVALID_RESOURCE_USE,
        PASS_RUN,
        DEVICE_ERROR,
    };

    auto mapGraphErrorToRenderGraphError(ende::graph::Error error) -> RenderGraphError;

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

    struct BufferInfo {
        u32 size = 0;
        BufferUsage usage = BufferUsage::STORAGE;
        MemoryType type = MemoryType::DEVICE;
        bool external = false;
        BufferHandle buffer = {};
        std::string name = {};
    };

    struct ImageInfo {
        u32 width = 1;
        u32 height = 1;
        u32 depth = 1;
        u32 mips = 1;
        Format format = Format::RGBA8_UNORM;
        ImageUsage usage = ImageUsage::STORAGE;
        bool external = false;
        ImageHandle image = {};
        bool swapchainImage = false;
        std::string name = {};
    };

    struct RenderGroup {
        i32 id = -1;
        std::array<f32, 4> colour = { 0, 0, 0, 1 };
        std::string name = {};
    };

    class RenderGraph;
    class PassBuilder;
    class ComputePass;
    class GraphicsPass;
    class HostPass;
    class PresentPass;

    template <typename T>
    struct Read {
        explicit Read(T r) : resource(std::move(r)) {}
        T resource;
    };

    template <typename T>
    struct Write {
        explicit Write(T r) : resource(std::move(r)) {}
        T resource;
    };

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

        auto dimensions() const -> ende::math::Vec<2, u32> { return _dimensions; }

        auto type() const -> Type { return _type; }

        auto queue() const -> QueueType { return _queueType; }

        auto name() const -> std::string_view { return _name; }

        auto group() const -> RenderGroup { return _group; }

        void setGroup(const RenderGroup &group) { _group = group; }

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

        template <std::ranges::range Range>
        auto unpack(PushData& dst, i32& i, const Range& range) {
            for (auto& t : range) {
                unpack(dst, i, t);
            }
        }

        template <typename... Args>
        auto pushConstants(const Args&... args) -> RenderPass& {
            i32 i = 0;
            unpack(_pushData, i, args...);
            return *this;
        }

        auto setPipeline(const PipelineHandle &pipeline) -> RenderPass&;
        auto setManualPipeline(const bool state) -> RenderPass& { _manualPipeline = state; return *this; }

        // auto setCallback(const std::function<void(CommandBuffer&, RenderGraph&, const PushData&)>& callback) -> RenderPass&;
        auto setCallback(const std::function<std::expected<bool, RenderGraphError>(CommandBuffer&, RenderGraph&, const PushData&)>& callback) -> RenderPass&;

        auto run(RenderGraph& graph, CommandBuffer& commands) const -> std::expected<bool, RenderGraphError>;

    protected:
        friend RenderGraph;
        friend PassBuilder;
        friend ComputePass;
        friend GraphicsPass;

        Type _type = Type::NONE;
        QueueType _queueType = QueueType::GRAPHICS;

        auto inputResourceIndex(u32 i) const -> u32;
        auto outputResourceIndex(u32 i) const -> u32;

        struct DeferredPushConstant {
            i32 type = 0;
            Edge value;
            i32 offset = 0;
        };
        static auto resolvePushConstants(const RenderGraph& graph, PushData data, std::span<const DeferredPushConstant> deferredConstants) -> std::expected<PushData, RenderGraphError>;

        void mergeAccesses();

        PipelineHandle _pipeline = {};
        bool _manualPipeline = false;

        std::vector<DeferredPushConstant> _deferredPushConstants;
        PushData _pushData = {};
        std::function<std::expected<bool, RenderGraphError>(CommandBuffer&, RenderGraph&, const PushData&)> _callback = {};
        std::vector<ResourceAccess> _accesses = {};

        ende::math::Vec<2, u32> _dimensions = { 0, 0 };
        struct Attachment {
            i32 index = -1;
            ImageLayout layout = ImageLayout::UNDEFINED;
            ClearValue clearColor = std::to_array({ 0.f, 0.f, 0.f, 1.f });
        };
        std::vector<Attachment> _colourAttachments = {};
        Attachment _depthAttachment = {};

        std::vector<canta::Attachment> _renderingColourAttachments = {};
        canta::Attachment _renderingDepthAttachment = {};

        struct Barrier {
            i32 index = 0;
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
        std::vector<Barrier> _barriers = {};
        std::vector<std::pair<i32, QueueType>> _queueWaits = {};

        RenderGroup _group = {};

        std::string _name = {};

    };

    class PassBuilder {
    public:

        PassBuilder(RenderGraph* graph, u32 index);

        auto pass() const -> RenderPass&;

        auto group() const -> RenderGroup { return pass().group(); }

        void setGroup(const RenderGroup &group) const { pass().setGroup(group); }

        auto pipeline(const PipelineHandle &pipeline) -> PassBuilder&;
        auto setManualPipeline(bool state) -> PassBuilder&;

        auto read(BufferIndex index, Access access, PipelineStage stage) -> std::expected<BufferInfo, RenderGraphError>;
        auto read(ImageIndex index, Access access, PipelineStage stage, ImageLayout layout) -> std::expected<ImageInfo, RenderGraphError>;

        auto write(BufferIndex index, Access access, PipelineStage stage) -> std::expected<BufferInfo, RenderGraphError>;
        auto write(ImageIndex index, Access access, PipelineStage stage, ImageLayout layout) -> std::expected<ImageInfo, RenderGraphError>;


        template <typename T, typename U, typename... Args>
        void unpack(RenderPass::PushData& dst, i32& i, const T& t, const U& u, const Args&... args) {
            unpack(dst, i, t);
            unpack(dst, i, u, args...);
        }

        template <typename T>
        void unpack(RenderPass::PushData& dst, i32& i, const T& t) {
            auto* data = reinterpret_cast<const u8*>(&t);
            for (auto j = 0; j < sizeof(T); j++) {
                dst.data[i + j] = data[j];
            }
            i += sizeof(T);
            dst.size += sizeof(T);
        }

        void unpack(RenderPass::PushData& dst, i32& i, const BufferIndex& index);

        void unpack(RenderPass::PushData& dst, i32& i, const ImageIndex& index);

        void unpack(RenderPass::PushData& dst, i32& i, const BufferHandle& handle);

        void unpack(RenderPass::PushData& dst, i32& i, const ImageHandle& handle);

        template <std::ranges::range Range>
        auto unpack(RenderPass::PushData& dst, i32& i, const Range& range) {
            for (auto& t : range) {
                unpack(dst, i, t);
            }
        }

        void unpack(RenderPass::PushData& dst, i32&, const Read<ImageIndex>& image);

        void unpack(RenderPass::PushData& dst, i32&, const Read<BufferIndex>& buffer);

        void unpack(RenderPass::PushData& dst, i32&, const Write<ImageIndex>& image);

        void unpack(RenderPass::PushData& dst, i32&, const Write<BufferIndex>& buffer);

        template <typename... Args>
        auto pushConstants(Args&&... args) -> PassBuilder& {
            i32 i = 0;
            unpack(pass()._pushData, i, args...);
            return *this;
        }

        template <typename T>
        auto output(const u32 index = 0) -> std::expected<T, RenderGraphError> {
            if (_error) return std::unexpected(_error.value());
            return pass().output<T>(index).transform_error(mapGraphErrorToRenderGraphError);
        }

        auto setCallback(const std::function<std::expected<bool, RenderGraphError>(CommandBuffer&, RenderGraph&, const RenderPass::PushData&)>& callback) -> PassBuilder&;


    // protected:

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
        friend GraphicsPass;
        friend ComputePass;
        friend HostPass;
        friend PresentPass;

        RenderGraph* _graph = nullptr;
        u32 _vertexIndex = 0;

        std::optional<RenderGraphError> _error = {};

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
            i32 i = 0;
            unpack(pass()._pushData, i, args...);
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

        auto addSampledRead(ImageIndex index, PipelineStage stage) -> GraphicsPass&;

        template <typename... Args>
        auto pushConstants(Args&&... args) -> GraphicsPass& {
            i32 i = 0;
            unpack(pass()._pushData, i, args...);
            return *this;
        }

        auto draw(u32 count, u32 instanceCount = 1, u32 firstVertex = 0, u32 firstIndex = 0, u32 firstInstance = 0, bool indexed = false) -> GraphicsPass&;
        auto drawIndirect(BufferHandle commands, u32 offset, u32 drawCount, bool indexed = false, u32 stride = 0) -> GraphicsPass&;
        auto drawIndirectCount(BufferHandle commands, u32 offset, BufferHandle countBuffer, u32 countOffset, bool indexed = false, u32 stride = 0) -> GraphicsPass&;

        auto drawMeshTasksThreads(u32 x = 1, u32 y = 1, u32 z = 1) -> GraphicsPass&;
        auto drawMeshTasksWorkgroups(u32 x = 1, u32 y = 1, u32 z = 1) -> GraphicsPass&;
        auto drawMeshTasksIndirect(BufferHandle commands, u32 offset, u32 drawCount, u32 stride = sizeof(VkDrawMeshTasksIndirectCommandEXT)) -> GraphicsPass&;
        auto drawMeshTasksIndirectCount(BufferHandle commands, u32 offset, BufferHandle countBuffer, u32 countOffset, u32 stride = sizeof(VkDrawMeshTasksIndirectCommandEXT)) -> GraphicsPass&;

        auto blit(ImageIndex src, ImageIndex dst, Filter filter = Filter::LINEAR) -> GraphicsPass&;


        auto imgui(ImGuiContext& context, ImageIndex image) -> std::expected<ImageIndex, RenderGraphError>;
    };

    class TransferPass : public PassBuilder {
    public:

        TransferPass(RenderGraph* graph, u32 index);

        auto addTransferRead(BufferIndex index) -> TransferPass&;
        auto addTransferWrite(BufferIndex index) -> TransferPass&;
        auto addTransferRead(ImageIndex index) -> TransferPass&;
        auto addTransferWrite(ImageIndex index) -> TransferPass&;

        auto copy(BufferIndex src, BufferIndex dst, u32 srcOffset = 0, u32 dstOffset = 0, u32 size = 0) -> std::expected<BufferIndex, RenderGraphError>;
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
        auto copy(BufferIndex src, ImageIndex dst, const ImageCopy& info) -> std::expected<ImageIndex, RenderGraphError>;
        auto copy(ImageIndex src, BufferIndex dst, ImageCopy info) -> std::expected<BufferIndex, RenderGraphError>;

        auto clear(ImageIndex index, const ClearValue& value = std::to_array({0.f, 0.f, 0.f, 1.f})) -> std::expected<ImageIndex, RenderGraphError>;
        auto clear(BufferIndex index, u32 value = 0, u32 offset = 0, u32 size = 0) -> std::expected<BufferIndex, RenderGraphError>;

    };

    class HostPass : public PassBuilder {
    public:

        HostPass(RenderGraph* graph, u32 index);

        auto read(BufferIndex index) -> HostPass&;
        auto read(ImageIndex index) -> HostPass&;

        auto write(BufferIndex index) -> HostPass&;
        auto write(ImageIndex index) -> HostPass&;

        auto setCallback(const std::function<void(RenderGraph&)>& callback) -> HostPass&;


        template <std::ranges::range Range>
        auto upload(const BufferIndex dst, const Range& range) -> std::expected<BufferIndex, RenderGraphError> {
            return upload(dst, std::span<const u8>(reinterpret_cast<const u8*>(std::ranges::data(range)), std::ranges::size(range) * sizeof(std::ranges::range_value_t<Range>)));
        }
        auto upload(BufferIndex dst, std::span<const u8> data) -> std::expected<BufferIndex, RenderGraphError>;

        template <std::ranges::range Range>
        auto readback(const BufferIndex dst, Range& range) -> std::expected<BufferIndex, RenderGraphError> {
            return readback(dst, std::span<u8>(reinterpret_cast<u8*>(std::ranges::data(range)), std::ranges::size(range) * sizeof(std::ranges::range_value_t<Range>)));
        }
        auto readback(BufferIndex src, std::span<u8> data) -> std::expected<BufferIndex, RenderGraphError>;
    };

    class PresentPass : public PassBuilder {
    public:

        PresentPass(RenderGraph* graph, u32 index);

        auto acquire(Swapchain* swapchain) -> std::expected<ImageIndex, RenderGraphError>;

        auto present(Swapchain* swapchain, ImageIndex index) -> std::expected<ImageIndex, RenderGraphError>;

    };

    class RenderGraph : public ende::graph::Graph<RenderPass> {
    public:

        using Resource = std::variant<BufferInfo, ImageInfo>;

        struct CreateInfo {
            Device* device;
            std::shared_ptr<ende::thread::ThreadPool> threadPool = nullptr;
            bool multiQueue = false;
        };

        static auto create(const CreateInfo &info) -> std::expected<RenderGraph, RenderGraphError>;

        // resource management
        auto addGroup(std::string_view name, const std::array<f32, 4>& colour) -> RenderGroup;

        auto addBuffer(BufferInfo info) -> BufferIndex;

        auto addImage(ImageInfo info) -> ImageIndex;

        auto alias(BufferIndex index) -> BufferIndex;
        auto alias(ImageIndex index) -> ImageIndex;

        auto duplicate(BufferIndex index) -> std::expected<BufferIndex, RenderGraphError>;
        auto duplicate(ImageIndex index) -> std::expected<ImageIndex, RenderGraphError>;

        auto getBuffer(BufferIndex index) const -> std::expected<BufferHandle, RenderGraphError>;
        auto getImage(ImageIndex index) const -> std::expected<ImageHandle, RenderGraphError>;

        auto getBufferInfo(BufferIndex index) const -> std::expected<BufferInfo, RenderGraphError>;
        auto getImageInfo(ImageIndex index) const -> std::expected<ImageInfo, RenderGraphError>;

        auto getResource(u32 index) const -> std::expected<Resource, RenderGraphError>;

        auto getResourceName(u32 index) const -> std::expected<std::string_view, RenderGraphError>;

        auto updateBufferInfo(BufferIndex index, BufferInfo info) -> std::expected<BufferInfo, RenderGraphError>;
        auto updateImageInfo(ImageIndex index, ImageInfo info) -> std::expected<ImageInfo, RenderGraphError>;

        // pass management
        auto pass(std::string_view name, RenderPass::Type type, const PipelineHandle& pipeline = {}, const RenderGroup& group = {}) -> PassBuilder;

        auto compute(std::string_view name, const PipelineHandle &pipeline = {}, const RenderGroup& group = {}) -> ComputePass;

        auto graphics(std::string_view name, const PipelineHandle &pipeline = {}, const RenderGroup& group = {}) -> GraphicsPass;

        auto transfer(std::string_view name, const PipelineHandle &pipeline = {}, const RenderGroup& group = {}) -> TransferPass;

        auto host(std::string_view name) -> HostPass;

        auto acquire(Swapchain* swapchain) -> std::expected<ImageIndex, RenderGraphError>;

        auto present(Swapchain* swapchain, ImageIndex index) -> std::expected<ImageIndex, RenderGraphError>;


        // finalisation

        void setRoot(BufferIndex index);
        void setRoot(ImageIndex index);

        auto compile() -> std::expected<bool, RenderGraphError>;

        auto run(std::span<SemaphorePair> waits = {}, std::span<SemaphorePair> signals = {}, bool async = true) -> std::expected<bool, RenderGraphError>;

        void reset();


        auto device() const -> Device* { return _device; }

        auto timeline() -> SemaphoreHandle { return _cpuTimeline; }

        auto passes() -> std::span<RenderPass> { return _orderedPasses; }

        auto getPass(std::string_view name) const -> std::expected<RenderPass, RenderGraphError>;

        struct TimerInfo {
            std::string name = {};
            QueueType queue = QueueType::NONE;
            Timer timer = {};
        };
        auto timers() -> std::span<TimerInfo> {
            if (_timers[_device->flyingIndex()].size() < _timerCount) return {};
            return {_timers[_device->flyingIndex()].data(), _timerCount};
        }

        struct StatisticInfo {
            std::string name = {};
            QueueType queue = QueueType::NONE;
            PipelineStatistics statistics = {};
        };
        auto statistics() -> std::span<StatisticInfo> {
            if (_statistics[_device->flyingIndex()].size() < _statsCount) return {};
            return {_statistics[_device->flyingIndex()].data(), _statsCount};
        }

        enum class QueryMode {
            DISABLED,
            PER_PASS,
            PER_GROUP,
        };

        void setTimingMode(const QueryMode mode) { _timingMode = mode; }
        auto timingMode() const -> QueryMode { return _timingMode; }

        void setStatsMode(const QueryMode mode) { _statsMode = mode; }
        auto statsMode() const -> QueryMode { return _statsMode; }

        void setMultiQueue(const bool state) { _multiQueue = state; }
        auto multiQueue() const -> bool { return _multiQueue; }

        struct Access {
            i32 passIndex = -1;
            ResourceAccess access;
        };
        [[nodiscard]] static auto getNextAccess(std::span<const RenderPass> passes, i32 startIndex, i32 resource) -> Access;
        [[nodiscard]] static auto getCurrAccess(std::span<const RenderPass> passes, i32 startIndex, i32 resource) -> Access;
        [[nodiscard]] static auto getPrevAccess(std::span<const RenderPass> passes, i32 startIndex, i32 resource) -> Access;

    private:

        RenderGraph() = default;

        [[nodiscard]] auto getResourceIndices(std::span<const RenderPass> passes) const -> std::vector<std::pair<u32, u32>>;


        [[nodiscard]] auto buildDependencyLevels(std::span<const RenderPass> passes) const -> std::expected<std::vector<std::vector<u32>>, RenderGraphError>;

        void submitBarriers(CommandBuffer& commands, std::span<const RenderPass::Barrier> barriers) const;

        void startTimer(CommandBuffer* commands, u32 index, std::string_view name, QueueType queue);
        void endTimer(CommandBuffer* commands, u32 index);

        void startStats(CommandBuffer* commands, u32 index, std::string_view name, QueueType queue);
        void endStats(CommandBuffer* commands, u32 index);

        void buildBarriers();
        void buildResources();
        auto buildRenderAttachments() -> std::expected<bool, RenderGraphError> ;

        Device* _device = nullptr;
        std::shared_ptr<ende::thread::ThreadPool> _threadPool = nullptr;
        bool _multiQueue = false;

        std::vector<RenderPass> _orderedPasses = {};

        i32 _rootEdge = -1;
        i32 _rootPass = -1;

        // VmaPool _resourcePool = {};
        // u32 _poolSize = 0;
        std::vector<Resource> _resources = {};

        i32 _groupId = 0;

        u32 _timerCount = 0;
        bool _timerRunning = false;
        QueryMode _timingMode = QueryMode::DISABLED;
        std::array<std::vector<TimerInfo>, FRAMES_IN_FLIGHT> _timers = {};
        u32 _statsCount = 0;
        bool _statsRunning = false;
        QueryMode _statsMode = QueryMode::DISABLED;
        std::array<std::vector<StatisticInfo>, FRAMES_IN_FLIGHT> _statistics = {};

        // 0 = graphics, 1 = compute, 2 = transfer
        std::array<std::array<CommandPool, 3>, FRAMES_IN_FLIGHT> _commandPools = {};
        SemaphoreHandle _cpuTimeline = {};

    };


}


#endif //CANTA_RENDERGRAPHV2_H