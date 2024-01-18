#include "Canta/Device.h"
#include <spdlog/spdlog.h>

#define VMA_IMPLEMENTATION
#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 0
#include <vk_mem_alloc.h>

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

    properties.limits.maxImageDimensions1D = deviceProperties2.properties.limits.maxImageDimension1D;
    properties.limits.maxImageDimensions2D = deviceProperties2.properties.limits.maxImageDimension2D;
    properties.limits.maxImageDimensions3D = deviceProperties2.properties.limits.maxImageDimension3D;
    properties.limits.maxImageDimensionsCube = deviceProperties2.properties.limits.maxImageDimensionCube;

    properties.limits.maxDescriptorSetSamplers = deviceProperties2.properties.limits.maxDescriptorSetSamplers;
    properties.limits.maxDescriptorSetUniformBuffers = deviceProperties2.properties.limits.maxDescriptorSetUniformBuffers;
    properties.limits.maxDescriptorSetStorageBuffers = deviceProperties2.properties.limits.maxDescriptorSetStorageBuffers;
    properties.limits.maxDescriptorSetSampledImages = deviceProperties2.properties.limits.maxDescriptorSetSampledImages;
    properties.limits.maxDescriptorSetStorageImages = deviceProperties2.properties.limits.maxDescriptorSetStorageImages;

    u32 maxBindlessCount = 1 << 16;

    // Check against limits for case when driver doesnt report correct correct values (e.g. amdgpu-pro on linux)
    properties.limits.maxBindlessSamplers = indexingProperties.maxDescriptorSetUpdateAfterBindSamplers < std::numeric_limits<u32>::max() - 10000 ? std::min(indexingProperties.maxDescriptorSetUpdateAfterBindSamplers - 100, maxBindlessCount) : 10000;
    properties.limits.maxBindlessUniformBuffers = indexingProperties.maxDescriptorSetUpdateAfterBindUniformBuffers < std::numeric_limits<u32>::max() - 10000 ? std::min(indexingProperties.maxDescriptorSetUpdateAfterBindUniformBuffers - 100, maxBindlessCount) : 1000;
    properties.limits.maxBindlessStorageBuffers = indexingProperties.maxDescriptorSetUpdateAfterBindStorageBuffers < std::numeric_limits<u32>::max() - 10000 ? std::min(indexingProperties.maxDescriptorSetUpdateAfterBindStorageBuffers - 100, maxBindlessCount) : 1000;
    properties.limits.maxBindlessSampledImages = indexingProperties.maxDescriptorSetUpdateAfterBindSampledImages < std::numeric_limits<u32>::max() - 10000 ? std::min(indexingProperties.maxDescriptorSetUpdateAfterBindSampledImages - 100, maxBindlessCount) : 1000;
    properties.limits.maxBindlessStorageImages = indexingProperties.maxDescriptorSetUpdateAfterBindStorageImages < std::numeric_limits<u32>::max() - 10000 ? std::min(indexingProperties.maxDescriptorSetUpdateAfterBindStorageImages - 100, maxBindlessCount) : 1000;

    properties.limits.maxSamplerAnisotropy = deviceProperties2.properties.limits.maxSamplerAnisotropy;

    properties.limits.timestampPeriod = deviceProperties2.properties.limits.timestampPeriod;


    properties.limits.maxTaskWorkGroupTotalCount = meshShaderProperties.maxTaskWorkGroupTotalCount;
    properties.limits.maxTaskWorkGroupCount[0] = meshShaderProperties.maxTaskWorkGroupCount[0];
    properties.limits.maxTaskWorkGroupCount[1] = meshShaderProperties.maxTaskWorkGroupCount[1];
    properties.limits.maxTaskWorkGroupCount[2] = meshShaderProperties.maxTaskWorkGroupCount[2];
    properties.limits.maxTaskWorkGroupInvocations = meshShaderProperties.maxTaskWorkGroupInvocations;
    properties.limits.maxTaskWorkGroupSize[0] = meshShaderProperties.maxTaskWorkGroupSize[0];
    properties.limits.maxTaskWorkGroupSize[1] = meshShaderProperties.maxTaskWorkGroupSize[1];
    properties.limits.maxTaskWorkGroupSize[2] = meshShaderProperties.maxTaskWorkGroupSize[2];
    properties.limits.maxTaskPayloadSize = meshShaderProperties.maxTaskPayloadSize;
    properties.limits.maxTaskSharedMemorySize = meshShaderProperties.maxTaskSharedMemorySize;
    properties.limits.maxTaskPayloadAndSharedMemorySize = meshShaderProperties.maxTaskPayloadAndSharedMemorySize;
    properties.limits.maxMeshWorkGroupTotalCount = meshShaderProperties.maxMeshWorkGroupTotalCount;
    properties.limits.maxMeshWorkGroupCount[0] = meshShaderProperties.maxMeshWorkGroupCount[0];
    properties.limits.maxMeshWorkGroupCount[1] = meshShaderProperties.maxMeshWorkGroupCount[1];
    properties.limits.maxMeshWorkGroupCount[2] = meshShaderProperties.maxMeshWorkGroupCount[2];
    properties.limits.maxMeshWorkGroupInvocations = meshShaderProperties.maxMeshWorkGroupInvocations;
    properties.limits.maxMeshWorkGroupSize[0] = meshShaderProperties.maxMeshWorkGroupSize[0];
    properties.limits.maxMeshWorkGroupSize[1] = meshShaderProperties.maxMeshWorkGroupSize[1];
    properties.limits.maxMeshWorkGroupSize[2] = meshShaderProperties.maxMeshWorkGroupSize[2];
    properties.limits.maxMeshSharedMemorySize = meshShaderProperties.maxMeshSharedMemorySize;
    properties.limits.maxMeshPayloadAndSharedMemorySize = meshShaderProperties.maxMeshPayloadAndSharedMemorySize;
    properties.limits.maxMeshOutputMemorySize = meshShaderProperties.maxMeshOutputMemorySize;
    properties.limits.maxMeshPayloadAndOutputMemorySize = meshShaderProperties.maxMeshPayloadAndOutputMemorySize;
    properties.limits.maxMeshOutputComponents = meshShaderProperties.maxMeshOutputComponents;
    properties.limits.maxMeshOutputVertices = meshShaderProperties.maxMeshOutputVertices;
    properties.limits.maxMeshOutputPrimitives = meshShaderProperties.maxMeshOutputPrimitives;
    properties.limits.maxMeshOutputLayers = meshShaderProperties.maxMeshOutputLayers;
    properties.limits.maxMeshMultiviewViewCount = meshShaderProperties.maxMeshMultiviewViewCount;
    properties.limits.meshOutputPerVertexGranularity = meshShaderProperties.meshOutputPerVertexGranularity;
    properties.limits.meshOutputPerPrimitiveGranularity = meshShaderProperties.meshOutputPerPrimitiveGranularity;
    properties.limits.maxPreferredTaskWorkGroupInvocations = meshShaderProperties.maxPreferredTaskWorkGroupInvocations;
    properties.limits.maxPreferredMeshWorkGroupInvocations = meshShaderProperties.maxPreferredMeshWorkGroupInvocations;
    properties.limits.prefersLocalInvocationVertexOutput = meshShaderProperties.prefersLocalInvocationVertexOutput;
    properties.limits.prefersLocalInvocationPrimitiveOutput = meshShaderProperties.prefersLocalInvocationPrimitiveOutput;
    properties.limits.prefersCompactVertexOutput = meshShaderProperties.prefersCompactVertexOutput;
    properties.limits.prefersCompactPrimitiveOutput = meshShaderProperties.prefersCompactPrimitiveOutput;

    return properties;
}



