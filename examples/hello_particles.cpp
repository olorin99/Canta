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

int main() {

    std::printf("%s", canta::formatString(canta::Format::RGBA8_UNORM));
    std::printf("%s", canta::formatString(canta::Format::RGBA32_SFLOAT));
    std::printf("%s", canta::formatString(canta::Format::R32_UINT));
    std::printf("%s", canta::formatString(canta::Format::BC7_UNORM));
    std::printf("%s", canta::formatString(canta::Format::R16_UNORM));

    canta::SDLWindow window("Hello Triangle", 1920, 1080);

    auto device = canta::Device::create({
        .applicationName = "hello_triangle",
        .instanceExtensions = window.requiredExtensions()
    }).value();

    auto swapchain = device->createSwapchain({
        .window = &window
    });

    auto imguiContext = canta::ImGuiContext::create({
        .device = device.get(),
        .window = &window
    });


    std::string particleDrawComp = R"(
#version 460

#include "canta.glsl"

layout (local_size_x = 32) in;

declareStorageImages(storageImages, image2D, writeonly);

struct Particle {
    vec2 position;
    vec2 velocity;
    vec3 colour;
    int radius;
};

declareBufferReference(ParticleBuffer,
    Particle particles[];
);

layout (push_constant) uniform Push {
    ParticleBuffer particleBuffer;
    int imageIndex;
    int maxParticles;
};

void main() {
    const uint idx = gl_GlobalInvocationID.x;
    if (idx >= maxParticles)
        return;

    Particle particle = particleBuffer.particles[idx];

    vec4 colour = vec4(particle.colour, 1);

    for (int x = -particle.radius; x < particle.radius; x++) {
        for (int y = -particle.radius; y < particle.radius; y++) {
            ivec2 position = ivec2(particle.position) + ivec2(x, y);
            imageStore(storageImages[imageIndex], position, colour);
        }
    }

    imageStore(storageImages[imageIndex], ivec2(particle.position), colour);
}
)";

    auto pipelineManager = canta::PipelineManager::create({
        .device = device.get()
    });

    auto pipeline = pipelineManager.getPipeline("/home/christian/Documents/Projects/Canta/examples/particles_update.pipeline", std::to_array({
        canta::Macro{
            .name = "ADDITIONAL",
            .value = "TEST"
        }
    }));

    for (auto& type : pipeline->interface().getTypeList()) {
        std::printf("%s - size: %d\n", type.name.c_str(), type.size);
    }

