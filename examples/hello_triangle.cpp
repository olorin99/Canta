#include <Canta/Device.h>

#include <Canta/SDLWindow.h>

int main() {

    canta::SDLWindow window("Hello Triangle", 1920, 1080);

    auto device = canta::Device::create({
        .applicationName = "hello_triangle",
        .instanceExtensions = window.requiredExtensions()
    }).value();

    auto swapchain = device->createSwapchain({
        .window = &window
    });


    bool running = true;

    SDL_Event event;
    while (running) {
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    running = false;
                    break;
            }
        }

        auto index = swapchain->acquire();

//        swapchain->present();
    }
    return 0;
}