#include <Canta/Device.h>
#include <Canta/SDLWindow.h>
#include <Canta/util.h>
#include <Ende/math/Vec.h>
#include <random>
#include <Canta/PipelineManager.h>
#include <Canta/RenderGraph.h>
#include <imgui.h>
#include <Canta/ImGuiContext.h>
#include <Canta/UploadBuffer.h>

#include <Canta/debug/PipelineManagerDebugger.h>
#include <Canta/debug/RenderGraphDebugger.h>

#include "Canta/debug/CommandQueueDebugger.h"
#include "Canta/debug/ui.h"

int main() {

    std::printf("%s", canta::formatString(canta::Format::RGBA8_UNORM));
    std::printf("%s", canta::formatString(canta::Format::RGBA32_SFLOAT));
    std::printf("%s", canta::formatString(canta::Format::R32_UINT));
    std::printf("%s", canta::formatString(canta::Format::BC7_UNORM));
    std::printf("%s", canta::formatString(canta::Format::R16_UNORM));

    canta::SDLWindow window("Hello Triangle", 1920, 1080);

    auto device = TRY_MAIN(canta::Device::create({
        .applicationName = "hello_triangle",
        .enableMeshShading = false,
        // .enableAsyncComputeQueue = false,
        // .enableAsyncTransferQueue = false,
        .instanceExtensions = window.requiredExtensions(),
        .logLevel = spdlog::level::warn,
    }));

    auto swapchain = device->createSwapchain({
        .window = &window
    });

    auto pipelineManager = canta::PipelineManager::create({
        .device = device.get(),
        .rootPath = CANTA_SRC_DIR
    });

    auto imguiContext = canta::ImGuiContext::create({
        .device = device.get(),
        .window = &window,
        .pipelineManager = &pipelineManager,
    });

    auto pipeline = TRY_MAIN(pipelineManager.getPipeline(CANTA_SRC_DIR"/examples/particles_update.pipeline", std::to_array({
        canta::Macro{
            .name = "ADDITIONAL",
            .value = "TEST"
        }
    }), {canta::SpecializationConstant{.name = "screen_width", .value = 780}}));

    for (auto& type : pipeline->interface().getTypeList()) {
        std::printf("%s - size: %d\n", type.name.c_str(), type.size);
    }

    auto pipelineDraw = TRY_MAIN(pipelineManager.getPipeline(canta::Pipeline::CreateInfo{
        .compute = {
            .module = TRY_MAIN(pipelineManager.getShader({
                .path = "examples/particles.slang",
                .stage = canta::ShaderStage::COMPUTE,
                .name = "particleDraw"
            })),
            .entryPoint = "drawMain"
        },
        .localSize = ende::math::Vec<3, u32>{ 64, 1, 1 },
        .specializationConstants = {
            canta::SpecializationConstant{.id = 0, .name = "screen_width", .value = 780}
        },
        .name = "particles_draw"
    }));

    auto testModule = pipelineManager.getShader({
        .path = CANTA_SRC_DIR"/examples/hello.slang",
        .stage = canta::ShaderStage::COMPUTE,
        .name = "hello"
    });

    constexpr const u32 numParticles = 1000;

    struct Particle {
        ende::math::Vec<2, f32> position;
        ende::math::Vec<2, f32> velocity;
        ende::math::Vec<3, f32> colour;
        i32 radius;
    };

    auto buffer = device->createBuffer({
        .size = static_cast<u32>(sizeof(Particle) * numParticles),
        .usage = canta::BufferUsage::STORAGE,
        .type = canta::MemoryType::STAGING,
        .persistentlyMapped = true,
        .name = "particle_positions"
    });

    std::random_device randomDevice;
    std::default_random_engine re(randomDevice());
    std::uniform_real_distribution<f32> xDist(0, window.extent().x());
    std::uniform_real_distribution<f32> yDist(0, window.extent().y());
    std::uniform_real_distribution<f32> velDist(0, 10);
    std::uniform_real_distribution<f32> colourDist(0, 1);

    std::array<Particle, numParticles> particles = {};
    for (u32 i = 0; i < numParticles; i++) {
        particles[i] = {
                .position = { xDist(re), yDist(re) },
                .velocity = { velDist(re), velDist(re) },
                .colour = { colourDist(re), colourDist(re), colourDist(re) },
                .radius = static_cast<i32>(velDist(re))
        };
    }

    auto uploadBuffer = TRY_MAIN(canta::UploadBuffer::create({
        .device = device.get(),
        .size = 1 << 16
    }));
    uploadBuffer.upload(buffer, particles);
    uploadBuffer.flushStagedData();
    if (!uploadBuffer.wait()) return -1;

    // auto renderGraph = TRY_MAIN(canta::RenderGraph::create({
    //     .device = device.get(),
    //     .multiQueue = false,
    //     .name = "Renderer"
    // }));

    auto renderGraph = TRY_MAIN(canta::RenderGraph::create({
        .device = device.get(),
        .multiQueue = false,
    }));

    auto renderGraphDebugger = canta::RenderGraphDebugger::create({
        .renderGraph = &renderGraph,
    });

    auto pipelineManagerDebugger = canta::PipelineManagerDebugger::create({
        .pipelineManager = &pipelineManager,
    });

    auto commandQueueDebugger = canta::CommandQueueDebugger::create({
        .device = device.get(),
    });

    f64 dt = 1.f / 60;

    while (!window.shouldClose()) {
        window.processEvents(&imguiContext);

        // device->startFrameCapture();
        device->beginFrame();
        device->gc();
        pipelineManager.reloadAll();
        uploadBuffer.clearSubmitted();

        imguiContext.beginFrame();
        ImGui::ShowDemoWindow();

        if (ImGui::Begin("Stats")) {

            ImGui::Text("Delta Time: %f", dt);

            const char* refreshModes[] = { "FIFO", "MAILBOX", "IMMEDIATE" };
            i32 refreshModeIndex = 0;
            switch (swapchain->getPresentMode()) {
                case canta::PresentMode::FIFO:
                    refreshModeIndex = 0;
                    break;
                case canta::PresentMode::MAILBOX:
                    refreshModeIndex = 1;
                    break;
                case canta::PresentMode::IMMEDIATE:
                    refreshModeIndex = 2;
                    break;
            }
            if (ImGui::Combo("PresentMode", &refreshModeIndex, refreshModes, 3)) {
                TRY_MAIN(device->waitIdle());
                switch (refreshModeIndex) {
                    case 0:
                        swapchain->setPresentMode(canta::PresentMode::FIFO);
                        break;
                    case 1:
                        swapchain->setPresentMode(canta::PresentMode::MAILBOX);
                        break;
                    case 2:
                        swapchain->setPresentMode(canta::PresentMode::IMMEDIATE);
                        break;
                }
            }

            auto multiQueue = renderGraph.multiQueue();
            if (ImGui::Checkbox("MultiQueue", &multiQueue))
                renderGraph.setMultiQueue(multiQueue);
            const char* modes[] = { "DISABLED", "PER_PASS", "PER_GROUP" };
            static int timingModeIndex = static_cast<int>(renderGraph.timingMode());
            if (ImGui::Combo("Timing Mode", &timingModeIndex, modes, 3)) {
                switch (timingModeIndex) {
                    case 0:
                        renderGraph.setTimingMode(canta::RenderGraph::QueryMode::DISABLED);
                        break;
                    case 1:
                        renderGraph.setTimingMode(canta::RenderGraph::QueryMode::PER_PASS);
                        break;
                    case 2:
                        renderGraph.setTimingMode(canta::RenderGraph::QueryMode::PER_GROUP);
                        break;
                }
            }
            static int statsModeIndex = static_cast<int>(renderGraph.statsMode());
            if (ImGui::Combo("Stats Mode", &statsModeIndex, modes, 3)) {
                switch (statsModeIndex) {
                    case 0:
                        renderGraph.setStatsMode(canta::RenderGraph::QueryMode::DISABLED);
                        break;
                    case 1:
                        renderGraph.setStatsMode(canta::RenderGraph::QueryMode::PER_PASS);
                        break;
                    case 2:
                        renderGraph.setStatsMode(canta::RenderGraph::QueryMode::PER_GROUP);
                        break;
                }
            }

            // auto pipelineStatsEnabled = renderGraph.pipelineStatisticsEnabled();
            // if (ImGui::Checkbox("RenderGraph PiplelineStats", &pipelineStatsEnabled))
            // renderGraph.setPipelineStatisticsEnabled(pipelineStatsEnabled);
            // auto individualPipelineStatistics = renderGraph.individualPipelineStatistics();
            // if (ImGui::Checkbox("RenderGraph Per Pass PiplelineStats", &individualPipelineStatistics))
            //     renderGraph.setIndividualPipelineStatistics(individualPipelineStatistics);
            //
            auto timers = renderGraph.timers();
            for (auto& timer : timers) {
                ImGui::Text("%s: %f ms", timer.name.data(), TRY_MAIN(timer.timer.result()) / 1000000.f);
            }
            auto pipelineStatistics = renderGraph.statistics();
            for (u32 statIndex = 0; auto& pipelineStats : pipelineStatistics) {
                ImGui::PushID(statIndex++);
                if (ImGui::TreeNode(pipelineStats.name.data())) {
                    auto stats = TRY_MAIN(pipelineStats.statistics.result());
                    canta::drawPipelineStats(stats);
                    ImGui::TreePop();
                }
                ImGui::PopID();
            }

            auto memoryUsage = device->memoryUsage();
            auto softMemoryUsage = device->softMemoryUsage();
            canta::drawMemoryUsage(memoryUsage);
            canta::drawMemoryUsage(softMemoryUsage);

            auto resourceStats = device->resourceStats();
            canta::drawResourceStats(resourceStats);

            const auto renderGraphStats = renderGraph.stats();
            ImGui::Text("Passes: %d", renderGraphStats.passes);
            ImGui::Text("Resource: %d", renderGraphStats.resources);
            ImGui::Text("Image: %d", renderGraphStats.images);
            ImGui::Text("Buffers: %d", renderGraphStats.buffers);
            ImGui::Text("Command Buffers: %d", renderGraphStats.commandBuffers);

            if (ImGui::TreeNode("markers")) {
                auto& markers = device->getFrameDebugMarkers(device->framePrevValue() % canta::FRAMES_IN_FLIGHT);
                for (u32 i = 0; auto& marker : markers) {
                    auto markerType = canta::util::getMarkerType(marker);
                    auto markerStage = canta::util::getMarkerStage(marker);
                    ImGui::Text("(%u) %s in %s", i++, canta::util::markerString(markerType), canta::pipelineStageString(markerStage));
                }
                ImGui::TreePop();
            }
        }
        ImGui::End();

        if (ImGui::Begin("Particles")) {
            if (ImGui::Button("Randomise")) {
                particles = {};
                for (u32 i = 0; i < numParticles; i++) {
                    particles[i] = {
                        .position = { xDist(re), yDist(re) },
                        .velocity = { velDist(re), velDist(re) },
                        .colour = { colourDist(re), colourDist(re), colourDist(re) },
                        .radius = static_cast<i32>(velDist(re))
                    };
                }

                uploadBuffer.upload(buffer, particles);
                uploadBuffer.flushStagedData();
            }
            if (ImGui::Button("Allocate")) {
                auto throwAwayBuffer = device->createBuffer({
                    .size = 10000,
                    .usage = canta::BufferUsage::STORAGE,
                    .type = canta::MemoryType::DEVICE,
                    .name = "throw_away",
                });
            }
        }
        ImGui::End();

        renderGraphDebugger.render();
        renderGraphDebugger.drawRenderGraph();

        pipelineManagerDebugger.render();

        commandQueueDebugger.render();

        ImGui::Render();

        renderGraph.reset();

        auto imageIndex = renderGraph.addImage({
            .width = 1920,
            .height = 1080,
            .mips = 4,
            .name = "image"
        });

        auto particleBufferIndex = renderGraph.addBuffer({
            .buffer = buffer,
            .name = "particles_buffer"
        });

        auto swapchainIndex = TRY_MAIN(renderGraph.acquire(&*swapchain));

        auto clearedImage = TRY_MAIN(renderGraph.transfer("clear_image").clear(imageIndex));

        const auto particlesGroup = renderGraph.addGroup("particles", { 0, 1, 0, 1 });

        auto movedParticles = TRY_MAIN(renderGraph.compute("particles_move", pipeline, particlesGroup)
            .pushConstants(canta::Write(particleBufferIndex), numParticles, static_cast<f32>(dt))
            .dispatchThreads(numParticles).output<canta::BufferIndex>());

        auto drawnParticles = TRY_MAIN(renderGraph.compute("particles_draw", pipelineDraw, particlesGroup)
            .addStorageImageWrite(clearedImage)
            .pushConstants(canta::Read(movedParticles), canta::Read(clearedImage), numParticles)
            .dispatchThreads(numParticles).output<canta::ImageIndex>());

        auto blittedSwapchain = TRY_MAIN(renderGraph.graphics("blit_to_swapchain").blit(drawnParticles, swapchainIndex).output<canta::ImageIndex>());
        auto uiSwapchain = TRY_MAIN(renderGraph.graphics("ui").imgui(imguiContext, blittedSwapchain));

        auto presentOutput = TRY_MAIN(renderGraph.present(&*swapchain, uiSwapchain));

        // renderGraph.setRoot(uiSwapchain);
        renderGraph.setRoot(presentOutput);

        // renderGraphDebugger.setRoot(particlesDrawPass);
        renderGraphDebugger.setBaseResource(imageIndex);


        // renderGraphDebugger.debug();

        if (!renderGraph.compile())
            return -1;
        auto waits = std::to_array({ canta::SemaphorePair(device->frameSemaphore(), device->framePrevValue()), canta::SemaphorePair(uploadBuffer.timeline()) });
        auto signals = std::to_array({ canta::SemaphorePair(device->frameSemaphore(), device->frameSemaphore()->value())});

        TRY_MAIN(renderGraph.run(waits, signals));

        dt = device->endFrame();
        // device->endFrameCapture();
        // printf("frame: %lu\n", device->frameValue());
    }

    TRY_MAIN(device->waitIdle());
    return 0;
}
