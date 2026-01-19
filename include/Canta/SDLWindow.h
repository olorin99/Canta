#ifndef CANTA_SDLWINDOW_H
#define CANTA_SDLWINDOW_H

#include <Canta/Window.h>
#include <SDL2/SDL.h>

namespace canta {

    class Device;

    class SDLWindow : public Window {
    public:

        SDLWindow(const char* title, u32 width, u32 height, u32 flags = 0);

        ~SDLWindow() override;

        [[nodiscard]] auto requiredExtensions() -> std::vector<const char*> override;

        [[nodiscard]] auto surface(Device& device) -> VkSurfaceKHR override;

        [[nodiscard]] auto extent() -> ende::math::Vec<2, u32> override;

        [[nodiscard]] auto window() -> SDL_Window* { return _window; }

    private:

        SDL_Window* _window = nullptr;
        VkSurfaceKHR _surface = VK_NULL_HANDLE;
        u32 _width = 0, _height = 0;

    };

}

#endif //CANTA_SDLWINDOW_H