auto canta::Device::create(canta::Device::CreateInfo info) noexcept -> std::expected<std::unique_ptr<Device>, Error> {

    std::unique_ptr<Device> device(new Device());

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

    VK_TRY(vkCreateInstance(&instanceCreateInfo, nullptr, &device->_instance));

    volkLoadInstanceOnly(device->_instance);

#ifndef NDEBUG
    // init debug messenger
    VkDebugUtilsMessengerCreateInfoEXT debugInfo = {};
    debugInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    debugInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT;
    debugInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
    debugInfo.pfnUserCallback = debugCallback;
    debugInfo.pUserData = nullptr;

    auto createDebugUtils = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(device->_instance, "vkCreateDebugUtilsMessengerEXT");
    VK_TRY(createDebugUtils(device->_instance, &debugInfo, nullptr, &device->_debugMessenger));
#endif

    // get physical device
    u32 physicalDeviceCount = 0;
    VK_TRY(vkEnumeratePhysicalDevices(device->_instance, &physicalDeviceCount, nullptr));
    std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);
    VK_TRY(vkEnumeratePhysicalDevices(device->_instance, &physicalDeviceCount, physicalDevices.data()));

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
            device->_properties = properties;
        }
    }
    device->_physicalDevice = physicalDevices[maxIndex];
    if (device->_physicalDevice == VK_NULL_HANDLE)
        return std::unexpected(Error::INVALID_GPU);


    // queue creation infos
    u32 queueCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device->_physicalDevice, &queueCount, nullptr);
    std::vector<VkQueueFamilyProperties> familyProperties(queueCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device->_physicalDevice, &queueCount, familyProperties.data());

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


    u32 supportedDeviceExtensionCount = 0;
    vkEnumerateDeviceExtensionProperties(device->physicalDevice(), nullptr, &supportedDeviceExtensionCount, nullptr);
    std::vector<VkExtensionProperties> supportedExtensions(supportedDeviceExtensionCount);
    vkEnumerateDeviceExtensionProperties(device->physicalDevice(), nullptr, &supportedDeviceExtensionCount, supportedExtensions.data());

    const auto isExtensionSupported = [&](std::string_view extensionName) {
        for (auto& extension : supportedExtensions) {
            if (strcmp(extension.extensionName, extensionName.data()) == 0)
                return true;
        }
        return false;
    };

#define REQUIRE_EXTENSION(extension, extensionList)             \
    if (isExtensionSupported(extension))                        \
        (extensionList).push_back(extension);                   \
    else                                                        \
        return std::unexpected(Error::EXTENSION_NOT_PRESENT);

#define ENABLE_EXTENSION(extension, extensionList) \
    if (isExtensionSupported(extension))           \
        (extensionList).push_back(extension);

    // init logical device
    std::vector<const char*> deviceExtensions;
    for (auto& extension : info.deviceExtensions) {
        REQUIRE_EXTENSION(extension, deviceExtensions);
    }
    REQUIRE_EXTENSION(VK_KHR_SWAPCHAIN_EXTENSION_NAME, deviceExtensions);
    if (info.enableMeshShading) {
        REQUIRE_EXTENSION(VK_EXT_MESH_SHADER_EXTENSION_NAME, deviceExtensions);
    }
#ifndef NDEBUG
    ENABLE_EXTENSION(VK_AMD_BUFFER_MARKER_EXTENSION_NAME, deviceExtensions);
