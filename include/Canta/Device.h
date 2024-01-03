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

#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 0
#include <vk_mem_alloc.h>

#define VK_TRY(x) { \
    VkResult tryResult = x; \
    assert(tryResult == VK_SUCCESS); \
}

namespace canta {

    using ShaderHandle = Handle<ShaderModule, ResourceList<ShaderModule>>;

    struct Properties {
        u32 apiVersion;
        u32 driverVersion;
        u32 vendorID;
        std::string vendorName;
        u32 deviceID;
        PhysicalDeviceType deviceType;
        std::string deviceName;
        int limits;
    };

    constexpr const u32 FRAMES_IN_FLIGHT = 2;

    class Device {
    public:

        struct CreateInfo {
            std::string applicationName = {};
            u32 applicationVersion = 0;
            std::function<u32(const Properties&)> selector = {};
            bool enableMeshShading = true;
            bool enableMeshShadingFallback = true;

            std::span<const char* const> instanceExtensions = {};
            std::span<const char* const> deviceExtensions = {};
        };

        static auto create(CreateInfo info) noexcept -> std::expected<std::unique_ptr<Device>, Error>;

        ~Device();

        Device(Device&& rhs) noexcept;
        auto operator=(Device&& rhs) noexcept -> Device&;

        auto instance() const -> VkInstance { return _instance; }
        auto physicalDevice() const -> VkPhysicalDevice { return _physicalDevice; }
        auto logicalDevice() const -> VkDevice { return _logicalDevice; }

        auto queue(QueueType type) const -> VkQueue;


        auto createSwapchain(Swapchain::CreateInfo info) -> std::expected<Swapchain, Error>;

        auto createSemaphore(Semaphore::CreateInfo info) -> std::expected<Semaphore, Error>;

        auto createCommandPool(CommandPool::CreateInfo info) -> std::expected<CommandPool, Error>;


        auto createShaderModule(ShaderModule::CreateInfo info) -> ShaderHandle;


        void setDebugName(u32 type, u64 object, std::string_view name) const;

    private:

        Device() = default;

        VkInstance _instance = VK_NULL_HANDLE;
        VkPhysicalDevice _physicalDevice = VK_NULL_HANDLE;
        VkDevice _logicalDevice = VK_NULL_HANDLE;

        VkDebugUtilsMessengerEXT _debugMessenger = VK_NULL_HANDLE;

        Properties _properties = {};

        VkQueue _graphicsQueue = VK_NULL_HANDLE;
        u32 _graphicsIndex = 0;
        VkQueue _computeQueue = VK_NULL_HANDLE;
        u32 _computeIndex = 0;
        VkQueue _transferQueue = VK_NULL_HANDLE;
        u32 _transferIndex = 0;

        VmaAllocator _allocator = VK_NULL_HANDLE;


        ResourceList<ShaderModule> _shaderList;

    };

}

#endif //CANTA_DEVICE_H
