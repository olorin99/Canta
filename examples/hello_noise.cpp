
#include <Canta/RenderGraph.h>
#include <Canta/PipelineManager.h>

#include "../cmake-build-debug-clang/_deps/ende-src/include/Ende/time/time.h"
#include "Canta/ImGuiContext.h"
#include "Canta/SDLWindow.h"
#include "Canta/util/random.h"

int main() {

    auto window = canta::SDLWindow("Hello Noise", 1920, 1080);

    auto device = TRY_MAIN(canta::Device::create({
        .applicationName = "hello_noise",
        .enableMeshShading = false,
        .instanceExtensions = window.requiredExtensions()
    }));

    auto swapchain = device->createSwapchain({
        .window = &window,
    });

    auto pipelineManger = canta::PipelineManager::create({
        .device = device.get(),
        .rootPath = CANTA_SRC_DIR
    });

    auto imgui = canta::ImGuiContext::create({
        .device = device.get(),
        .window = &window,
        .pipelineManager = &pipelineManger,
    });

    auto renderGraph = TRY_MAIN(canta::RenderGraph::create({
        .device = device.get(),
        .multiQueue = false,
        .name = "render_graph"
    }));

    i32 noiseTypeIndex = 0;
    i32 seed = 0;
    f32 timeScale = 1.0f;
    i32 octaves = 6;

    auto clock = ende::time::SystemTime::now();

    f64 dt = 1.f / 60;
    bool running = true;
    SDL_Event e;
    while (running) {
        while (SDL_PollEvent(&e)) {
            switch (e.type) {
                case SDL_QUIT:
                    running = false;
                    break;
            }
            imgui.processEvent(&e);
        }

        device->beginFrame();
        device->gc();

        imgui.beginFrame();

        if (ImGui::Begin("Noise")) {
            const char* noiseTypes[] = { "Random", "Perlin" };
            if (ImGui::Combo("Type", &noiseTypeIndex, noiseTypes, 2)) {
                if (noiseTypeIndex == 1 && seed == 0) {
                    seed = 1;
                }
            }

            ImGui::SliderInt("Seed", &seed, 0, 1e6);
            ImGui::SliderFloat("Time Scale", &timeScale, 0.0f, 1.0f);
            ImGui::SliderInt("Octaves", &octaves, 0, 10);
        }
        ImGui::End();

        ImGui::Render();


        renderGraph.reset();

        auto swapImage = TRY_MAIN(swapchain->acquire());

        auto swapIndex = renderGraph.addImage({
            .handle = swapImage,
            .name = "swapchain_image"
        });

        canta::ImageIndex noiseImage = {};
        assert(noiseTypeIndex <= 1);
        switch (noiseTypeIndex) {
            case 0:
                noiseImage = canta::generateRandomNoise(renderGraph, 1920, 1080, seed);
                break;
            case 1:
                noiseImage = canta::generatePerlinNoise(renderGraph, 1920, 1080, {
                    .time = (clock.elapsed().milliseconds() / 1000.f) * timeScale,
                    .octaves = static_cast<u32>(octaves),
                    .seed = static_cast<u32>(seed)
                });
                break;
            default:
                noiseImage = canta::generateRandomNoise(renderGraph, 1920, 1080, seed);
                break;
        }
        // auto noiseImage = canta::generateRandomNoise(renderGraph, 1920, 1080, seed);
        // auto noiseImage = canta::generatePerlinNoise(renderGraph, 1920, 1080, {.time = (clock.elapsed().milliseconds() / 1000.f) * timeScale, .seed = static_cast<u32>(seed)});

        auto uiSwapImage = TRY_MAIN(renderGraph.addBlitPass("blit_to_swapchain", noiseImage, swapIndex).aliasImageOutput(0));

        auto swapchainFormat = swapchain->format();
        auto backbuffer = TRY_MAIN(renderGraph.addPass({.name = "ui", .type = canta::PassType::GRAPHICS})
            .setManualPipeline(true)
            .addColourRead(uiSwapImage)
            .addColourWrite(swapIndex)
            .setExecuteFunction([&imgui, swapchainFormat] (auto& cmd, auto& graph) {
                imgui.render(ImGui::GetDrawData(), cmd, swapchainFormat);
            }).aliasImageOutput(0));

        renderGraph.setBackbuffer(backbuffer);

        TRY_MAIN(renderGraph.compile());

        auto waits = std::to_array({
            canta::SemaphorePair{device->frameSemaphore(), device->framePrevValue()},
            canta::SemaphorePair{swapchain->acquireSemaphore()}
        });
        auto signals = std::to_array({
            canta::SemaphorePair{device->frameSemaphore()},
            canta::SemaphorePair{swapchain->presentSemaphore()}
        });
        TRY_MAIN(renderGraph.execute(waits, signals));

        TRY_MAIN(swapchain->present());

        dt = device->endFrame();
    }


    return 0;
}
