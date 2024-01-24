#ifndef CANTA_DEVICE_H
#define CANTA_DEVICE_H

#include <Ende/platform.h>
#include <volk.h>
#include <expected>
#include <string>
#include <functional>
#include <span>
#include <memory>
#include <Canta/Enums.h>
#include <Canta/Swapchain.h>
#include <Canta/Semaphore.h>
#include <Canta/CommandPool.h>
#include <Canta/ResourceList.h>
#include <Canta/ShaderModule.h>
#include <Canta/Pipeline.h>
#include <Canta/Image.h>
#include <Canta/Buffer.h>
#include <Canta/Sampler.h>
#include <Canta/Timer.h>
#include <Canta/PipelineStatistics.h>
#include <Canta/Queue.h>
#include <Ende/time/StopWatch.h>

#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 0
#include <vk_mem_alloc.h>

#define VK_TRY(x) { \
    VkResult tryResult = x; \
    assert(tryResult == VK_SUCCESS); \
}

#define CANTA_BINDLESS_SAMPLERS 0
#define CANTA_BINDLESS_SAMPLED_IMAGES 1
#define CANTA_BINDLESS_STORAGE_IMAGES 2
#define CANTA_BINDLESS_STORAGE_BUFFERS 3

namespace canta {

    using ShaderHandle = Handle<ShaderModule, ResourceList<ShaderModule>>;
    using PipelineHandle = Handle<Pipeline, ResourceList<Pipeline>>;
    using ImageHandle = Handle<Image, ResourceList<Image>>;
    using BufferHandle = Handle<Buffer, ResourceList<Buffer>>;
    using SamplerHandle = Handle<Sampler, ResourceList<Sampler>>;

    struct Limits {
        u32 maxImageDimensions1D = 0;
        u32 maxImageDimensions2D = 0;
        u32 maxImageDimensions3D = 0;
        u32 maxImageDimensionsCube = 0;

        u32 maxDescriptorSetSamplers = 0;
        u32 maxDescriptorSetUniformBuffers = 0;
        u32 maxDescriptorSetStorageBuffers = 0;
        u32 maxDescriptorSetSampledImages = 0;
        u32 maxDescriptorSetStorageImages = 0;

        u32 maxBindlessSamplers = 0;
        u32 maxBindlessUniformBuffers = 0;
        u32 maxBindlessStorageBuffers = 0;
        u32 maxBindlessSampledImages = 0;
        u32 maxBindlessStorageImages = 0;

        f32 maxSamplerAnisotropy = 0;

        f32 timestampPeriod = 0;

        u32 maxTaskWorkGroupTotalCount = 0;
        u32 maxTaskWorkGroupCount[3] = {};
        u32 maxTaskWorkGroupInvocations = 0;
        u32 maxTaskWorkGroupSize[3] = {};
        u32 maxTaskPayloadSize = 0;
        u32 maxTaskSharedMemorySize = 0;
        u32 maxTaskPayloadAndSharedMemorySize = 0;
        u32 maxMeshWorkGroupTotalCount = 0;
        u32 maxMeshWorkGroupCount[3] = {};
        u32 maxMeshWorkGroupInvocations = 0;
        u32 maxMeshWorkGroupSize[3] = {};
        u32 maxMeshSharedMemorySize = 0;
        u32 maxMeshPayloadAndSharedMemorySize = 0;
        u32 maxMeshOutputMemorySize = 0;
        u32 maxMeshPayloadAndOutputMemorySize = 0;
        u32 maxMeshOutputComponents = 0;
        u32 maxMeshOutputVertices = 0;
        u32 maxMeshOutputPrimitives = 0;
        u32 maxMeshOutputLayers = 0;
        u32 maxMeshMultiviewViewCount = 0;
        u32 meshOutputPerVertexGranularity = 0;
        u32 meshOutputPerPrimitiveGranularity = 0;
        u32 maxPreferredTaskWorkGroupInvocations = 0;
        u32 maxPreferredMeshWorkGroupInvocations = 0;
        bool prefersLocalInvocationVertexOutput = false;
        bool prefersLocalInvocationPrimitiveOutput = false;
        bool prefersCompactVertexOutput = false;
        bool prefersCompactPrimitiveOutput = false;
    };