#endif

    VkPhysicalDeviceFeatures2 deviceFeatures2 = {};
    deviceFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    //TODO: choose manually dont enable all
    vkGetPhysicalDeviceFeatures2(device->_physicalDevice, &deviceFeatures2);

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
    vulkan13Features.dynamicRendering = true;

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

    auto result = vkCreateDevice(device->_physicalDevice, &deviceCreateInfo, nullptr, &device->_logicalDevice);
    if (result != VK_SUCCESS)
        return std::unexpected(static_cast<Error>(result));

    device->_enabledExtensions.insert(device->_enabledExtensions.begin(), deviceExtensions.begin(), deviceExtensions.end());
    device->_enabledExtensions.insert(device->_enabledExtensions.begin(), instanceExtensions.begin(), instanceExtensions.end());

    volkLoadDevice(device->_logicalDevice);

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
    device->_graphicsIndex = findQueueIndex(QueueType::GRAPHICS);
    device->_computeIndex = findQueueIndex(QueueType::COMPUTE, QueueType::GRAPHICS);
    device->_transferIndex = findQueueIndex(QueueType::TRANSFER, QueueType::GRAPHICS);

    if (device->_graphicsIndex >= 0)
        vkGetDeviceQueue(device->_logicalDevice, device->_graphicsIndex, 0, &device->_graphicsQueue);
    if (device->_computeIndex >= 0)
        vkGetDeviceQueue(device->_logicalDevice, device->_computeIndex, 0, &device->_computeQueue);
    if (device->_transferIndex >= 0)
        vkGetDeviceQueue(device->_logicalDevice, device->_transferIndex, 0, &device->_transferQueue);


    // init VMA
    VmaAllocatorCreateInfo allocatorCreateInfo{};
    allocatorCreateInfo.vulkanApiVersion = applicationInfo.apiVersion;
    allocatorCreateInfo.physicalDevice = device->_physicalDevice;
    allocatorCreateInfo.device = device->_logicalDevice;
    allocatorCreateInfo.instance = device->_instance;

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

    VK_TRY(vmaCreateAllocator(&allocatorCreateInfo, &device->_allocator));


    device->_frameTimeline = device->createSemaphore({
        .initialValue = 0,
        .name = "frameTimelineSemaphore"
    }).value();


    VkDescriptorPoolSize poolSizes[] = {
            { VK_DESCRIPTOR_TYPE_SAMPLER, device->limits().maxBindlessSamplers },
            { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, device->limits().maxBindlessSampledImages },
            { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, device->limits().maxBindlessStorageImages },
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, device->limits().maxBindlessStorageBuffers },
    };

    VkDescriptorPoolCreateInfo bindlessPoolCreateInfo = {};
    bindlessPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    bindlessPoolCreateInfo.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;
    bindlessPoolCreateInfo.maxSets = 1;
    bindlessPoolCreateInfo.poolSizeCount = 4;
    bindlessPoolCreateInfo.pPoolSizes = poolSizes;
    VK_TRY(vkCreateDescriptorPool(device->logicalDevice(), &bindlessPoolCreateInfo, nullptr, &device->_bindlessPool));
    device->setDebugName(VK_OBJECT_TYPE_DESCRIPTOR_POOL, (u64)device->_bindlessPool, "bindless_pool");

    VkDescriptorSetLayoutBinding bindlessLayoutBindings[4] = {};

    bindlessLayoutBindings[0].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
    bindlessLayoutBindings[0].descriptorCount = device->limits().maxBindlessSamplers;
    bindlessLayoutBindings[0].binding = CANTA_BINDLESS_SAMPLERS;
    bindlessLayoutBindings[0].stageFlags = VK_SHADER_STAGE_ALL;
    bindlessLayoutBindings[0].pImmutableSamplers = nullptr;

    bindlessLayoutBindings[1].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    bindlessLayoutBindings[1].descriptorCount = device->limits().maxBindlessSampledImages;
    bindlessLayoutBindings[1].binding = CANTA_BINDLESS_SAMPLED_IMAGES;
    bindlessLayoutBindings[1].stageFlags = VK_SHADER_STAGE_ALL;
    bindlessLayoutBindings[1].pImmutableSamplers = nullptr;

    bindlessLayoutBindings[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    bindlessLayoutBindings[2].descriptorCount = device->limits().maxBindlessStorageImages;
    bindlessLayoutBindings[2].binding = CANTA_BINDLESS_STORAGE_IMAGES;
    bindlessLayoutBindings[2].stageFlags = VK_SHADER_STAGE_ALL;
    bindlessLayoutBindings[2].pImmutableSamplers = nullptr;

    bindlessLayoutBindings[3].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    bindlessLayoutBindings[3].descriptorCount = device->limits().maxBindlessStorageBuffers;
    bindlessLayoutBindings[3].binding = CANTA_BINDLESS_STORAGE_BUFFERS;
    bindlessLayoutBindings[3].stageFlags = VK_SHADER_STAGE_ALL;
    bindlessLayoutBindings[3].pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutCreateInfo bindlessLayoutCreateInfo = {};
    bindlessLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    bindlessLayoutCreateInfo.bindingCount = 4;
    bindlessLayoutCreateInfo.pBindings = bindlessLayoutBindings;
    bindlessLayoutCreateInfo.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;

    VkDescriptorBindingFlags bindingFlags[4] = {
            VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT,
            VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT,
            VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT,
            VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT,
    };

    VkDescriptorSetLayoutBindingFlagsCreateInfo bindlessExtendedInfo = {};
    bindlessExtendedInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
    bindlessExtendedInfo.bindingCount = 4;
    bindlessExtendedInfo.pBindingFlags = bindingFlags;
    bindlessLayoutCreateInfo.pNext = &bindlessExtendedInfo;

    VK_TRY(vkCreateDescriptorSetLayout(device->logicalDevice(), &bindlessLayoutCreateInfo, nullptr, &device->_bindlessLayout));

    VkDescriptorSetAllocateInfo bindlessAllocInfo = {};
    bindlessAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    bindlessAllocInfo.descriptorSetCount = 1;
    bindlessAllocInfo.descriptorPool = device->_bindlessPool;
    bindlessAllocInfo.pSetLayouts = &device->_bindlessLayout;

    VK_TRY(vkAllocateDescriptorSets(device->logicalDevice(), &bindlessAllocInfo, &device->_bindlessSet));
    device->setDebugName(VK_OBJECT_TYPE_DESCRIPTOR_SET, (u64)device->_bindlessSet, "bindless_set");

#ifndef NDEBUG
    if (device->isExtensionEnabled(VK_AMD_BUFFER_MARKER_EXTENSION_NAME)) {
        for (u32 i = 0; auto& buffer : device->_markerBuffers) {
            buffer = device->createBuffer({
                .size = sizeof(u32) * 1000,
                .usage = BufferUsage::TRANSFER_DST,
                .type = MemoryType::READBACK,
                .requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
                .persistentlyMapped = true,
                .name = std::format("marker_buffer_{}", i++)
            });
        }
    }
#endif

    return device;
}

