#include "Canta/Image.h"
#include <Canta/Device.h>

canta::Image::~Image() {
    if (!_device || !_allocation)
        return;
    vmaDestroyImage(_device->allocator(), _image, _allocation);
}

canta::Image::Image(canta::Image &&rhs) noexcept {
    std::swap(_device, rhs._device);
    std::swap(_image, rhs._image);
    std::swap(_allocation, rhs._allocation);
    std::swap(_width, rhs._width);
    std::swap(_height, rhs._height);
    std::swap(_depth, rhs._depth);
    std::swap(_layers, rhs._layers);
    std::swap(_mips, rhs._mips);
    std::swap(_type, rhs._type);
    std::swap(_format, rhs._format);
    std::swap(_usage, rhs._usage);
    std::swap(_layout, rhs._layout);
    std::swap(_defaultView, rhs._defaultView);
    std::swap(_name, rhs._name);
}

auto canta::Image::operator=(canta::Image &&rhs) noexcept -> Image & {
    std::swap(_device, rhs._device);
    std::swap(_image, rhs._image);
    std::swap(_allocation, rhs._allocation);
    std::swap(_width, rhs._width);
    std::swap(_height, rhs._height);
    std::swap(_depth, rhs._depth);
    std::swap(_layers, rhs._layers);
    std::swap(_mips, rhs._mips);
    std::swap(_type, rhs._type);
    std::swap(_format, rhs._format);
    std::swap(_usage, rhs._usage);
    std::swap(_layout, rhs._layout);
    std::swap(_defaultView, rhs._defaultView);
    std::swap(_name, rhs._name);
    return *this;
}

canta::Image::View::~View() noexcept {
    if (!_device || !_image)
        return;
    vkDestroyImageView(_device->logicalDevice(), _view, nullptr);
}

canta::Image::View::View(canta::Image::View &&rhs) noexcept {
    std::swap(_device, rhs._device);
    std::swap(_view, rhs._view);
    std::swap(_image, rhs._image);
}

auto canta::Image::View::operator=(canta::Image::View &&rhs) noexcept -> View & {
    std::swap(_device, rhs._device);
    std::swap(_view, rhs._view);
    std::swap(_image, rhs._image);
    return *this;
}

auto canta::Image::createView(View::CreateInfo info) const -> View {
    VkImageViewCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;

    auto type = info.type;
    if (type == ImageViewType::AUTO) {
        switch (_type) {
            case ImageType::IMAGE1D:
                if (_layers > 1)
                    type = ImageViewType::VIEW1D_ARRAY;
                else
                    type = ImageViewType::VIEW1D;
                break;
            case ImageType::IMAGE2D:
                if (_layers == 6)
                    type = ImageViewType::VIEW_CUBE;
                else if (_layers > 1)
                    type = ImageViewType::VIEW2D_ARRAY;
                else
                    type = ImageViewType::VIEW2D;
                break;
            case ImageType::IMAGE3D:
                if (_layers > 1)
                    type = ImageViewType::VIEW3D_ARRAY;
                else
                    type = ImageViewType::VIEW3D;
                break;
            case ImageType::AUTO:
                break;
        }
    }

    auto format = info.format;
    if (format == Format::UNDEFINED)
        format = _format;

    createInfo.image = _image;
    createInfo.viewType = static_cast<VkImageViewType>(type);
    createInfo.format = static_cast<VkFormat>(format);
    createInfo.subresourceRange.aspectMask = isDepthFormat(format) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
    createInfo.subresourceRange.baseMipLevel = info.mipLevel;
    createInfo.subresourceRange.levelCount = info.levelCount == 0 ? VK_REMAINING_MIP_LEVELS : info.levelCount;
    createInfo.subresourceRange.baseArrayLayer = info.layer;
    createInfo.subresourceRange.layerCount = info.layerCount == 0 ? VK_REMAINING_ARRAY_LAYERS : info.layerCount;

    VkImageView view;
    vkCreateImageView(_device->logicalDevice(), &createInfo, nullptr, &view);

    View v = {};
    v._device = _device;
    v._image = this;
    v._view = view;

    return v;
}