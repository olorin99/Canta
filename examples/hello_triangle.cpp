#include <Canta/Device.h>
#include <Canta/SDLWindow.h>
#include <Canta/util.h>

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

    std::string vertexGLSL = R"(
#version 460

layout (location = 0) out vec3 colour;

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
    colour = triangleColours[gl_VertexIndex];
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

    auto vertexSpirv = canta::util::compileGLSLToSpirv("triangleVertex", vertexGLSL, canta::ShaderStage::VERTEX).value();
    auto fragmentSpirv = canta::util::compileGLSLToSpirv("triangleFragment", fragmentGLSL, canta::ShaderStage::FRAGMENT).value();

    auto vertexShader = device->createShaderModule({
        .spirv = vertexSpirv,
        .main = "main",
        .stage = canta::ShaderStage::VERTEX
    });

    auto fragmentShader = device->createShaderModule({
        .spirv = fragmentSpirv,
        .main = "main",
        .stage = canta::ShaderStage::FRAGMENT
    });

    auto pipelineShaders = std::to_array({
        canta::ShaderInfo{
            .module = vertexShader,
            .entryPoint = "main"
        },
        canta::ShaderInfo{
            .module = fragmentShader,
            .entryPoint = "main"
        }
    });

    auto colourFormat = swapchain->format();

    auto pipeline = device->createPipepline({
        .shaders = pipelineShaders,
        .colourFormats = { &colourFormat, 1 },
    });


    auto image = device->createImage({
        .width = 100,
        .height = 100,
        .format = canta::Format::RGBA8_UNORM
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
            .image = swapImage->image(),
            .dstStage = canta::PipelineStage::COLOUR_OUTPUT,
            .dstAccess = canta::Access::COLOUR_WRITE,
            .dstLayout = canta::ImageLayout::COLOUR_ATTACHMENT
        });

        auto attachments = std::to_array({
            canta::Attachment{
                .imageView = swapImage->defaultView().view(),
                .imageLayout = canta::ImageLayout::COLOUR_ATTACHMENT
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
        commandBuffer.draw(3);
        commandBuffer.endRendering();

        commandBuffer.barrier({
            .image = swapImage->image(),
            .srcStage = canta::PipelineStage::COLOUR_OUTPUT,
            .dstStage = canta::PipelineStage::BOTTOM,
            .srcAccess = canta::Access::COLOUR_WRITE,
            .dstAccess = canta::Access::MEMORY_WRITE | canta::Access::MEMORY_READ,
            .srcLayout = canta::ImageLayout::COLOUR_ATTACHMENT,
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