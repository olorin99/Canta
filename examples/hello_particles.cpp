#include <Canta/Device.h>
#include <Canta/SDLWindow.h>
#include <Canta/util.h>
#include <Ende/math/Vec.h>
#include <random>
#include <Canta/PipelineManager.h>
#include <Canta/RenderGraph.h>

int main() {

    canta::SDLWindow window("Hello Triangle", 1920, 1080);

    auto device = canta::Device::create({
        .applicationName = "hello_triangle",
        .instanceExtensions = window.requiredExtensions()
    }).value();

    auto swapchain = device->createSwapchain({
        .window = &window
    });
    std::array<canta::CommandPool, canta::FRAMES_IN_FLIGHT> commandPools = {};
    for (auto& pool : commandPools) {
        pool = device->createCommandPool({
            .queueType = canta::QueueType::GRAPHICS
        }).value();
    }


    std::string particleDrawComp = R"(
#version 460

#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_GOOGLE_include_directive : enable
#extension GL_EXT_buffer_reference : enable
#extension GL_EXT_buffer_reference2 : enable
#extension GL_EXT_scalar_block_layout : enable
#extension GL_EXT_shader_explicit_arithmetic_types : enable

#include "canta.glsl"

layout (local_size_x = 32) in;

//layout (set = 0, binding = 2) uniform writeonly image2D storageImages[];

CANTA_USE_STORAGE_IMAGE(image2D, writeonly, storageImages);

struct Particle {
    vec2 position;
    vec2 velocity;
    vec3 colour;
    int radius;
};

layout (scalar, buffer_reference, buffer_reference_align = 8) readonly buffer ParticleBuffer {
    Particle particles[];
};

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

    auto pipeline = pipelineManager.getPipeline({
        .compute = {
            .module = pipelineManager.getShader({
                .path = "/home/christian/Documents/Projects/Canta/examples/particles.comp",
                .stage = canta::ShaderStage::COMPUTE
            })
        }
    });

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

    buffer->data(particles);

    canta::Timer timers[canta::FRAMES_IN_FLIGHT] = {};
    for (auto& timer : timers)
        timer = device->createTimer();


    auto renderGraph = canta::RenderGraph::create({
        .device = device.get()
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
        device->beginFrame();
        device->gc();
        pipelineManager.reloadAll();

        auto swapImage = swapchain->acquire().value();

        auto waits = std::to_array({
            { device->frameSemaphore(), device->framePrevValue() },
            swapchain->acquireSemaphore()->getPair()
        });
        auto signals = std::to_array({
            device->frameSemaphore()->getPair(),
            swapchain->presentSemaphore()->getPair()
        });

        auto flyingIndex = device->flyingIndex();

        commandPools[flyingIndex].reset();
        auto& commandBuffer = commandPools[flyingIndex].getBuffer();

        commandBuffer.begin();
        std::printf("%lu\n", timers[(flyingIndex - 1) % canta::FRAMES_IN_FLIGHT].result().value());
        timers[flyingIndex].begin(commandBuffer);

        renderGraph.reset();

        auto imageIndex = renderGraph.addImage({
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

        auto& particlesMovePass = renderGraph.addPass("particles_move");
        particlesMovePass.addStorageBufferWrite(particleBufferIndex, canta::PipelineStage::COMPUTE_SHADER);
        particlesMovePass.setExecuteFunction([pipeline, particleBufferIndex, numParticles](canta::CommandBuffer& cmd, canta::RenderGraph& graph) {
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
                    .dt = 1.f / 60
            });
            cmd.dispatchThreads(numParticles);
        });

        auto& particlesDrawPass = renderGraph.addPass("particles_draw");
        particlesDrawPass.addStorageBufferRead(particleBufferIndex, canta::PipelineStage::COMPUTE_SHADER);
        particlesDrawPass.addStorageImageWrite(imageIndex, canta::PipelineStage::COMPUTE_SHADER);
        particlesDrawPass.setExecuteFunction([pipelineDraw, particleBufferIndex, imageIndex, numParticles](canta::CommandBuffer& cmd, canta::RenderGraph& graph) {
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
                    .index = image.index(),
                    .maxParticles = numParticles
            });
            cmd.dispatchThreads(numParticles);
        });

        renderGraph.addBlitPass("blit_to_swapchain", imageIndex, swapchainIndex);

        renderGraph.setBackbuffer(swapchainIndex);
        renderGraph.compile();
        renderGraph.execute(commandBuffer);

        commandBuffer.barrier({
            .image = swapImage,
            .srcStage = canta::PipelineStage::TRANSFER,
            .dstStage = canta::PipelineStage::BOTTOM,
            .srcAccess = canta::Access::TRANSFER_WRITE | canta::Access::TRANSFER_READ,
            .dstAccess = canta::Access::MEMORY_WRITE | canta::Access::MEMORY_READ,
            .srcLayout = canta::ImageLayout::TRANSFER_DST,
            .dstLayout = canta::ImageLayout::PRESENT
        });

        timers[flyingIndex].end(commandBuffer);
        commandBuffer.end();

        commandBuffer.submit(waits, signals);

        swapchain->present();

        device->endFrame();
    }

    device->waitIdle();
    return 0;
}