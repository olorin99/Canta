#ifndef CANTA_IMAGE_H
#define CANTA_IMAGE_H

#include <Ende/platform.h>
#include <volk.h>
#include <Canta/Enums.h>
#include <string>
#include <vk_mem_alloc.h>
#include <Canta/ResourceList.h>

namespace canta {

    class Device;
    class Image;

    class ImageView {
    public:

        struct CreateInfo {
            const Image* image = nullptr;
            u32 mipLevel = 0;
            u32 levelCount = 0;
            u32 layer = 0;
            u32 layerCount = 0;
            ImageViewType type = ImageViewType::AUTO;
            Format format = Format::UNDEFINED;
        };

        ImageView() = default;
        ~ImageView();

        ImageView(ImageView&& rhs) noexcept;
        auto operator=(ImageView&& rhs) noexcept -> ImageView&;

        [[nodiscard]] auto view() const -> VkImageView { return _view; }

    private:
        friend Image;
        friend Device;

        Device* _device = nullptr;
        VkImageView _view = VK_NULL_HANDLE;
        const Image* _image = nullptr;

    };

    using ImageViewHandle = Handle<ImageView, ResourceList<ImageView>>;

    class Image {
    public:

        struct CreateInfo {
            u32 width = 1;
            u32 height = 1;
            u32 depth = 1;
            Format format = Format::RGBA8_UNORM;
            u32 mipLevels = 1;
            bool allocateMipViews = false;
            u32 layers = 1;
            ImageUsage usage = ImageUsage::SAMPLED | ImageUsage::TRANSFER_DST;
            ImageType type = ImageType::AUTO;
            std::string_view name = {};
        };

        Image() = default;

        ~Image();

        Image(Image&& rhs) noexcept;
        auto operator=(Image&& rhs) noexcept -> Image&;

        auto image() const -> VkImage { return _image; }
        auto width() const -> u32 { return _width; }
        auto height() const -> u32 { return _height; }
        auto depth() const -> u32 { return _depth; }
        auto layers() const -> u32 { return _layers; }
        auto mips() const -> u32 { return _mips; }
        auto format() const -> Format { return _format; }
        auto usage() const -> ImageUsage { return _usage; }
        auto layout() const -> ImageLayout { return _layout; }
        auto name() const -> std::string_view { return _name; }
        auto size() const -> u32 { return _width * _height * _depth * _layers * _mips * formatSize(_format); }




        auto createView(ImageView::CreateInfo info) const -> ImageViewHandle;

        auto defaultView() const -> ImageViewHandle { return _views.front(); }

        auto mipView(u32 mip = 0) -> ImageViewHandle;

    private:
        friend Device;

        Device* _device = nullptr;
        VkImage _image = VK_NULL_HANDLE;
        VmaAllocation _allocation = VK_NULL_HANDLE;
        u32 _width = 1;
        u32 _height = 1;
        u32 _depth = 1;
        u32 _layers = 1;
        u32 _mips = 1;
        ImageType _type = ImageType::IMAGE2D;
        Format _format = Format::RGBA8_UNORM;
        ImageUsage _usage = ImageUsage::TRANSFER_DST;
        ImageLayout _layout = ImageLayout::UNDEFINED;
        std::string _name = {};

        std::vector<ImageViewHandle> _views = {};

    };

}

#endif //CANTA_IMAGE_H