    struct Properties {
        u32 apiVersion;
        u32 driverVersion;
        u32 vendorID;
        std::string vendorName;
        u32 deviceID;
        PhysicalDeviceType deviceType;
        std::string deviceName;
        Limits limits;
    };

    constexpr const u32 FRAMES_IN_FLIGHT = 2;

    class Device {
    public:

        struct CreateInfo {
            std::string applicationName = {};
            u32 applicationVersion = 0;
            std::function<u32(const Properties&)> selector = {};
            bool enableMeshShading = true;
            bool enableTaskShading = false;
            bool enableAsyncComputeQueue = true;
            bool enableAsyncTransferQueue = true;
            std::span<const char* const> instanceExtensions = {};
            std::span<const char* const> deviceExtensions = {};
        };

        static auto create(CreateInfo info) noexcept -> std::expected<std::unique_ptr<Device>, Error>;

        ~Device();

        Device(Device&& rhs) noexcept;
        auto operator=(Device&& rhs) noexcept -> Device&;

        void gc();

        void beginFrame();
        auto endFrame() -> f64;

        template <typename Func>
        void immediate(Func func, QueueType queueType = QueueType::GRAPHICS) {
            auto& cmd = _immediatePool.getBuffer();
            cmd.begin();
            func(cmd);
            cmd.end();
            auto wait = std::to_array({
                _immediateTimeline.getPair()
            });
            _immediateTimeline.increment();
            auto signal = std::to_array({
                _immediateTimeline.getPair()
            });
            cmd.submit(wait, signal).and_then([&](auto result) {
                return _immediateTimeline.wait(_immediateTimeline.value());
            });
        }

        auto frameSemaphore() -> Semaphore* { return &_frameTimeline; }
        auto frameValue() const -> u64 { return _frameTimeline.value(); }
        auto framePrevValue() const -> u64 { return std::max(0l, static_cast<i64>(_frameTimeline.value()) - 1); }
        auto flyingIndex() const -> u32 { return _frameTimeline.value() % FRAMES_IN_FLIGHT; }

        auto instance() const -> VkInstance { return _instance; }
        auto physicalDevice() const -> VkPhysicalDevice { return _physicalDevice; }
        auto logicalDevice() const -> VkDevice { return _logicalDevice; }
        auto allocator() const -> VmaAllocator { return _allocator; }

        auto properties() const -> const Properties& { return _properties; }
        auto limits() const -> const Limits& { return _properties.limits; }
        // avoid - loops all supported extensions
        auto isExtensionEnabled(std::string_view extensionName) -> bool;
        auto meshShadersEnabled() const -> bool { return _meshShadersEnabled; }
        auto taskShadersEnabled() const -> bool { return _taskShadersEnabled; }

        auto bindlessSet() const -> VkDescriptorSet { return _bindlessSet; }

        auto queue(QueueType type) -> Queue&;

        auto waitIdle() const -> std::expected<bool, Error>;


        auto createSwapchain(Swapchain::CreateInfo info) -> std::expected<Swapchain, Error>;

        auto createSemaphore(Semaphore::CreateInfo info) -> std::expected<Semaphore, Error>;

        auto createCommandPool(CommandPool::CreateInfo info) -> std::expected<CommandPool, Error>;


        auto createShaderModule(ShaderModule::CreateInfo info, ShaderHandle oldHandle = {}) -> ShaderHandle;
        auto createPipeline(Pipeline::CreateInfo info, PipelineHandle oldHandle = {}) -> PipelineHandle;
        auto createImage(Image::CreateInfo info, ImageHandle oldHandle = {}) -> ImageHandle;
        auto createBuffer(Buffer::CreateInfo info, BufferHandle oldHandle = {}) -> BufferHandle;
        auto createSampler(Sampler::CreateInfo info, SamplerHandle oldHandle = {}) -> SamplerHandle;

        auto registerImage(Image::CreateInfo info, VkImage image, VkImageView view) -> ImageHandle;
        auto resizeBuffer(BufferHandle handle, u32 newSize) -> BufferHandle;

        void setDebugName(u32 type, u64 object, std::string_view name) const;

