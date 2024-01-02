#include "Canta/Device.h"
#include <spdlog/spdlog.h>

#define VMA_IMPLEMENTATION
#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 0
#include <vk_mem_alloc.h>

#define VK_TRY(x) { \
    VkResult tryResult = x; \
    assert(tryResult == VK_SUCCESS); \
}

template <typename T, typename U>
void appendFeatureChain(T* start, U* next) {
    auto* oldNext = start->pNext;
    start->pNext = next;
    next->pNext = oldNext;
}

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData
) {
    if (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
        return VK_FALSE;
    switch (messageSeverity) {
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
            spdlog::info("{}", pCallbackData->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
            spdlog::info("{}", pCallbackData->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
            spdlog::warn("{}", pCallbackData->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
            spdlog::error("{}", pCallbackData->pMessage);
            break;
        default:
            break;
    }
    return VK_FALSE;
}

canta::Properties getPhysicalDeviceProperties(VkPhysicalDevice deviceProperties) {
    canta::Properties properties = {};

    VkPhysicalDeviceProperties2 deviceProperties2 = {};
    deviceProperties2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;

    VkPhysicalDeviceDescriptorIndexingProperties indexingProperties = {};
    indexingProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_PROPERTIES;

    VkPhysicalDeviceMeshShaderPropertiesEXT meshShaderProperties = {};
    meshShaderProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_PROPERTIES_EXT;

    deviceProperties2.pNext = &indexingProperties;
    indexingProperties.pNext = &meshShaderProperties;

    vkGetPhysicalDeviceProperties2(deviceProperties, &deviceProperties2);

    properties.apiVersion = deviceProperties2.properties.apiVersion;
    properties.driverVersion = deviceProperties2.properties.driverVersion;
    properties.vendorID = deviceProperties2.properties.vendorID;
    switch (deviceProperties2.properties.vendorID) {
        case 0x1002:
            properties.vendorName = "AMD";
            break;
        case 0x1010:
            properties.vendorName = "ImgTec";
            break;
        case 0x10DE:
            properties.vendorName = "NVIDIA";
            break;
        case 0x13B5:
            properties.vendorName = "ARM";
            break;
        case 0x5143:
            properties.vendorName = "Qualcomm";
            break;
        case 0x8086:
            properties.vendorName = "INTEL";
            break;
    }
    properties.deviceID = deviceProperties2.properties.deviceID;
    properties.deviceType = static_cast<canta::PhysicalDeviceType>(deviceProperties2.properties.deviceType);
    properties.deviceName = deviceProperties2.properties.deviceName;

    return properties;
}



auto canta::Device::create(canta::Device::CreateInfo info) noexcept -> std::expected<Device, Error> {

    Device device = {};

    // init instance
    VK_TRY(volkInitialize());

    VkApplicationInfo applicationInfo = {};
    applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    applicationInfo.pApplicationName = info.applicationName.c_str();
    applicationInfo.applicationVersion = info.applicationVersion;
    applicationInfo.pEngineName = "Canta";
    applicationInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    applicationInfo.apiVersion = VK_API_VERSION_1_3;

    std::vector<const char*> instanceExtensions;
    instanceExtensions.insert(instanceExtensions.end(), info.instanceExtensions.begin(), info.instanceExtensions.end());
#ifndef NDEBUG
    instanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif
    instanceExtensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);

    VkInstanceCreateInfo instanceCreateInfo = {};
    instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceCreateInfo.pApplicationInfo = &applicationInfo;
    instanceCreateInfo.enabledLayerCount = 0;
    instanceCreateInfo.enabledExtensionCount = instanceExtensions.size();
    instanceCreateInfo.ppEnabledExtensionNames = instanceExtensions.data();

    VK_TRY(vkCreateInstance(&instanceCreateInfo, nullptr, &device._instance));

    volkLoadInstanceOnly(device._instance);

#ifndef NDEBUG
    // init debug messenger
    VkDebugUtilsMessengerCreateInfoEXT debugInfo = {};
    debugInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    debugInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT;
    debugInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
    debugInfo.pfnUserCallback = debugCallback;
    debugInfo.pUserData = nullptr;

    auto createDebugUtils = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(device._instance, "vkCreateDebugUtilsMessengerEXT");
    VK_TRY(createDebugUtils(device._instance, &debugInfo, nullptr, &device._debugMessenger));
#endif

    // get physical device
    u32 physicalDeviceCount = 0;
    VK_TRY(vkEnumeratePhysicalDevices(device._instance, &physicalDeviceCount, nullptr));
    std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);
    VK_TRY(vkEnumeratePhysicalDevices(device._instance, &physicalDeviceCount, physicalDevices.data()));

    const auto defaultDeviceSelector = [](const Properties& properties) -> u32 {
        switch (properties.deviceType) {
            case PhysicalDeviceType::DISCRETE:
                return 1000;
            case PhysicalDeviceType::INTEGRATED:
                return 100;
        }
        return 0;
    };

    u32 maxIndex = 0;
    u32 maxScore = 0;
    for (u32 i = 0; i < physicalDeviceCount; i++) {
        auto properties = getPhysicalDeviceProperties(physicalDevices[i]);
        u32 score = info.selector ? info.selector(properties) : defaultDeviceSelector(properties);
        if (score > maxScore) {
            maxScore = score;
            maxIndex = i;
            device._properties = properties;
        }
    }
    device._physicalDevice = physicalDevices[maxIndex];
    if (device._physicalDevice == VK_NULL_HANDLE)
        return std::unexpected(Error::INVALID_GPU);


    // queue creation infos
    u32 queueCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device._physicalDevice, &queueCount, nullptr);
    std::vector<VkQueueFamilyProperties> familyProperties(queueCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device._physicalDevice, &queueCount, familyProperties.data());

    f32 queuePriority = 1.0;

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos = {};
    for (u32 i = 0; i < familyProperties.size(); i++) {
        VkDeviceQueueCreateInfo queueCreateInfo = {};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = i;
        queueCreateInfo.queueCount = 1; //TODO: ensure enough queues for both async compute and transfer
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }



    // init logical device
    std::vector<const char*> deviceExtensions;
    deviceExtensions.insert(deviceExtensions.end(), info.deviceExtensions.begin(), info.deviceExtensions.end());
    deviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    deviceExtensions.push_back(VK_EXT_MESH_SHADER_EXTENSION_NAME);


    VkPhysicalDeviceFeatures2 deviceFeatures2 = {};
    deviceFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    //TODO: choose manually dont enable all
    vkGetPhysicalDeviceFeatures2(device._physicalDevice, &deviceFeatures2);

    VkPhysicalDeviceVulkan11Features vulkan11Features = {};
    vulkan11Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES;
    appendFeatureChain(&deviceFeatures2, &vulkan11Features);
    vulkan11Features.shaderDrawParameters = true;
    vulkan11Features.multiview = true;
    vulkan11Features.storageBuffer16BitAccess = true;

    VkPhysicalDeviceVulkan12Features vulkan12Features = {};
    vulkan12Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
    appendFeatureChain(&deviceFeatures2, &vulkan12Features);
    vulkan12Features.drawIndirectCount = true;
    vulkan12Features.hostQueryReset = true;
    vulkan12Features.timelineSemaphore = true;
    vulkan12Features.bufferDeviceAddress = true;
    vulkan12Features.scalarBlockLayout = true;
    vulkan12Features.storageBuffer8BitAccess = true;
    vulkan12Features.vulkanMemoryModel = true;
    vulkan12Features.vulkanMemoryModelDeviceScope = true;
    vulkan12Features.runtimeDescriptorArray = true;
    vulkan12Features.descriptorIndexing = true;
    vulkan12Features.descriptorBindingPartiallyBound = true;
    vulkan12Features.descriptorBindingSampledImageUpdateAfterBind = true;
    vulkan12Features.descriptorBindingStorageImageUpdateAfterBind = true;
    vulkan12Features.descriptorBindingStorageBufferUpdateAfterBind = true;
    vulkan12Features.shaderSampledImageArrayNonUniformIndexing = true;

    VkPhysicalDeviceVulkan13Features vulkan13Features = {};
    vulkan13Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
    appendFeatureChain(&deviceFeatures2, &vulkan13Features);
    vulkan13Features.maintenance4 = true;
    vulkan13Features.synchronization2 = true;

    VkPhysicalDeviceMeshShaderFeaturesEXT meshShaderFeatures = {};
    meshShaderFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_EXT;
    if (info.enableMeshShading)
        appendFeatureChain(&deviceFeatures2, &meshShaderFeatures);
    meshShaderFeatures.meshShader = true;
    meshShaderFeatures.taskShader = true;
    meshShaderFeatures.multiviewMeshShader = true;


    VkDeviceCreateInfo deviceCreateInfo = {};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

    deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
    deviceCreateInfo.queueCreateInfoCount = queueCreateInfos.size();

    deviceCreateInfo.pNext = &deviceFeatures2;

    deviceCreateInfo.enabledExtensionCount = deviceExtensions.size();
    deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();

    auto result = vkCreateDevice(device._physicalDevice, &deviceCreateInfo, nullptr, &device._logicalDevice);
    if (result != VK_SUCCESS)
        return std::unexpected(static_cast<Error>(result));

    volkLoadDevice(device._logicalDevice);

    // get queues
    const auto findQueueIndex = [&](QueueType type, QueueType reject = QueueType::NONE) -> i32 {
        u32 flags = static_cast<u32>(type);
        u32 rejectFlags = static_cast<u32>(reject);
        i32 i = 0;
        for (auto& queueFamily : familyProperties) {
            if ((queueFamily.queueFlags & rejectFlags) != 0) {
                i++;
                continue;
            }
            if (queueFamily.queueCount > 0) {
                if ((queueFamily.queueFlags & flags) == flags) {
                    return i;
                }
            }
        }
        return -1;
    };
    auto graphicsQueueIndex = findQueueIndex(QueueType::GRAPHICS);
    auto computeQueueIndex = findQueueIndex(QueueType::COMPUTE, QueueType::GRAPHICS);
    auto transferQueueIndex = findQueueIndex(QueueType::TRANSFER, QueueType::GRAPHICS);

    if (graphicsQueueIndex >= 0)
        vkGetDeviceQueue(device._logicalDevice, graphicsQueueIndex, 0, &device._graphicsQueue);
    if (computeQueueIndex >= 0)
        vkGetDeviceQueue(device._logicalDevice, computeQueueIndex, 0, &device._computeQueue);
    if (transferQueueIndex >= 0)
        vkGetDeviceQueue(device._logicalDevice, transferQueueIndex, 0, &device._transferQueue);


    // init VMA
    VmaAllocatorCreateInfo allocatorCreateInfo{};
    allocatorCreateInfo.vulkanApiVersion = applicationInfo.apiVersion;
    allocatorCreateInfo.physicalDevice = device._physicalDevice;
    allocatorCreateInfo.device = device._logicalDevice;
    allocatorCreateInfo.instance = device._instance;

    VmaVulkanFunctions vulkanFunctions{};
    vulkanFunctions.vkGetInstanceProcAddr               = vkGetInstanceProcAddr;
    vulkanFunctions.vkGetDeviceProcAddr                 = vkGetDeviceProcAddr;
    vulkanFunctions.vkAllocateMemory                    = vkAllocateMemory;
    vulkanFunctions.vkBindBufferMemory                  = vkBindBufferMemory;
    vulkanFunctions.vkBindBufferMemory2KHR              = vkBindBufferMemory2;
    vulkanFunctions.vkBindImageMemory                   = vkBindImageMemory;
    vulkanFunctions.vkBindImageMemory2KHR               = vkBindImageMemory2;
    vulkanFunctions.vkCreateBuffer                      = vkCreateBuffer;
    vulkanFunctions.vkCreateImage                       = vkCreateImage;
    vulkanFunctions.vkDestroyBuffer                     = vkDestroyBuffer;
    vulkanFunctions.vkDestroyImage                      = vkDestroyImage;
    vulkanFunctions.vkFlushMappedMemoryRanges           = vkFlushMappedMemoryRanges;
    vulkanFunctions.vkFreeMemory                        = vkFreeMemory;
    vulkanFunctions.vkGetBufferMemoryRequirements       = vkGetBufferMemoryRequirements;
    vulkanFunctions.vkGetBufferMemoryRequirements2KHR   = vkGetBufferMemoryRequirements2;
    vulkanFunctions.vkGetImageMemoryRequirements        = vkGetImageMemoryRequirements;
    vulkanFunctions.vkGetImageMemoryRequirements2KHR    = vkGetImageMemoryRequirements2;
    vulkanFunctions.vkGetPhysicalDeviceMemoryProperties = vkGetPhysicalDeviceMemoryProperties;
    vulkanFunctions.vkGetPhysicalDeviceMemoryProperties2KHR = vkGetPhysicalDeviceMemoryProperties2;
    vulkanFunctions.vkGetPhysicalDeviceProperties       = vkGetPhysicalDeviceProperties;
    vulkanFunctions.vkInvalidateMappedMemoryRanges      = vkInvalidateMappedMemoryRanges;
    vulkanFunctions.vkMapMemory                         = vkMapMemory;
    vulkanFunctions.vkUnmapMemory                       = vkUnmapMemory;
    vulkanFunctions.vkCmdCopyBuffer                     = vkCmdCopyBuffer;
    vulkanFunctions.vkGetDeviceBufferMemoryRequirements = vkGetDeviceBufferMemoryRequirements;
    vulkanFunctions.vkGetDeviceImageMemoryRequirements  = vkGetDeviceImageMemoryRequirements;

    allocatorCreateInfo.pVulkanFunctions = &vulkanFunctions;
    if (vulkan12Features.bufferDeviceAddress)
        allocatorCreateInfo.flags |= VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;

    VK_TRY(vmaCreateAllocator(&allocatorCreateInfo, &device._allocator));

    return device;
}

canta::Device::~Device() {
    vkDestroyInstance(_instance, nullptr);
}

canta::Device::Device(canta::Device &&rhs) noexcept {
    std::swap(_instance, rhs._instance);
    std::swap(_physicalDevice, rhs._physicalDevice);
    std::swap(_logicalDevice, rhs._logicalDevice);
    std::swap(_debugMessenger, rhs._debugMessenger);
    std::swap(_properties, rhs._properties);
    std::swap(_graphicsQueue, rhs._graphicsQueue);
    std::swap(_computeQueue, rhs._computeQueue);
    std::swap(_transferQueue, rhs._transferQueue);
    std::swap(_allocator, rhs._allocator);
}

auto canta::Device::operator==(canta::Device &&rhs) noexcept -> Device & {
    std::swap(_instance, rhs._instance);
    std::swap(_physicalDevice, rhs._physicalDevice);
    std::swap(_logicalDevice, rhs._logicalDevice);
    std::swap(_debugMessenger, rhs._debugMessenger);
    std::swap(_properties, rhs._properties);
    std::swap(_graphicsQueue, rhs._graphicsQueue);
    std::swap(_computeQueue, rhs._computeQueue);
    std::swap(_transferQueue, rhs._transferQueue);
    std::swap(_allocator, rhs._allocator);
    return *this;
}