canta::Device::~Device() {
    _frameTimeline.signal(std::numeric_limits<u64>::max());
    vkDeviceWaitIdle(_logicalDevice);

    for (auto& buffer : _markerBuffers)
        buffer = {};

    _shaderList.clearAll([&](auto& module) {
        vkDestroyShaderModule(logicalDevice(), module.module(), nullptr);
    });
    _pipelineList.clearAll();
    _imageList.clearAll();
    _bufferList.clearAll();
    _samplerList.clearAll();

#ifndef NDEBUG
    vkDestroyDebugUtilsMessengerEXT(_instance, _debugMessenger, nullptr);
#endif

    _frameTimeline = {};

    for (auto& queryPool : _timestampPools)
        vkDestroyQueryPool(_logicalDevice, queryPool, nullptr);

    vmaDestroyAllocator(_allocator);

    vkDestroyDescriptorSetLayout(_logicalDevice, _bindlessLayout, nullptr);
    vkDestroyDescriptorPool(_logicalDevice, _bindlessPool, nullptr);

    vkDestroyDevice(_logicalDevice, nullptr);
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
    std::swap(_frameTimeline, rhs._frameTimeline);
}

auto canta::Device::operator=(canta::Device &&rhs) noexcept -> Device & {
    std::swap(_instance, rhs._instance);
    std::swap(_physicalDevice, rhs._physicalDevice);
    std::swap(_logicalDevice, rhs._logicalDevice);
    std::swap(_debugMessenger, rhs._debugMessenger);
    std::swap(_properties, rhs._properties);
    std::swap(_graphicsQueue, rhs._graphicsQueue);
    std::swap(_computeQueue, rhs._computeQueue);
    std::swap(_transferQueue, rhs._transferQueue);
    std::swap(_allocator, rhs._allocator);
    std::swap(_frameTimeline, rhs._frameTimeline);
    return *this;
}

void canta::Device::gc() {
    _shaderList.clearQueue([&](auto& module) {
        vkDestroyShaderModule(logicalDevice(), module.module(), nullptr);
        module._module = VK_NULL_HANDLE;
    });
    _pipelineList.clearQueue();
    _imageList.clearQueue();
    _bufferList.clearQueue();
    _samplerList.clearQueue();
}

void canta::Device::beginFrame() {
    u64 frameValue = std::max(0l, static_cast<i64>(_frameTimeline.value()) - (FRAMES_IN_FLIGHT - 1));
    _frameTimeline.wait(frameValue);
    _frameTimeline.increment();

#ifndef NDEBUG
    _markerOffset = 0;
    _marker = 0;
    _markerCommands[flyingIndex()].clear();
    std::memset(_markerBuffers[flyingIndex()]->_mapped.address(), 0, _markerBuffers[flyingIndex()]->size());
#endif
}

void canta::Device::endFrame() {

}

auto canta::Device::isExtensionEnabled(std::string_view extensionName) -> bool {
    for (auto& extension : _enabledExtensions) {
        if (strcmp(extension.c_str(), extensionName.data()) == 0)
            return true;
    }
    return false;
}

auto canta::Device::queue(canta::QueueType type) const -> VkQueue {
    switch (type) {
        case QueueType::GRAPHICS:
            return _graphicsQueue;
        case QueueType::COMPUTE:
            return _computeQueue;
        case QueueType::TRANSFER:
            return _transferQueue;
    }
    return VK_NULL_HANDLE;
}

auto canta::Device::waitIdle() const -> std::expected<bool, Error> {
    auto result = vkDeviceWaitIdle(logicalDevice());
    if (result != VK_SUCCESS)
        return std::unexpected(static_cast<Error>(result));
    return true;
}

auto canta::Device::createSwapchain(Swapchain::CreateInfo info) -> std::expected<Swapchain, Error> {
    Swapchain swapchain = {};
    swapchain._device = this;

    if (!info.window)
        return std::unexpected(Error::INVALID_PLATFORM);

    auto platformExtent = info.window->extent();
    swapchain._extent = { platformExtent.x(), platformExtent.y() };
    swapchain._surface = info.window->surface(*this);
    swapchain._selector = info.selector;

    swapchain.createSwapchain();
    swapchain.createSemaphores();

    return swapchain;
}

auto canta::Device::createSemaphore(Semaphore::CreateInfo info) -> std::expected<Semaphore, Error> {
    Semaphore semaphore = {};
    semaphore._device = this;

    VkSemaphoreCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkSemaphoreTypeCreateInfo typeCreateInfo = {};
    typeCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO;
    if (info.initialValue >= 0) {
        typeCreateInfo.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;
        typeCreateInfo.initialValue = info.initialValue;
        createInfo.pNext = &typeCreateInfo;
        semaphore._value = info.initialValue;
        semaphore._isTimeline = true;
    }

    auto result = vkCreateSemaphore(logicalDevice(), &createInfo, nullptr, &semaphore._semaphore);
    if (result != VK_SUCCESS)
        return std::unexpected(static_cast<Error>(result));

    if (!info.name.empty())
        setDebugName(VK_OBJECT_TYPE_SEMAPHORE, (u64)semaphore._semaphore, info.name);
    return semaphore;
}

auto canta::Device::createCommandPool(CommandPool::CreateInfo info) -> std::expected<CommandPool, Error> {
    CommandPool pool = {};
    pool._device = this;

    VkCommandPoolCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    switch (info.queueType) {
        case QueueType::GRAPHICS:
            createInfo.queueFamilyIndex = _graphicsIndex;
            break;
        case QueueType::COMPUTE:
            createInfo.queueFamilyIndex = _computeIndex;
            break;
        case QueueType::TRANSFER:
            createInfo.queueFamilyIndex = _transferIndex;
            break;
    }
    auto result = vkCreateCommandPool(logicalDevice(), &createInfo, nullptr, &pool._pool);
    if (result != VK_SUCCESS)
        return std::unexpected(static_cast<Error>(result));

    pool._queueType = info.queueType;

    if (!info.name.empty())
        setDebugName(VK_OBJECT_TYPE_COMMAND_POOL, (u64)pool._pool, info.name);
    return pool;
}

