#include <Canta/Device.h>
#include <Canta/SDLWindow.h>

#include <Canta/ResourceList.h>

int main() {

    canta::SDLWindow window("Hello Triangle", 1920, 1080);

    auto device = canta::Device::create({
        .applicationName = "hello_triangle",
        .instanceExtensions = window.requiredExtensions()
    }).value();

    auto swapchain = device->createSwapchain({
        .window = &window
    });
    auto commandPool = device->createCommandPool({
        .queueType = canta::QueueType::GRAPHICS
    });
    auto& commandBuffer = commandPool->getBuffer();


    canta::ResourceList<i32> resourceList;

    i32 index = resourceList.allocate();
    auto handle = resourceList.getHandle(index);


    auto handle2 = handle;

    std::printf("%d, %d, %d\n", *handle2, handle2.index(), handle2.count());

    {
        auto handle3 = handle;
        std::printf("%d, %d, %d\n", *handle2, handle2.index(), handle2.count());
    }
    std::printf("%d, %d, %d\n", *handle2, handle2.index(), handle2.count());

    handle = handle2;
    std::printf("%d, %d, %d\n", *handle2, handle2.index(), handle2.count());

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

        auto waits = std::to_array({
            { swapchain->frameSemaphore(), swapchain->framePrevValue() },
            swapchain->acquireSemaphore()->getPair()
        });
        auto signals = std::to_array({
            swapchain->frameSemaphore()->getPair(),
            swapchain->presentSemaphore()->getPair()
        });

        commandBuffer.begin();
        commandBuffer.end();

        commandBuffer.submit(waits, signals);

        swapchain->present();
    }
    return 0;
}