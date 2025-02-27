#include <Canta/Device.h>
#include <Canta/SDLWindow.h>
#include <Canta/util.h>
#include <Canta/PipelineManager.h>

int main() {

    canta::SDLWindow window("Hello Triangle", 1920, 1080);

    auto device = canta::Device::create({
        .applicationName = "hello_triangle",
        .instanceExtensions = window.requiredExtensions()
    }).value();

    std::printf("VK_AMD_BUFFER_MARKER is%s enabled", device->isExtensionEnabled(VK_AMD_BUFFER_MARKER_EXTENSION_NAME) ? "" : " not");

    auto swapchain = device->createSwapchain({
        .window = &window
    });
    std::array<canta::CommandPool, canta::FRAMES_IN_FLIGHT> commandPools = {};
    for (auto& pool : commandPools)
        pool = device->createCommandPool({
            .queueType = canta::QueueType::GRAPHICS
        }).value();

    std::string vertexGLSL = R"(
#version 460

#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_GOOGLE_include_directive : enable
#extension GL_EXT_buffer_reference : enable
#extension GL_EXT_buffer_reference2 : enable
#extension GL_EXT_scalar_block_layout : enable
#extension GL_EXT_shader_explicit_arithmetic_types : enable

layout (location = 0) out vec3 colour;

layout (scalar, buffer_reference, buffer_reference_align = 8) readonly buffer SomeBuffer {
    int data[];
};

layout (push_constant) uniform Push {
    SomeBuffer bufferAddress;
};

vec2 trianglePositions[3] = vec2[](
    vec2(0.5, -0.5),
    vec2(0.5, 0.5),
    vec2(-0.5, 0.5)
);

vec3 triangleColours[3] = vec3[](
    vec3(1.0, 0.0, 0.0),
    vec3(0.0, 1.0, 0.0),
    vec3(0.0, 0.0, 1.0)
);

void main() {
    gl_Position = vec4(trianglePositions[gl_VertexIndex], 0.5, 1.0);
//    colour = triangleColours[gl_VertexIndex];
    colour = triangleColours[bufferAddress.data[gl_VertexIndex]];
}
)";

    std::string fragmentGLSL = R"(
#version 460

layout (location = 0) in vec3 inColour;
layout (location = 0) out vec4 outColour;

void main() {
    outColour = vec4(inColour, 1.0);
}
)";

    auto pipelineManager = canta::PipelineManager::create({
        .device = device.get()
    });

    auto colourFormat = swapchain->format();

    auto pipeline = pipelineManager.getPipeline({
        .vertex = {
                .module = pipelineManager.getShader({
                    .glsl = vertexGLSL,
                    .stage = canta::ShaderStage::VERTEX
                })
        },
        .fragment = {
                .module = pipelineManager.getShader({
                    .glsl = fragmentGLSL,
                    .stage = canta::ShaderStage::FRAGMENT
                })
        },
        .colourFormats = std::vector{ colourFormat }
    });

    auto sampler = device->createSampler({});

    auto image = device->createImage({
        .width = 100,
        .height = 100,
        .format = canta::Format::RGBA8_UNORM
    });

    auto buffer = device->createBuffer({
        .size = sizeof(i32) * 3,
        .usage = canta::BufferUsage::STORAGE,
        .type = canta::MemoryType::STAGING,
        .persistentlyMapped = true,
        .name = "test_buffer"
    });

    buffer = device->resizeBuffer(buffer, 100);

    i32 data[] = {
            2, 1, 2
    };

    buffer->data(data);

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
            .image = swapImage,
            .dstStage = canta::PipelineStage::COLOUR_OUTPUT,
            .dstAccess = canta::Access::COLOUR_WRITE,
            .dstLayout = canta::ImageLayout::COLOUR_ATTACHMENT
        });

        auto attachments = std::to_array({
            canta::Attachment{
                .image = swapImage,
                .imageLayout = canta::ImageLayout::COLOUR_ATTACHMENT,
                .loadOp = canta::LoadOp::CLEAR,
                .storeOp = canta::StoreOp::STORE
            }
        });

        commandBuffer.beginRendering({
            .size = window.extent(),
            .colourAttachments = attachments
        });
        commandBuffer.setViewport({
            static_cast<f32>(window.extent().x()),
            static_cast<f32>(window.extent().y())
        });
        commandBuffer.bindPipeline(pipeline);
        commandBuffer.pushConstants(canta::ShaderStage::VERTEX, buffer->address());
        commandBuffer.draw(3);
        commandBuffer.endRendering();

        commandBuffer.barrier({
            .image = swapImage,
            .srcStage = canta::PipelineStage::COLOUR_OUTPUT,
            .dstStage = canta::PipelineStage::BOTTOM,
            .srcAccess = canta::Access::COLOUR_WRITE,
            .dstAccess = canta::Access::MEMORY_WRITE | canta::Access::MEMORY_READ,
            .srcLayout = canta::ImageLayout::COLOUR_ATTACHMENT,
            .dstLayout = canta::ImageLayout::PRESENT
        });

        commandBuffer.end();

        device->queue(canta::QueueType::GRAPHICS).submit({ &commandBuffer, 1 }, waits, signals);

        swapchain->present();

        device->endFrame();
    }

    device->waitIdle();
    return 0;
}