        template <typename T>
        void setDebugName(T& object, std::string_view name) const {};

        template <typename T>
        void setDebugName(VkImage& object, std::string_view name) const {
            setDebugName(VK_OBJECT_TYPE_IMAGE, (u64)object, name);
        }

        template <typename T>
        void setDebugName(VkImageView& object, std::string_view name) const {
            setDebugName(VK_OBJECT_TYPE_IMAGE_VIEW, (u64)object, name);
        }

        template <typename T>
        void setDebugName(VkBuffer& object, std::string_view name) const {
            setDebugName(VK_OBJECT_TYPE_BUFFER, (u64)object, name);
        }

        auto timestampPools() -> std::span<VkQueryPool> { return _timestampPools; }

        auto createTimer() -> Timer;
        void destroyTimer(u32 poolIndex, u32 queryIndex);

        auto pipelineStatisticsPools() -> std::span<VkQueryPool> { return _pipelineStatisticsPools; }

        auto createPipelineStatistics() -> PipelineStatistics;
        void destroyPipelineStatistics(u32 poolIndex, u32 queryIndex);

        struct ResourceStats {
            u32 shaderCount = 0;
            u32 shaderAllocated = 0;
            u32 pipelineCount = 0;
            u32 pipelineAllocated = 0;
            u32 imageCount = 0;
            u32 imageAllocated = 0;
            u32 bufferCount = 0;
            u32 bufferAllocated = 0;
            u32 samplerCount = 0;
            u32 samplerAllocated = 0;
        };
        auto resourceStats() const -> ResourceStats;

    private:
        friend CommandBuffer;

        Device() = default;

        void updateBindlessImage(u32 index, const Image::View& image, bool sampled, bool storage);
        void updateBindlessBuffer(u32 index, const Buffer& buffer);
        void updateBindlessSampler(u32 index, const Sampler& sampler);

        VkInstance _instance = VK_NULL_HANDLE;
        VkPhysicalDevice _physicalDevice = VK_NULL_HANDLE;
        VkDevice _logicalDevice = VK_NULL_HANDLE;

        VkDebugUtilsMessengerEXT _debugMessenger = VK_NULL_HANDLE;

        Properties _properties = {};
        std::vector<std::string> _enabledExtensions = {};

        bool _meshShadersEnabled = false;
        bool _taskShadersEnabled = false;

        Queue _graphicsQueue = {};
        Queue _computeQueue = {};
        Queue _transferQueue = {};

        VmaAllocator _allocator = VK_NULL_HANDLE;

        ende::time::StopWatch _frameClock = {};
        std::chrono::high_resolution_clock::duration _lastFrameDuration = {};

        CommandPool _immediatePool = {};

        std::vector<VkQueryPool> _timestampPools = {};
        u32 _lastPoolQueryCount = 0;
        struct FreeTimer {
            u32 poolIndex = 0;
            u32 queryIndex = 0;
        };
        std::vector<FreeTimer> _freeTimers = {};

        std::vector<VkQueryPool> _pipelineStatisticsPools = {};
        u32 _lastStatPoolQueryCount = 0;
        struct FreeStat {
            u32 poolIndex = 0;
            u32 queryIndex = 0;
        };
        std::vector<FreeStat> _freePipelineStats = {};

        std::array<BufferHandle, FRAMES_IN_FLIGHT> _markerBuffers = {};
        std::vector<std::string> _markerCommands[FRAMES_IN_FLIGHT] = {};
        u32 _markerOffset = 0;
        u32 _marker = 0;

        Semaphore _frameTimeline = {};
        Semaphore _immediateTimeline = {};

        VkDescriptorPool _bindlessPool = VK_NULL_HANDLE;
        VkDescriptorSetLayout _bindlessLayout = VK_NULL_HANDLE;
        VkDescriptorSet _bindlessSet = VK_NULL_HANDLE;

        ResourceList<ShaderModule> _shaderList;
        ResourceList<Pipeline> _pipelineList;
        ResourceList<Image> _imageList;
        ResourceList<Buffer> _bufferList;
        ResourceList<Sampler> _samplerList;

    };

}

#endif //CANTA_DEVICE_H