auto canta::Device::createShaderModule(ShaderModule::CreateInfo info, ShaderHandle oldHandle) -> ShaderHandle {
    VkShaderModule module = {};
    VkShaderModuleCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = info.spirv.size() * sizeof(u32);
    createInfo.pCode = info.spirv.data();

    auto result = vkCreateShaderModule(logicalDevice(), &createInfo, nullptr, &module);
    if (result != VK_SUCCESS)
        return {};

    ShaderHandle handle = {};
    if (oldHandle)
        handle = _shaderList.reallocate(oldHandle);
    else
        handle = _shaderList.allocate();

    handle->_device = this;
    handle->_module = module;
    handle->_stage = info.stage;
    auto ar = std::to_array({ ShaderInterface::CreateInfo{ info.spirv, info.stage } });
    handle->_interface = ShaderInterface::create(ar);
    return handle;
}

auto canta::Device::createPipeline(Pipeline::CreateInfo info, PipelineHandle oldHandle) -> PipelineHandle {
    ShaderInterface interface = {};
    std::vector<VkPipelineShaderStageCreateInfo> shaderStages = {};
    for (auto& shaderInfo : info.shaders) {
        auto array = std::to_array({ interface, shaderInfo.module->interface() });
        interface = ShaderInterface::merge(array);

        VkPipelineShaderStageCreateInfo stageCreateInfo = {};
        stageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        stageCreateInfo.stage = static_cast<VkShaderStageFlagBits>(shaderInfo.module->stage());
        stageCreateInfo.module = shaderInfo.module->module();
        stageCreateInfo.pName = shaderInfo.entryPoint.data();
        shaderStages.push_back(stageCreateInfo);
    }

    std::vector<VkDescriptorSetLayout> setLayouts = {};
    setLayouts.push_back(_bindlessLayout);

    for (u32 i = 1; i < interface.setCount(); i++) {
        std::vector<VkDescriptorSetLayoutBinding> layoutBindings = {};
        u32 layoutBindingCount = 0;
        auto& set = interface.getSet(i);
        layoutBindings.resize(set.bindingCount);

        for (u32 j = 0; j < set.bindingCount; j++) {
            auto& binding = set.bindings[j];

//            if (binding.type == ShaderInterface::BindingType::NONE)
//                break;

            VkDescriptorSetLayoutBinding layoutBinding = {};

            layoutBindings[layoutBindingCount].binding = j;
            VkDescriptorType type;
            switch (binding.type) {
                case ShaderInterface::BindingType::UNIFORM_BUFFER:
                    type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                    break;
                case ShaderInterface::BindingType::SAMPLED_IMAGE:
                    type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    break;
                case ShaderInterface::BindingType::STORAGE_IMAGE:
                    type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
                    break;
                case ShaderInterface::BindingType::STORAGE_BUFFER:
                    type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                    break;
                default:
                    type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            }
            layoutBinding.descriptorType = type;
            layoutBinding.descriptorCount = 1;
            layoutBinding.stageFlags = static_cast<VkShaderStageFlagBits>(binding.stages);
            layoutBinding.pImmutableSamplers = nullptr;
            layoutBindings.push_back(layoutBinding);
        }

        VkDescriptorSetLayoutCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        createInfo.bindingCount = layoutBindings.size();
        createInfo.pBindings = layoutBindings.data();

        VkDescriptorSetLayout setLayout;
        VK_TRY(vkCreateDescriptorSetLayout(logicalDevice(), &createInfo, nullptr, &setLayout));
        setLayouts.push_back(setLayout);
    }

    std::vector<VkPushConstantRange> pushConstantRanges = {};
    for (u32 i = 0; i < interface.pushRangeCount(); i++) {
        VkPushConstantRange range = {};
        range.stageFlags |= static_cast<VkShaderStageFlagBits>(interface.getPushRange(i).stage);
        range.size = interface.getPushRange(i).size;
        range.offset = interface.getPushRange(i).offset;
        pushConstantRanges.push_back(range);
    }

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = setLayouts.size();
    pipelineLayoutInfo.pSetLayouts = setLayouts.data();

    pipelineLayoutInfo.pushConstantRangeCount = pushConstantRanges.size();
    pipelineLayoutInfo.pPushConstantRanges = pushConstantRanges.data();

    VkPipelineLayout  pipelineLayout;
    VK_TRY(vkCreatePipelineLayout(logicalDevice(), &pipelineLayoutInfo, nullptr, &pipelineLayout));


    VkPipeline pipeline;
    if (info.mode == PipelineMode::GRAPHICS) {
        VkGraphicsPipelineCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;

        createInfo.stageCount = shaderStages.size();
        createInfo.pStages = shaderStages.data();

        std::vector<VkVertexInputBindingDescription> inputBindings = {};
        for (auto& binding : info.inputBindings) {
            inputBindings.push_back({
                .binding = binding.binding,
                .stride = binding.stride,
                .inputRate = static_cast<VkVertexInputRate>(binding.inputRate)
            });

        }
        std::vector<VkVertexInputAttributeDescription> inputAttributes = {};
        for (auto& attribute : info.inputAttributes) {
            inputAttributes.push_back({
                .location = attribute.location,
                .binding = attribute.binding,
                .format = static_cast<VkFormat>(attribute.format),
                .offset = attribute.offset
            });
        }
        VkPipelineVertexInputStateCreateInfo vertexInputState = {};
        vertexInputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputState.vertexBindingDescriptionCount = inputBindings.size();
        vertexInputState.pVertexBindingDescriptions = inputBindings.data();
        vertexInputState.vertexAttributeDescriptionCount = inputAttributes.size();
        vertexInputState.pVertexAttributeDescriptions = inputAttributes.data();

        VkPipelineInputAssemblyStateCreateInfo inputAssemblyState = {};
        inputAssemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssemblyState.topology = static_cast<VkPrimitiveTopology>(info.topology);
        inputAssemblyState.primitiveRestartEnable = info.primitiveRestart;

        VkPipelineRenderingCreateInfo renderingCreateInfo = {};
        renderingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;

        renderingCreateInfo.colorAttachmentCount = info.colourFormats.size();
        static_assert(sizeof(VkFormat) == sizeof(Format));
        renderingCreateInfo.pColorAttachmentFormats = reinterpret_cast<VkFormat*>(info.colourFormats.data());
        if (info.depthFormat != Format::UNDEFINED)
            renderingCreateInfo.depthAttachmentFormat = static_cast<VkFormat>(info.depthFormat);

        VkPipelineViewportStateCreateInfo viewportState = {};
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount = 1;
        viewportState.scissorCount = 1;

        VkPipelineRasterizationStateCreateInfo rasterisationState = {};
        rasterisationState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterisationState.depthClampEnable = info.rasterState.depthClamp;
        rasterisationState.rasterizerDiscardEnable = info.rasterState.rasterDiscard;
        rasterisationState.polygonMode = static_cast<VkPolygonMode>(info.rasterState.polygonMode);
        rasterisationState.lineWidth = info.rasterState.lineWidth;
        rasterisationState.cullMode = static_cast<VkCullModeFlagBits>(info.rasterState.cullMode);
        rasterisationState.frontFace = static_cast<VkFrontFace>(info.rasterState.frontFace);
        rasterisationState.depthBiasEnable = info.rasterState.depthBias;
        rasterisationState.depthBiasConstantFactor = 0.f;
        rasterisationState.depthBiasClamp = 0.f;
        rasterisationState.depthBiasSlopeFactor = 0.f;

        VkPipelineMultisampleStateCreateInfo multisampleState = {};
        multisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampleState.sampleShadingEnable = false;
        multisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        multisampleState.minSampleShading = 1.f;
        multisampleState.pSampleMask = nullptr;
        multisampleState.alphaToCoverageEnable = false;
        multisampleState.alphaToOneEnable = false;

        VkPipelineDepthStencilStateCreateInfo depthStencilState = {};
        depthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depthStencilState.depthTestEnable = info.depthState.test;
        depthStencilState.depthWriteEnable = info.depthState.write;
        depthStencilState.depthCompareOp = static_cast<VkCompareOp>(info.depthState.compareOp);
        depthStencilState.depthBoundsTestEnable = true;
        depthStencilState.minDepthBounds = 0.f;
        depthStencilState.maxDepthBounds = 1.f;
        depthStencilState.stencilTestEnable = false;
        depthStencilState.front = {};
        depthStencilState.back = {};

        VkPipelineColorBlendAttachmentState colorBlendAttachment{};
        colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachment.blendEnable = info.blendState.blend;
        colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
        colorBlendAttachment.srcColorBlendFactor = static_cast<VkBlendFactor>(info.blendState.srcFactor);
        colorBlendAttachment.dstColorBlendFactor = static_cast<VkBlendFactor>(info.blendState.dstFactor);
        colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
        colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
        colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_DST_ALPHA;

        std::vector<VkPipelineColorBlendAttachmentState> colourBlendAttachments = {};
        for (u32 i = 0; i < info.colourFormats.size(); i++)
            colourBlendAttachments.push_back(colorBlendAttachment);

        VkPipelineColorBlendStateCreateInfo colourBlendState{};
        colourBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colourBlendState.logicOpEnable = VK_FALSE;
        colourBlendState.logicOp = VK_LOGIC_OP_COPY;
        colourBlendState.attachmentCount = info.colourFormats.size();
        colourBlendState.pAttachments = colourBlendAttachments.data();
        colourBlendState.blendConstants[0] = 0.f;
        colourBlendState.blendConstants[1] = 0.f;
        colourBlendState.blendConstants[2] = 0.f;
        colourBlendState.blendConstants[3] = 0.f;

        VkDynamicState dynamicStates[2] = {
                VK_DYNAMIC_STATE_VIEWPORT,
                VK_DYNAMIC_STATE_SCISSOR,
        };
        VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo = {};
        dynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicStateCreateInfo.dynamicStateCount = 2;
        dynamicStateCreateInfo.pDynamicStates = dynamicStates;

        createInfo.pVertexInputState = &vertexInputState;
        createInfo.pInputAssemblyState = &inputAssemblyState;
        createInfo.pNext = &renderingCreateInfo;
        createInfo.renderPass = VK_NULL_HANDLE;
        createInfo.pRasterizationState = &rasterisationState;
        createInfo.pMultisampleState = &multisampleState;
        createInfo.pDepthStencilState = &depthStencilState;
        createInfo.pColorBlendState = &colourBlendState;
        createInfo.pViewportState = &viewportState;
        createInfo.pDynamicState = &dynamicStateCreateInfo;
        createInfo.layout = pipelineLayout;
        createInfo.basePipelineHandle = VK_NULL_HANDLE;
        createInfo.basePipelineIndex = -1;

        auto result = vkCreateGraphicsPipelines(logicalDevice(), VK_NULL_HANDLE, 1, &createInfo, nullptr, &pipeline);
        if (result != VK_SUCCESS)
            return {};

    } else {
        VkComputePipelineCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;

        createInfo.stage = shaderStages.front();
        createInfo.layout = pipelineLayout;
        createInfo.basePipelineHandle = VK_NULL_HANDLE;
        createInfo.basePipelineIndex = -1;

        auto result = vkCreateComputePipelines(logicalDevice(), VK_NULL_HANDLE, 1, &createInfo, nullptr, &pipeline);
        if (result != VK_SUCCESS)
            return {};
    }

    PipelineHandle handle = {};
    if (oldHandle)
        handle = _pipelineList.reallocate(oldHandle);
    else
        handle = _pipelineList.allocate();

    handle->_device = this;
    handle->_pipeline = pipeline;
    handle->_layout = pipelineLayout;
    handle->_mode = info.mode;
    handle->_interface = interface;

    return handle;
}

