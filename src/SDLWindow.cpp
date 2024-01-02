#include "Canta/SDLWindow.h"
#include <SDL2/SDL_vulkan.h>
#include <Canta/Device.h>

canta::SDLWindow::SDLWindow(const char *title, u32 width, u32 height, u32 flags) {
    _window = SDL_CreateWindow(title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, flags | SDL_WINDOW_VULKAN | SDL_WINDOW_SHOWN);
}

canta::SDLWindow::~SDLWindow() {
    SDL_DestroyWindow(_window);
}

auto canta::SDLWindow::requiredExtensions() -> std::vector<const char*> {
    u32 extensionCount = 0;
    SDL_Vulkan_GetInstanceExtensions(_window, &extensionCount, nullptr);
    std::vector<const char*> extensionNames(extensionCount);
    SDL_Vulkan_GetInstanceExtensions(_window, &extensionCount, extensionNames.data());
    return std::move(extensionNames);
}

auto canta::SDLWindow::surface(Device& device) -> VkSurfaceKHR {
    if (_surface != VK_NULL_HANDLE)
        return _surface;
    if (!SDL_Vulkan_CreateSurface(_window, device.instance(), &_surface))
        throw "unable create surface";
    return _surface;
}

auto canta::SDLWindow::extent() -> ende::math::Vec<2, u32> {
    i32 width, height;
    SDL_GetWindowSize(_window, &width, &height);
    return { static_cast<u32>(width), static_cast<u32>(height) };
}