#include "Canta/SDLWindow.h"
#include <SDL2/SDL_vulkan.h>
#include <Canta/Device.h>

#include "Canta/ImGuiContext.h"

canta::SDLWindow::SDLWindow(const char *title, const u32 width, const u32 height, const u32 flags) {
    _window = SDL_CreateWindow(title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, flags | SDL_WINDOW_VULKAN | SDL_WINDOW_SHOWN);
    registerEvent(SDL_QUIT, [this] (auto&) { _shouldClose = true; });
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

void canta::SDLWindow::registerEvent(const u32 event, const std::function<void(const SDL_Event&)> &callback) {
    _eventCallbacks[event] = callback;
}

void canta::SDLWindow::processEvents(ImGuiContext* imguiContext) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (auto e = _eventCallbacks.find(event.type); e != _eventCallbacks.end()) {
            e.value()(event);
        }
        imguiContext->processEvent(&event);
    }
}