auto canta::Device::createImage(Image::CreateInfo info, ImageHandle oldHandle) -> ImageHandle {
    VkImage image;
    VmaAllocation allocation;
    VkImageCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;

    auto type = info.type;
    if (info.type == ImageType::AUTO) {
        if (info.depth > 1) {
            createInfo.imageType = VK_IMAGE_TYPE_3D;
            type = ImageType::IMAGE3D;
        } else if (info.height > 1) {
            createInfo.imageType = VK_IMAGE_TYPE_2D;
            type = ImageType::IMAGE2D;
        } else {
            createInfo.imageType = VK_IMAGE_TYPE_1D;
            type = ImageType::IMAGE1D;
        }
    } else {
        createInfo.imageType = static_cast<VkImageType>(info.type);
    }

    createInfo.format = static_cast<VkFormat>(info.format);
    createInfo.extent.width = info.width;
    createInfo.extent.height = info.height;
    createInfo.extent.depth = info.depth;
    createInfo.mipLevels = info.mipLevels;
    createInfo.arrayLayers = info.layers;

    createInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    createInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    createInfo.usage = static_cast<VkImageUsageFlagBits>(info.usage);
    createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    createInfo.samples = VK_SAMPLE_COUNT_1_BIT;

    if (info.layers == 6)
        createInfo.flags |= VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;

    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
    allocInfo.flags = 0;
    VK_TRY(vmaCreateImage(_allocator, &createInfo, &allocInfo, &image, &allocation, nullptr))

    setDebugName(image, info.name);

    ImageHandle handle = {};
    if (oldHandle)
        handle = _imageList.reallocate(oldHandle);
    else
        handle = _imageList.allocate();

    handle->_device = this;
    handle->_image = image;
    handle->_allocation = allocation;
    handle->_width = info.width;
    handle->_height = info.height;
    handle->_depth = info.depth;
    handle->_layers = info.layers;
    handle->_mips = info.mipLevels;
    handle->_type = type;
    handle->_format = info.format;
    handle->_usage = info.usage;
    handle->_layout = ImageLayout::UNDEFINED;
    handle->_defaultView = handle->createView({});
    handle->_name = info.name;

    bool isSampled = (info.usage & ImageUsage::SAMPLED) == ImageUsage::SAMPLED;
    bool isStorage = (info.usage & ImageUsage::STORAGE) == ImageUsage::STORAGE;

    updateBindlessImage(handle.index(), handle->defaultView(), isSampled, isStorage);

    return handle;
}

