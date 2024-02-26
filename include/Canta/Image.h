#ifndef CANTA_IMAGE_H
#define CANTA_IMAGE_H

#include <Ende/platform.h>
#include <volk.h>
#include <Canta/Enums.h>
#include <string>
#include <vk_mem_alloc.h>

namespace canta {

    class Device;

    class Image {
    public:

        struct CreateInfo {
            u32 width = 1;
            u32 height = 1;
            u32 depth = 1;
            Format format = Format::RGBA8_UNORM;
            u32 mipLevels = 1;
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


        class View {
        public:

            struct CreateInfo {
                u32 mipLevel = 0;
                u32 levelCount = 0;
                u32 layer = 0;
                u32 layerCount = 0;
                ImageViewType type = ImageViewType::AUTO;
                Format format = Format::UNDEFINED;
            };

            View() = default;
            ~View();

            View(View&& rhs) noexcept;
            auto operator=(View&& rhs) noexcept -> View&;

            auto view() const -> VkImageView { return _view; }

        private:
            friend Image;
            friend Device;

            Device* _device = nullptr;
            VkImageView _view = VK_NULL_HANDLE;
            const Image* _image = nullptr;

        };

        auto createView(View::CreateInfo info) const -> View;

        auto defaultView() const -> const View& { return _defaultView; }

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
        View _defaultView = {};
        std::string _name = {};

    };

}

#endif //CANTA_IMAGE_H
