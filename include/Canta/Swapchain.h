#ifndef CANTA_SWAPCHAIN_H
#define CANTA_SWAPCHAIN_H

#include <Ende/platform.h>
#include <volk.h>
#include <vector>
#include <functional>
#include <expected>
#include <Canta/Enums.h>
#include <Canta/Semaphore.h>
#include <Canta/Window.h>
#include <Canta/ResourceList.h>

namespace canta {

    class Device;

    class Image;
    using ImageHandle = Handle<Image, ResourceList<Image>>;

    class Swapchain {
    public:

        struct CreateInfo {
            Window* window = nullptr;
            std::function<u32(Format)> selector = {};
        };

        ~Swapchain();

        Swapchain(Swapchain&& rhs) noexcept;
        auto operator=(Swapchain&& rhs) noexcept -> Swapchain&;

        auto acquire() -> std::expected<ImageHandle, VulkanError>;

        auto present() -> std::expected<u32, VulkanError>;

        auto acquireSemaphore() -> Semaphore* { return &_semaphores[_semaphoreIndex].acquire; }
        auto presentSemaphore() -> Semaphore* { return &_semaphores[_semaphoreIndex].present; }

        void setPresentMode(PresentMode mode);
        auto getPresentMode() const -> PresentMode { return _presentMode; }

        auto format() const -> Format { return _format; }
        auto width() const -> u32 { return _extent.width; }
        auto height() const -> u32 { return _extent.height; }
        auto index() const -> u32 { return _index; }

        void resize(u32 width, u32 height);

    private:
        friend Device;

        Swapchain() = default;

        void createSwapchain();
        void createSemaphores();

        void recreate();

        Device* _device = nullptr;
        VkSurfaceKHR _surface = VK_NULL_HANDLE;
        VkSwapchainKHR _swapchain = VK_NULL_HANDLE;
        PresentMode _presentMode = PresentMode::FIFO;
        VkExtent2D _extent = {};
        Format _format = Format::UNDEFINED;
        u32 _index = 0;

        std::vector<ImageHandle> _imageHandles = {};
        std::vector<VkImage> _images = {};
        std::vector<VkImageView> _imageViews = {};
        struct SemaphorePair {
            Semaphore acquire;
            Semaphore present;
        };
        std::vector<SemaphorePair> _semaphores = {};
        u32 _semaphoreIndex = 0;

        std::function<u32(Format)> _selector = {};

    };

}


#endif //CANTA_SWAPCHAIN_H
