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
    std::swap(_name, rhs._name);
    std::swap(_views, rhs._views);
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
    std::swap(_name, rhs._name);
    std::swap(_views, rhs._views);
    return *this;
}

canta::ImageView::~ImageView() noexcept {
    if (!_device || !_image)
        return;
    vkDestroyImageView(_device->logicalDevice(), _view, nullptr);
}

canta::ImageView::ImageView(canta::ImageView &&rhs) noexcept {
    std::swap(_device, rhs._device);
    std::swap(_view, rhs._view);
    std::swap(_image, rhs._image);
}

auto canta::ImageView::operator=(canta::ImageView &&rhs) noexcept -> ImageView & {
    std::swap(_device, rhs._device);
    std::swap(_view, rhs._view);
    std::swap(_image, rhs._image);
    return *this;
}

auto canta::Image::createView(ImageView::CreateInfo info) const -> ImageViewHandle {
    info.image = this;
    return _device->createImageView(info);
}

auto canta::Image::mipView(u32 mip) -> ImageViewHandle {
    if (_views.size() > mip)
        return _views[mip];

    u32 i = 1;
    while (_views.size() <= mip) {
        _views.push_back(createView({
            .image = this,
            .mipLevel = i++,
            .levelCount = 1
        }));
    }
    return _views[mip];
}