auto canta::Device::registerImage(Image::CreateInfo info, VkImage image, VkImageView view) -> ImageHandle {

    auto type = info.type;
    if (info.type == ImageType::AUTO) {
        if (info.depth > 1) {
            type = ImageType::IMAGE3D;
        } else if (info.height > 1) {
            type = ImageType::IMAGE2D;
        } else {
            type = ImageType::IMAGE1D;
        }
    }

    Image::View defaultView = {};
    // set device as null so object destructor doesnt destroy view
    defaultView._device = nullptr;
    defaultView._image = nullptr;
    defaultView._view = view;

    auto handle = _imageList.allocate();

    handle->_device = this;
    handle->_image = image;
    handle->_allocation = VK_NULL_HANDLE;
    handle->_width = info.width;
    handle->_height = info.height;
    handle->_depth = info.depth;
    handle->_layers = info.layers;
    handle->_mips = info.mipLevels;
    handle->_type = type;
    handle->_format = info.format;
    handle->_usage = info.usage;
    handle->_layout = ImageLayout::UNDEFINED;
    handle->_defaultView = std::move(defaultView);
    handle->_name = info.name;

    bool isSampled = (info.usage & ImageUsage::SAMPLED) == ImageUsage::SAMPLED;
    bool isStorage = (info.usage & ImageUsage::STORAGE) == ImageUsage::STORAGE;

    updateBindlessImage(handle.index(), handle->defaultView(), isSampled, isStorage);

    return handle;
}

auto canta::Device::createBuffer(Buffer::CreateInfo info, BufferHandle oldHandle) -> BufferHandle {
    info.usage |= BufferUsage::TRANSFER_DST | BufferUsage::TRANSFER_SRC | BufferUsage::STORAGE | BufferUsage::DEVICE_ADDRESS;

    VkBuffer buffer;
    VmaAllocation allocation;
    VkBufferCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;

    createInfo.size = info.size;
    createInfo.usage = static_cast<VkBufferUsageFlagBits>(info.usage);
    createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
    allocInfo.requiredFlags = info.requiredFlags;
    allocInfo.preferredFlags = info.preferredFlags;

    switch (info.type) {
        case MemoryType::DEVICE:
            info.persistentlyMapped = false;
            break;
        case MemoryType::STAGING:
            allocInfo.flags |= VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
            break;
        case MemoryType::READBACK:
            allocInfo.flags |= VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT;
            break;
    }

    VK_TRY(vmaCreateBuffer(_allocator, &createInfo, &allocInfo, &buffer, &allocation, nullptr));

    setDebugName(buffer, info.name);

    VkBufferDeviceAddressInfo deviceAddressInfo = {};
    deviceAddressInfo.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
    deviceAddressInfo.buffer = buffer;
    auto address = vkGetBufferDeviceAddress(logicalDevice(), &deviceAddressInfo);

    BufferHandle handle = {};
    if (oldHandle)
        handle = _bufferList.reallocate(oldHandle);
    else
        handle = _bufferList.allocate();

    handle->_device = this;
    handle->_buffer = buffer;
    handle->_allocation = allocation;
    handle->_deviceAddress = address;
    handle->_size = info.size;
    handle->_usage = info.usage;
    handle->_type = info.type;
    handle->_requiredFlags = info.requiredFlags;
    handle->_preferredFlags = info.preferredFlags;
    handle->_name = info.name;

    if (info.persistentlyMapped)
        handle->_mapped = handle->map();

    updateBindlessBuffer(handle.index(), *handle);

    return handle;
}

auto canta::Device::createSampler(Sampler::CreateInfo info, SamplerHandle oldHandle) -> SamplerHandle {
    VkSampler sampler;

    VkSamplerCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    createInfo.magFilter = static_cast<VkFilter>(info.filter);
    createInfo.minFilter = static_cast<VkFilter>(info.filter);
    createInfo.mipmapMode = static_cast<VkSamplerMipmapMode>(info.mipmapMode);
    createInfo.addressModeU = static_cast<VkSamplerAddressMode>(info.addressMode);
    createInfo.addressModeV = createInfo.addressModeU;
    createInfo.addressModeW = createInfo.addressModeV;
    createInfo.mipLodBias = info.mipLodBias;
    createInfo.anisotropyEnable = info.anisotropy;
    createInfo.maxAnisotropy = info.maxAnisotropy == 0 ? limits().maxSamplerAnisotropy : info.maxAnisotropy;
    createInfo.compareEnable = info.compare;
    createInfo.compareOp = static_cast<VkCompareOp>(info.compareOp);
    createInfo.minLod = info.minLod;
    createInfo.maxLod = info.maxLod;
    createInfo.borderColor = static_cast<VkBorderColor>(info.borderColour);
    createInfo.unnormalizedCoordinates = info.unnormalisedCoordinates;

    VK_TRY(vkCreateSampler(logicalDevice(), &createInfo, nullptr, &sampler));

    SamplerHandle handle = {};
    if (oldHandle)
        handle = _samplerList.reallocate(oldHandle);
    else
        handle = _samplerList.allocate();

    handle->_device = this;
    handle->_sampler = sampler;

    updateBindlessSampler(handle.index(), *handle);

    return handle;
}

