#include <Canta/Device.h>
#include <Canta/SDLWindow.h>
#include <Canta/util.h>
#include <Ende/math/Vec.h>
#include <random>

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
    for (auto& pool : commandPools)
        pool = device->createCommandPool({
                                                 .queueType = canta::QueueType::GRAPHICS
                                         }).value();

    std::string particleMoveComp = R"(
#version 460

#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_GOOGLE_include_directive : enable
#extension GL_EXT_buffer_reference : enable
#extension GL_EXT_buffer_reference2 : enable
#extension GL_EXT_scalar_block_layout : enable
#extension GL_EXT_shader_explicit_arithmetic_types : enable

layout (local_size_x = 32, local_size_y = 32) in;

struct Particle {
    vec2 position;
    vec2 velocity;
    int radius;
};

layout (scalar, buffer_reference, buffer_reference_align = 8) readonly buffer ParticleBuffer {
    Particle particles[];
};

layout (push_constant) uniform Push {
    ParticleBuffer particleBuffer;
};

void main() {
    const uint idx = gl_GlobalInvocationID.x + gl_GlobalInvocationID.y * gl_NumWorkGroups.x * gl_WorkGroupSize.x;
    if (idx >= 100)
        return;
    Particle particle = particleBuffer.particles[idx];

    vec2 newPosition = particle.position + particle.velocity;
    if (newPosition.x > 1920) {
        vec2 newVelocity = reflect(particle.velocity, vec2(-1, 0));
        particle.velocity = newVelocity;
    } else if (newPosition.x < 0) {
        vec2 newVelocity = reflect(particle.velocity, vec2(1, 0));
        particle.velocity = newVelocity;
    } else if (newPosition.y > 1080) {
        vec2 newVelocity = reflect(particle.velocity, vec2(0, -1));
        particle.velocity = newVelocity;
    } else if (newPosition.y < 0) {
        vec2 newVelocity = reflect(particle.velocity, vec2(0, 1));
        particle.velocity = newVelocity;
    }

    particle.position = newPosition;
    particleBuffer.particles[idx] = particle;
}
)";

    std::string particleDrawComp = R"(
#version 460

#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_GOOGLE_include_directive : enable
#extension GL_EXT_buffer_reference : enable
#extension GL_EXT_buffer_reference2 : enable
#extension GL_EXT_scalar_block_layout : enable
#extension GL_EXT_shader_explicit_arithmetic_types : enable

layout (local_size_x = 32, local_size_y = 32) in;

layout (set = 0, binding = 2) uniform writeonly image2D storageImages[];

struct Particle {
    vec2 position;
    vec2 velocity;
    int radius;
};

layout (scalar, buffer_reference, buffer_reference_align = 8) readonly buffer ParticleBuffer {
    Particle particles[];
};

layout (push_constant) uniform Push {
    ParticleBuffer particleBuffer;
    int imageIndex;
    int padding;
};

