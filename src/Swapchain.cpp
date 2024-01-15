#include "Canta/Swapchain.h"
#include <Canta/Device.h>
#include <numeric>
#include <format>

VkSurfaceCapabilitiesKHR getCapabilities(VkPhysicalDevice device, VkSurfaceKHR surface) {
    VkSurfaceCapabilitiesKHR capabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &capabilities);
    return capabilities;
}

VkSurfaceFormatKHR getSurfaceFormat(VkPhysicalDevice device, VkSurfaceKHR surface) {
    u32 count = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &count, nullptr);
    std::vector<VkSurfaceFormatKHR> formats(count);
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &count, formats.data());

    if (formats.size() == 1 && formats[0].format == VK_FORMAT_UNDEFINED)
        return {VK_FORMAT_B8G8R8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};
    for (auto& format : formats) {
        if (format.format == VK_FORMAT_B8G8R8_UNORM && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            return format;
    }
    return formats.front();
}

VkPresentModeKHR getValidPresentMode(VkPhysicalDevice device, VkSurfaceKHR surface, VkPresentModeKHR preference = VK_PRESENT_MODE_MAILBOX_KHR) {
    u32 count = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &count, nullptr);
    std::vector<VkPresentModeKHR> modes(count);
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &count, modes.data());

    for (auto& mode : modes) {
        if (mode == preference)
            return mode;
    }
    for (auto& mode : modes) {
        if (mode == VK_PRESENT_MODE_MAILBOX_KHR)
            return mode;
    }
    for (auto& mode : modes) {
        if (mode == VK_PRESENT_MODE_IMMEDIATE_KHR)
            return mode;
    }
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D getExtent(const VkSurfaceCapabilitiesKHR& capabilities, u32 width, u32 height) {
    if (capabilities.currentExtent.width != std::numeric_limits<u32>::max())
        return capabilities.currentExtent;

    VkExtent2D extent = {width, height};
    extent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, extent.width));
    extent.height = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.height, extent.height));
    return extent;
}


canta::Swapchain::~Swapchain() {
    if (!_device)
        return;

    for (auto& view : _imageViews)
        vkDestroyImageView(_device->logicalDevice(), view, nullptr);

    vkDestroySwapchainKHR(_device->logicalDevice(), _swapchain, nullptr);
    vkDestroySurfaceKHR(_device->instance(), _surface, nullptr);
}

canta::Swapchain::Swapchain(canta::Swapchain &&rhs) noexcept {
    std::swap(_device, rhs._device);
    std::swap(_surface, rhs._surface);
    std::swap(_swapchain, rhs._swapchain);
    std::swap(_presentMode, rhs._presentMode);
    std::swap(_extent, rhs._extent);
    std::swap(_format, rhs._format);
    std::swap(_index, rhs._index);
    std::swap(_imageHandles, rhs._imageHandles);
    std::swap(_images, rhs._images);
    std::swap(_imageViews, rhs._imageViews);
    std::swap(_semaphores, rhs._semaphores);
    std::swap(_semaphoreIndex, rhs._semaphoreIndex);
    std::swap(_selector, rhs._selector);
}

auto canta::Swapchain::operator=(canta::Swapchain &&rhs) noexcept -> Swapchain & {
    std::swap(_device, rhs._device);
    std::swap(_surface, rhs._surface);
    std::swap(_swapchain, rhs._swapchain);
    std::swap(_presentMode, rhs._presentMode);
    std::swap(_extent, rhs._extent);
    std::swap(_format, rhs._format);
    std::swap(_index, rhs._index);
    std::swap(_imageHandles, rhs._imageHandles);
    std::swap(_images, rhs._images);
    std::swap(_imageViews, rhs._imageViews);
    std::swap(_semaphores, rhs._semaphores);
    std::swap(_semaphoreIndex, rhs._semaphoreIndex);
    std::swap(_selector, rhs._selector);
    return *this;
}

auto canta::Swapchain::acquire() -> std::expected<ImageHandle, Error> {
    _semaphoreIndex = (_semaphoreIndex % _semaphores.size());
    auto result = vkAcquireNextImageKHR(_device->logicalDevice(), _swapchain, std::numeric_limits<u64>::max(), _semaphores[_semaphoreIndex].acquire.semaphore(), VK_NULL_HANDLE, &_index);
    switch (result) {
        case VK_SUCCESS:
            break;
        case VK_ERROR_OUT_OF_DATE_KHR:
            recreate();
            break;
        case VK_SUBOPTIMAL_KHR:
//            recreate();
            break;
        default:
            return std::unexpected(static_cast<Error>(result));
    }
    return _imageHandles[_index];
}