auto canta::Device::resizeBuffer(canta::BufferHandle handle, u32 newSize) -> BufferHandle {
    return createBuffer({
        .size = newSize,
        .usage = handle->usage(),
        .type = handle->type(),
        .requiredFlags = handle->_requiredFlags,
        .preferredFlags = handle->_preferredFlags,
        .persistentlyMapped = handle->persitentlyMapped(),
        .name = handle->name()
    }, handle);
}

void canta::Device::setDebugName(u32 type, u64 object, std::string_view name) const {
#ifndef NDEBUG
    VkDebugUtilsObjectNameInfoEXT objectNameInfo = {};
    objectNameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
    objectNameInfo.objectType = (VkObjectType)type;
    objectNameInfo.pObjectName = name.begin();
    objectNameInfo.objectHandle = object;
    vkSetDebugUtilsObjectNameEXT(logicalDevice(), &objectNameInfo);
#endif
}

void canta::Device::updateBindlessImage(u32 index, const Image::View &image, bool sampled, bool storage) {
    VkWriteDescriptorSet descriptorWrite[2] = {};
    i32 writeNum = 0;

    VkDescriptorImageInfo sampledImageInfo = {};
    if (sampled) {
        sampledImageInfo.imageView = image.view();
        sampledImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        descriptorWrite[writeNum].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite[writeNum].descriptorCount = 1;
        descriptorWrite[writeNum].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
        descriptorWrite[writeNum].dstArrayElement = index;
        descriptorWrite[writeNum].dstSet = _bindlessSet;
        descriptorWrite[writeNum].dstBinding = CANTA_BINDLESS_SAMPLED_IMAGES;
        descriptorWrite[writeNum].pImageInfo = &sampledImageInfo;
        writeNum++;
    }
    VkDescriptorImageInfo storageImageInfo = {};
    if (storage) {
        storageImageInfo.imageView = image.view();
        storageImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

        descriptorWrite[writeNum].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite[writeNum].descriptorCount = 1;
        descriptorWrite[writeNum].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        descriptorWrite[writeNum].dstArrayElement = index;
        descriptorWrite[writeNum].dstSet = _bindlessSet;
        descriptorWrite[writeNum].dstBinding = CANTA_BINDLESS_STORAGE_IMAGES;
        descriptorWrite[writeNum].pImageInfo = &storageImageInfo;
        writeNum++;
    }

    vkUpdateDescriptorSets(logicalDevice(), writeNum, descriptorWrite, 0, nullptr);
}

void canta::Device::updateBindlessBuffer(u32 index, const canta::Buffer &buffer) {
    VkWriteDescriptorSet descriptorWrite = {};

    VkDescriptorBufferInfo bufferInfo = {};
    bufferInfo.buffer = buffer.buffer();
    bufferInfo.offset = 0;
    bufferInfo.range = buffer.size();

    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    descriptorWrite.dstArrayElement = index;
    descriptorWrite.dstSet = _bindlessSet;
    descriptorWrite.dstBinding = CANTA_BINDLESS_STORAGE_BUFFERS;
    descriptorWrite.pBufferInfo = &bufferInfo;

    vkUpdateDescriptorSets(logicalDevice(), 1, &descriptorWrite, 0, nullptr);
}

void canta::Device::updateBindlessSampler(u32 index, const canta::Sampler &sampler) {
    VkWriteDescriptorSet descriptorWrite = {};

    VkDescriptorImageInfo samplerInfo = {};
    samplerInfo.sampler = sampler.sampler();

    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
    descriptorWrite.dstArrayElement = index;
    descriptorWrite.dstSet = _bindlessSet;
    descriptorWrite.dstBinding = CANTA_BINDLESS_SAMPLERS;
    descriptorWrite.pImageInfo = &samplerInfo;

    vkUpdateDescriptorSets(logicalDevice(), 1, &descriptorWrite, 0, nullptr);
}

auto canta::Device::createTimer() -> Timer {
    constexpr const u32 poolQueryCount = 10;

    u32 poolIndex = 0;
    u32 queryIndex = 0;
    if (!_freeTimers.empty()) {
        auto freeSlot = _freeTimers.back();
        _freeTimers.pop_back();
        poolIndex = freeSlot.poolIndex;
        queryIndex = freeSlot.queryIndex;
    } else if (_lastPoolQueryCount < poolQueryCount && !_timestampPools.empty()) {
        poolIndex = _timestampPools.size() - 1;
        queryIndex = _lastPoolQueryCount++;
    } else {
        _lastPoolQueryCount = 0;
        VkQueryPoolCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
        createInfo.queryType = VK_QUERY_TYPE_TIMESTAMP;
        createInfo.queryCount = poolQueryCount * 2;
        VkQueryPool pool;
        VK_TRY(vkCreateQueryPool(logicalDevice(), &createInfo, nullptr, &pool));
        vkResetQueryPool(logicalDevice(), pool, 0, createInfo.queryCount);
        poolIndex = _timestampPools.size();
        queryIndex = _lastPoolQueryCount++;
        _timestampPools.push_back(pool);
    }

    Timer timer = {};

    timer._device = this;
    timer._queryPoolIndex = poolIndex;
    timer._index = queryIndex;
    timer._value = 0;

    return timer;
}

void canta::Device::destroyTimer(u32 poolIndex, u32 queryIndex) {
    _freeTimers.push_back({
        .poolIndex = poolIndex,
        .queryIndex = queryIndex
    });
}