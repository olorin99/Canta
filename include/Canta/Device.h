#ifndef CANTA_DEVICE_H
#define CANTA_DEVICE_H

#include <Ende/platform.h>
#include <volk.h>
#include <expected>
#include <string>
#include <functional>
#include <span>
#include <Canta/Enums.h>

#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 0
#include <vk_mem_alloc.h>

namespace canta {

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

        static auto create(CreateInfo info) noexcept -> std::expected<Device, Error>;

        ~Device();

        Device(Device&& rhs) noexcept;
        auto operator==(Device&& rhs) noexcept -> Device&;

    private:

        Device() = default;

        VkInstance _instance = VK_NULL_HANDLE;
        VkPhysicalDevice _physicalDevice = VK_NULL_HANDLE;
        VkDevice _logicalDevice = VK_NULL_HANDLE;

        VkDebugUtilsMessengerEXT _debugMessenger = VK_NULL_HANDLE;

        Properties _properties = {};

        VkQueue _graphicsQueue = VK_NULL_HANDLE;
        VkQueue _computeQueue = VK_NULL_HANDLE;
        VkQueue _transferQueue = VK_NULL_HANDLE;

        VmaAllocator _allocator = VK_NULL_HANDLE;

    };

}

#endif //CANTA_DEVICE_H
