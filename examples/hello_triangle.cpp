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

    std::string vertexSlang = R"(
struct VSOutput {
    float4 position : SV_Position;
    float3 colour : COLOUR;
};

static const float2 trianglePositions[3] = float2[](
    float2(0.5, -0.5),
    float2(0.5, 0.5),
    float2(-0.5, 0.5)
);

static const float3 triangleColours[3] = float3[](
    float3(1.0, 0.0, 0.0),
    float3(0.0, 1.0, 0.0),
    float3(0.0, 0.0, 1.0)
);

[shader("vertex")]
VSOutput main(
    int vertexIndex : SV_VertexID,
    uniform int* colour
) {
    VSOutput out;
    out.position = float4(trianglePositions[vertexIndex], 0, 1.0);
    out.colour = triangleColours[colour[vertexIndex]];
    return out;
}
)";

    std::string fragmentSlang = R"(
struct VSOutput {
    float4 position : SV_Position;
    float3 colour : COLOUR;
};

[shader("fragment")]
float4 main(
    in VSOutput in,
) {
    return float4(in.colour, 1.0);
}
)";

    auto pipelineManager = canta::PipelineManager::create({
        .device = device.get()
    });

    auto colourFormat = swapchain->format();

    auto pipeline = pipelineManager.getPipeline({
        .vertex = {
            .module = pipelineManager.getShader({
                .slang = vertexSlang,
                .stage = canta::ShaderStage::VERTEX,
                .name = "vertex_shader"
            }).value()
        },
        .fragment = {
            .module = pipelineManager.getShader({
                .slang = fragmentSlang,
                .stage = canta::ShaderStage::FRAGMENT,
                .name = "fragment_shader"
            }).value()
        },
        .colourFormats = std::vector{ colourFormat }
    }).value();

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
            canta::SemaphorePair{swapchain->acquireSemaphore()}
        });
        auto signals = std::to_array({
            canta::SemaphorePair{device->frameSemaphore()},
            canta::SemaphorePair{swapchain->presentSemaphore()}
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

        device->queue(canta::QueueType::GRAPHICS)->submit({ &commandBuffer, 1 }, waits, signals);

        swapchain->present();

        device->endFrame();
    }

    device->waitIdle();
    return 0;
}