void main() {
    const uint idx = gl_GlobalInvocationID.x + gl_GlobalInvocationID.y * gl_NumWorkGroups.x * gl_WorkGroupSize.x;
    if (idx >= 100)
        return;

    Particle particle = particleBuffer.particles[idx];

    vec4 colour = vec4(1, 0, 0, 1);

    for (int x = -particle.radius; x < particle.radius; x++) {
        for (int y = -particle.radius; y < particle.radius; y++) {
            ivec2 position = ivec2(particle.position) + ivec2(x, y);
            imageStore(storageImages[imageIndex], position, colour);
        }
    }

    imageStore(storageImages[imageIndex], ivec2(particle.position), colour);
}
)";

    auto particleSpirv = canta::util::compileGLSLToSpirv("particleMove", particleMoveComp, canta::ShaderStage::COMPUTE).transform_error([](const auto& error) {
        std::printf("%s", error.c_str());
        return error;
    }).value();
    auto particleDrawSpirv = canta::util::compileGLSLToSpirv("particleDraw", particleDrawComp, canta::ShaderStage::COMPUTE).transform_error([](const auto& error) {
        std::printf("%s", error.c_str());
        return error;
    }).value();

    auto particleShader = device->createShaderModule({
        .spirv = particleSpirv,
        .stage = canta::ShaderStage::COMPUTE
    });

    auto particleDrawShader = device->createShaderModule({
        .spirv = particleDrawSpirv,
        .stage = canta::ShaderStage::COMPUTE
    });

    auto pipelineShaders = std::to_array({
        canta::ShaderInfo{
            .module = particleShader,
            .entryPoint = "main"
        }
    });

    auto pipeline = device->createPipeline({
        .shaders = pipelineShaders,
        .mode = canta::PipelineMode::COMPUTE
    });

    auto pipelineDrawShaders = std::to_array({
        canta::ShaderInfo{
            .module = particleDrawShader,
            .entryPoint = "main"
        }
    });

    auto pipelineDraw = device->createPipeline({
        .shaders = pipelineDrawShaders,
        .mode = canta::PipelineMode::COMPUTE
    });

    constexpr const u32 numParticles = 100;

    struct Particle {
        ende::math::Vec<2, f32> position;
        ende::math::Vec<2, f32> velocity;
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

    std::array<Particle, numParticles> particles = {};
    for (u32 i = 0; i < numParticles; i++) {
        particles[i] = {
                .position = { xDist(re), yDist(re) },
                .velocity = { velDist(re), velDist(re) },
                .radius = static_cast<i32>(velDist(re))
        };
    }

    buffer->data(particles);

    auto image = device->createImage({
        .width = window.extent().x(),
        .height = window.extent().y(),
        .usage = canta::ImageUsage::STORAGE | canta::ImageUsage::TRANSFER_SRC | canta::ImageUsage::TRANSFER_DST
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

        commandBuffer.barrier({
            .image = image->image(),
            .dstStage = canta::PipelineStage::TRANSFER,
            .dstAccess = canta::Access::TRANSFER_READ | canta::Access::TRANSFER_WRITE,
            .dstLayout = canta::ImageLayout::GENERAL
        });
        commandBuffer.clearImage(image);
        commandBuffer.barrier({
            .image = image->image(),
            .srcStage = canta::PipelineStage::TRANSFER,
            .dstStage = canta::PipelineStage::COMPUTE_SHADER,
            .srcAccess = canta::Access::TRANSFER_WRITE | canta::Access::TRANSFER_READ,
            .dstAccess = canta::Access::SHADER_WRITE | canta::Access::SHADER_READ,
            .srcLayout = canta::ImageLayout::GENERAL,
            .dstLayout = canta::ImageLayout::GENERAL
        });

        commandBuffer.bindPipeline(pipeline);
        commandBuffer.pushConstants(canta::ShaderStage::COMPUTE, buffer->address());
        commandBuffer.dispatchWorkgroups();

        commandBuffer.bindPipeline(pipelineDraw);
        struct Push {
            u64 address;
            i32 index;
            i32 padding;
        };
        commandBuffer.pushConstants(canta::ShaderStage::COMPUTE, Push{
            .address = buffer->address(),
            .index = image.index()
        });
        commandBuffer.dispatchWorkgroups();

        commandBuffer.barrier({
            .image = image->image(),
            .srcStage = canta::PipelineStage::COMPUTE_SHADER,
            .dstStage = canta::PipelineStage::TRANSFER,
            .srcAccess = canta::Access::SHADER_WRITE | canta::Access::SHADER_READ,
            .dstAccess = canta::Access::TRANSFER_WRITE | canta::Access::TRANSFER_READ,
            .srcLayout = canta::ImageLayout::GENERAL,
            .dstLayout = canta::ImageLayout::TRANSFER_SRC
        });
        commandBuffer.barrier({
            .image = swapImage->image(),
            .dstStage = canta::PipelineStage::TRANSFER,
            .dstAccess = canta::Access::TRANSFER_WRITE | canta::Access::TRANSFER_READ,
            .dstLayout = canta::ImageLayout::TRANSFER_DST
        });

        commandBuffer.blit({
            .src = image,
            .dst = swapImage,
            .srcLayout = canta::ImageLayout::TRANSFER_SRC,
            .dstLayout = canta::ImageLayout::TRANSFER_DST
        });

        commandBuffer.barrier({
            .image = swapImage->image(),
            .srcStage = canta::PipelineStage::TRANSFER,
            .dstStage = canta::PipelineStage::BOTTOM,
            .srcAccess = canta::Access::TRANSFER_WRITE | canta::Access::TRANSFER_READ,
            .dstAccess = canta::Access::MEMORY_WRITE | canta::Access::MEMORY_READ,
            .srcLayout = canta::ImageLayout::TRANSFER_DST,
            .dstLayout = canta::ImageLayout::PRESENT
        });

        commandBuffer.end();

        commandBuffer.submit(waits, signals);

        swapchain->present();

        device->endFrame();
    }

    vkDeviceWaitIdle(device->logicalDevice());
    return 0;
}