//    auto pipeline = pipelineManager.getPipeline({
//        .compute = {
//            .module = pipelineManager.getShader({
//                .path = "/home/christian/Documents/Projects/Canta/examples/particles.comp",
//                .macros = { canta::Macro{"CANTA_TEST", "HELLO"} },
//                .stage = canta::ShaderStage::COMPUTE
//            })
//        }
//    });

    auto pipelineDraw = pipelineManager.getPipeline({
        .compute = {
            .module = pipelineManager.getShader({
                .glsl = particleDrawComp,
                .stage = canta::ShaderStage::COMPUTE
            })
        }
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

    auto uploadBuffer = canta::UploadBuffer::create({
        .device = device.get(),
        .size = 1 << 16
    });
    uploadBuffer.upload(buffer, particles);
    uploadBuffer.flushStagedData();
    uploadBuffer.wait();

    auto renderGraph = canta::RenderGraph::create({
        .device = device.get(),
        .name = "Renderer"
    });

    f64 dt = 1.f / 60;
    bool running = true;
    SDL_Event event;
    while (running) {
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    running = false;
                    break;
            }
            imguiContext.processEvent(&event);
        }
        device->beginFrame();
        device->gc();
        pipelineManager.reloadAll();
        uploadBuffer.clearSubmitted();

        imguiContext.beginFrame();
        ImGui::ShowDemoWindow();

        if (ImGui::Begin("Stats")) {

            ImGui::Text("Delta Time: %f", dt);

            auto timingEnabled = renderGraph.timingEnabled();
            if (ImGui::Checkbox("RenderGraph Timing", &timingEnabled))
                renderGraph.setTimingEnabled(timingEnabled);
            const char* modes[] = { "PER_PASS", "PER_GROUP", "SINGLE" };
            static int modeIndex = 0;
            if (ImGui::Combo("TimingMode", &modeIndex, modes, 3)) {
                switch (modeIndex) {
                    case 0:
                        renderGraph.setTimingMode(canta::RenderGraph::TimingMode::PER_PASS);
                        break;
                    case 1:
                        renderGraph.setTimingMode(canta::RenderGraph::TimingMode::PER_GROUP);
                        break;
                    case 2:
                        renderGraph.setTimingMode(canta::RenderGraph::TimingMode::SINGLE);
                        break;
                }
            }
            auto pipelineStatsEnabled = renderGraph.pipelineStatisticsEnabled();
            if (ImGui::Checkbox("RenderGraph PiplelineStats", &pipelineStatsEnabled))
                renderGraph.setPipelineStatisticsEnabled(pipelineStatsEnabled);
            auto individualPipelineStatistics = renderGraph.individualPipelineStatistics();
            if (ImGui::Checkbox("RenderGraph Per Pass PiplelineStats", &individualPipelineStatistics))
                renderGraph.setIndividualPipelineStatistics(individualPipelineStatistics);

            auto timers = renderGraph.timers();
            for (auto& timer : timers) {
                ImGui::Text("%s: %f ms", timer.first.data(), timer.second.result().value() / 1000000.f);
            }
            auto pipelineStatistics = renderGraph.pipelineStatistics();
            for (auto& pipelineStats : pipelineStatistics) {
                if (ImGui::TreeNode(pipelineStats.first.c_str())) {
                    auto stats = pipelineStats.second.result().value();
                    ImGui::Text("Input Assembly Vertices: %d", stats.inputAssemblyVertices);
                    ImGui::Text("Input Assembly Primitives: %d", stats.inputAssemblyPrimitives);
                    ImGui::Text("Vertex Shader Invocations: %d", stats.vertexShaderInvocations);
                    ImGui::Text("Geometry Shader Invocations: %d", stats.geometryShaderInvocations);
                    ImGui::Text("Geometry Shader Primitives: %d", stats.geometryShaderPrimitives);
                    ImGui::Text("Clipping Invocations: %d", stats.clippingInvocations);
                    ImGui::Text("Clipping Primitives: %d", stats.clippingPrimitives);
                    ImGui::Text("Fragment Shader Invocations: %d", stats.fragmentShaderInvocations);
                    ImGui::Text("Tessellation Control Shader Patches: %d", stats.tessellationControlShaderPatches);
                    ImGui::Text("Tessellation Evaluation Shader Invocations: %d", stats.tessellationEvaluationShaderInvocations);
                    ImGui::Text("Compute Shader Invocations: %d", stats.computeShaderInvocations);
                    ImGui::TreePop();
                }
            }

            auto resourceStats = device->resourceStats();
            ImGui::Text("Shader Count %d", resourceStats.shaderCount);
            ImGui::Text("Shader Allocated %d", resourceStats.shaderAllocated);
            ImGui::Text("Pipeline Count %d", resourceStats.pipelineCount);
            ImGui::Text("Pipeline Allocated %d", resourceStats.pipelineAllocated);
            ImGui::Text("Image Count %d", resourceStats.imageCount);
            ImGui::Text("Image Allocated %d", resourceStats.imageAllocated);
            ImGui::Text("Buffer Count %d", resourceStats.bufferCount);
            ImGui::Text("Buffer Allocated %d", resourceStats.bufferAllocated);
            ImGui::Text("Sampler Count %d", resourceStats.samplerCount);
            ImGui::Text("Sampler Allocated %d", resourceStats.shaderAllocated);
            ImGui::Text("Timestamp Query Pools %d", resourceStats.timestampQueryPools);
            ImGui::Text("PipelineStats Pools %d", resourceStats.pipelineStatsPools);

            auto renderGraphStats = renderGraph.statistics();
            ImGui::Text("Passes: %d", renderGraphStats.passes);
            ImGui::Text("Resource: %d", renderGraphStats.resources);
            ImGui::Text("Image: %d", renderGraphStats.images);
            ImGui::Text("Buffers: %d", renderGraphStats.buffers);
            ImGui::Text("Command Buffers: %d", renderGraphStats.commandBuffers);
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
        }
        ImGui::End();

        ImGui::Render();



        auto swapImage = swapchain->acquire().value();

        renderGraph.reset();

        auto imageIndex = renderGraph.addImage({
            .mipLevels = 4,
            .name = "image"
        });
        auto swapchainIndex = renderGraph.addImage({
            .handle = swapImage,
            .name = "swapchain_image"
        });

        auto particleBufferIndex = renderGraph.addBuffer({
            .handle = buffer,
            .name = "particles_buffer"
        });

        renderGraph.addClearPass("clear_image", imageIndex);

        auto particleGroup = renderGraph.getGroup("particles");

        auto& particlesMovePass = renderGraph.addPass("particles_move")
            .setGroup(particleGroup)
            .addStorageBufferWrite(particleBufferIndex, canta::PipelineStage::COMPUTE_SHADER)
            .setExecuteFunction([pipeline, particleBufferIndex, numParticles, dt](canta::CommandBuffer& cmd, canta::RenderGraph& graph) {
            auto buffer = graph.getBuffer(particleBufferIndex);
            cmd.bindPipeline(pipeline);
            struct Push {
                u64 address;
                i32 maxParticles;
                f32 dt;
            };
            cmd.pushConstants(canta::ShaderStage::COMPUTE, Push {
                    .address = buffer->address(),
                    .maxParticles = numParticles,
                    .dt = static_cast<f32>(dt)
            });
            cmd.dispatchThreads(numParticles);
        });

        auto& particlesDrawPass = renderGraph.addPass("particles_draw")
            .setGroup(particleGroup)
            .addStorageBufferRead(particleBufferIndex, canta::PipelineStage::COMPUTE_SHADER)
            .addStorageImageWrite(imageIndex, canta::PipelineStage::COMPUTE_SHADER)
            .setExecuteFunction([pipelineDraw, particleBufferIndex, imageIndex, numParticles](canta::CommandBuffer& cmd, canta::RenderGraph& graph) {
            auto buffer = graph.getBuffer(particleBufferIndex);
            auto image = graph.getImage(imageIndex);
            cmd.bindPipeline(pipelineDraw);
            struct Push {
                u64 address;
                i32 index;
                i32 maxParticles;
            };
            cmd.pushConstants(canta::ShaderStage::COMPUTE, Push{
                    .address = buffer->address(),
                    .index = image->defaultView().index(),
                    .maxParticles = numParticles
            });
            cmd.dispatchThreads(numParticles);
        });

        auto [uiSwapchainIndex] = renderGraph.addBlitPass("blit_to_swapchain", imageIndex, swapchainIndex).aliasImageOutputs<1>();

        auto& uiPass = renderGraph.addPass("ui", canta::PassType::GRAPHICS)
            .addColourRead(swapchainIndex)
            .addColourWrite(uiSwapchainIndex)
            .setExecuteFunction([&imguiContext, &swapchain](canta::CommandBuffer& cmd, canta::RenderGraph& graph) {
            imguiContext.render(ImGui::GetDrawData(), cmd, swapchain->format());
        });

        renderGraph.setBackbuffer(uiSwapchainIndex, canta::ImageLayout::PRESENT);
//        renderGraph.setBackbuffer(swapchainIndex);
        renderGraph.compile();

        auto waits = std::to_array({
            { device->frameSemaphore(), device->framePrevValue() },
            swapchain->acquireSemaphore()->getPair(),
            uploadBuffer.timeline().getPair()
        });
        auto signals = std::to_array({
            device->frameSemaphore()->getPair(),
            swapchain->presentSemaphore()->getPair()
        });
        renderGraph.execute(waits, signals);

        swapchain->present();

        dt = device->endFrame();
    }

    device->waitIdle();
    return 0;
}