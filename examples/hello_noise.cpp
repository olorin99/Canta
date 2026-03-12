
#include <Canta/RenderGraph.h>
#include <Canta/PipelineManager.h>
#include <Ende/time/time.h>
#include "Canta/ImGuiContext.h"
#include "Canta/SDLWindow.h"
#include "Canta/util/random.h"

int main() {

    auto window = canta::SDLWindow("Hello Noise", 1920, 1080);

    auto device = TRY_MAIN(canta::Device::create({
        .applicationName = "hello_noise",
        .enableMeshShading = false,
        .instanceExtensions = window.requiredExtensions(),
        .logLevel = spdlog::level::warn,
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
    });

    auto renderGraph = TRY_MAIN(canta::RenderGraph::create({
        .device = device.get(),
        .multiQueue = false,
        // .name = "render_graph"
    }));

    i32 noiseTypeIndex = 0;
    i32 seed = 0;
    f32 timeScale = 1.0f;
    i32 octaves = 6;

    auto clock = ende::time::SystemTime::now();

    f64 dt = 1.f / 60;
    while (!window.shouldClose()) {
        window.processEvents(&imgui);

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

        auto swapIndex = TRY_MAIN(renderGraph.acquire(&*swapchain));

        auto uiSwapImage = TRY_MAIN(renderGraph.graphics("blit_to_swapchain").blit(noiseImage, swapIndex).output<canta::ImageIndex>());

        auto uiImage = TRY_MAIN(renderGraph.graphics("ui").imgui(imgui, uiSwapImage));

        auto root = TRY_MAIN(renderGraph.present(&*swapchain, uiImage));


        renderGraph.setRoot(root);

        TRY_MAIN(renderGraph.compile());

        auto waits = std::to_array({
            canta::SemaphorePair{device->frameSemaphore(), device->framePrevValue()},
        });
        auto signals = std::to_array({
            canta::SemaphorePair{device->frameSemaphore()},
        });
        TRY_MAIN(renderGraph.run(waits, signals));

        dt = device->endFrame();
    }


    TRY_MAIN(device->waitIdle());
    return 0;
}