auto canta::Swapchain::present() -> std::expected<u32, Error> {
    auto presentSemaphore = _semaphores[_semaphoreIndex].present.semaphore();
    _semaphoreIndex = (_semaphoreIndex + 1 % _semaphores.size());

    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &presentSemaphore;

    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &_swapchain;

    presentInfo.pImageIndices = &_index;
    presentInfo.pResults = nullptr;

    auto result = vkQueuePresentKHR(_device->queue(QueueType::GRAPHICS), &presentInfo);
    if (result != VK_SUCCESS)
        return std::unexpected(static_cast<Error>(result));
    return _index;
}

void canta::Swapchain::setPresentMode(canta::PresentMode mode) {
    _presentMode = mode;
    recreate();
}

void canta::Swapchain::resize(u32 width, u32 height) {
    _extent.width = width;
    _extent.height = height;
    recreate();
}

void canta::Swapchain::createSwapchain() {
    VkSurfaceCapabilitiesKHR capabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(_device->physicalDevice(), _surface, &capabilities);

    auto mode = getValidPresentMode(_device->physicalDevice(), _surface, static_cast<VkPresentModeKHR>(_presentMode));
    auto extent = getExtent(capabilities, _extent.width, _extent.height);

    u32 formatCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(_device->physicalDevice(), _surface, &formatCount, nullptr);
    std::vector<VkSurfaceFormatKHR> formats(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(_device->physicalDevice(), _surface, &formatCount, formats.data());

    const auto defaultFormatSelector = [](Format format) -> u32 {
        if (format == Format::BGR8_UNORM)
            return 100;
        return 0;
    };

    u32 maxIndex = 0;
    u32 maxScore = 0;
    for (u32 i = 0; i < formatCount; i++) {
        auto format = formats[i];
        u32 score = _selector ? _selector(static_cast<Format>(format.format)) : defaultFormatSelector(static_cast<Format>(format.format));
        if (score > maxScore) {
            maxScore = score;
            maxIndex = i;
        }
    }
    auto format = formats[maxIndex];

    u32 imageCount = capabilities.minImageCount + 1;
    if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount)
        imageCount = capabilities.maxImageCount;

    VkSwapchainCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = _surface;

    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = format.format;
    createInfo.imageColorSpace = format.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

    createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    createInfo.queueFamilyIndexCount = 0;
    createInfo.pQueueFamilyIndices = nullptr;

    createInfo.preTransform = capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = mode;
    createInfo.clipped = true;
    createInfo.oldSwapchain = _swapchain;
    auto oldSwapchain = _swapchain;

    auto result = vkCreateSwapchainKHR(_device->logicalDevice(), &createInfo, nullptr, &_swapchain);


    u32 count = 0;
    vkGetSwapchainImagesKHR(_device->logicalDevice(), _swapchain, &count, nullptr);
    _images.resize(count);
    vkGetSwapchainImagesKHR(_device->logicalDevice(), _swapchain, &count, _images.data());

    vkDestroySwapchainKHR(_device->logicalDevice(), oldSwapchain, nullptr);
    _extent = extent;
    _format = static_cast<Format>(format.format);


    _imageViews.resize(_images.size());
    for (u32 i = 0; i < _imageViews.size(); i++) {
        VkImageViewCreateInfo imageViewCreateInfo = {};
        imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        imageViewCreateInfo.image = _images[i];
        imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        imageViewCreateInfo.format = static_cast<VkFormat>(_format);

        imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

        imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
        imageViewCreateInfo.subresourceRange.levelCount = 1;
        imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
        imageViewCreateInfo.subresourceRange.layerCount = 1;

        vkCreateImageView(_device->logicalDevice(), &imageViewCreateInfo, nullptr, &_imageViews[i]);
        _device->setDebugName(_imageViews[i], std::format("swapchain_view_{}", i));
    }

    _imageHandles.resize(_images.size());
    for (u32 i = 0; i < _imageHandles.size(); i++) {
        _imageHandles[i] = _device->registerImage({
            .width = _extent.width,
            .height = _extent.height,
            .depth = 1,
            .format = _format,
            .mipLevels = 1,
            .layers = 1,
            .usage = ImageUsage::COLOUR_ATTACHMENT | ImageUsage::TRANSFER_DST | ImageUsage::TRANSFER_SRC,
            .name = std::format("swapchain_image_{}", i)
        }, _images[i], _imageViews[i]);
    }
}

void canta::Swapchain::createSemaphores() {
    if (!_semaphores.empty())
        return;

    for (u32 i = 0; i < _imageViews.size(); i++) {
        auto acquire = _device->createSemaphore({
            .name = std::format("swapchain_semaphore_{}_acquire", i)
        }).value();
        auto present = _device->createSemaphore({
            .name = std::format("swapchain_semaphore_{}_present", i)
        }).value();
        _semaphores.push_back({ std::move(acquire), std::move(present) });
    }
}

void canta::Swapchain::recreate() {
    vkDeviceWaitIdle(_device->logicalDevice());
    for (auto& view : _imageViews)
        vkDestroyImageView(_device->logicalDevice(), view, nullptr);

    createSwapchain();
}