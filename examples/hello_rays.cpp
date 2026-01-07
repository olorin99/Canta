#include <Canta/Device.h>
#include <Canta/RenderGraph.h>
#include <Canta/SDLWindow.h>
#include <Ende/filesystem/File.h>

#include "Canta/PipelineManager.h"

class Camera {
public:

    Camera(const ende::math::Vec3f& origin = { 0, 0, 0 }, f32 aspectRatio = 16.0 / 9.0, f32 width = 2.0) {
        const f32 viewportHeight = width;
        const f32 viewportWidth = aspectRatio * viewportHeight;
        const f32 focalLength = 1.0;

        _origin = origin;
        _horizonal = { viewportWidth, 0, 0 };
        _vertical = { 0, viewportHeight, 0 };
        _lowerLeft = _origin - (_horizonal / 2.0) - (_vertical / 2.0) - ende::math::Vec3f{ 0, 0, focalLength };
    }

private:

    ende::math::Vec3f _origin = {};
    ende::math::Vec3f _lowerLeft = {};
    ende::math::Vec3f _horizonal = {};
    ende::math::Vec3f _vertical = {};

};


int main() {
    // auto window = canta::SDLWindow("Hello Rays", 1920, 1080);
    auto device = canta::Device::create({
        .applicationName = "hello_rays",
        .headless = true,
        .enableMeshShading = false,
        // .instanceExtensions = window.requiredExtensions(),
        .enableRenderDoc = true,
    }).value();

    const auto aspectRatio = 16.f / 9.f;
    u32 imageWidth = 400;
    u32 imageHeight = imageWidth / aspectRatio;

    // auto swapchain = device->createSwapchain({
    //     .window = &window
    // });

    device->triggerCapture();
    device->startFrameCapture();

    auto pipelineManager = canta::PipelineManager::create({
        .device = device.get(),
        .rootPath = CANTA_SRC_DIR
    });

    auto renderGraph = canta::RenderGraph::create({
        .device = device.get(),
        .multiQueue = false,
        .name = "graph"
    });

    auto camera = Camera({ 0, 0, 0 }, aspectRatio);

    auto outputBuffer = device->createBuffer({
        .size = static_cast<u32>(imageWidth * imageHeight * sizeof(ende::math::Vec4f)),
        .usage = canta::BufferUsage::TRANSFER_DST,
        .type = canta::MemoryType::READBACK,
        .persistentlyMapped = true,
        .name = "output_buffer"
    });

    std::vector<ende::math::Vec4f> spheres = {
        {0, 0, -1, 0.5},
        {0, -100.5, -1, 100},
    };

    auto sphereBuffer = renderGraph.addBuffer({
        .size = static_cast<u32>(sizeof(ende::math::Vec4f) * spheres.size()),
        .name = "sphere_buffer"
    });

    auto outputImage = renderGraph.addImage({
        .matchesBackbuffer = false,
        .width = imageWidth,
        .height = imageHeight,
        .format = canta::Format::RGBA32_SFLOAT,
        .name = "output_image"
    });

    auto outputBufferIndex = renderGraph.addBuffer({
        .handle = outputBuffer,
        .name = "output_buffer"
    });

    renderGraph.addPass({.name = "sphere_upload", .type = canta::PassType::HOST})
        .addStorageBufferWrite(sphereBuffer, canta::PipelineStage::HOST)
        .setExecuteFunction([&](auto& buffer, auto& graph) {
            graph.getBuffer(sphereBuffer)->data(spheres);
        });

    renderGraph.addPass({.name = "trace_rays"})
        .setPipeline(pipelineManager.getPipeline(canta::Pipeline::CreateInfo{
            .compute = {
                .module = pipelineManager.getShader({
                    .path = CANTA_SRC_DIR"/examples/hello_raytrace.slang",
                    .stage = canta::ShaderStage::COMPUTE,
                    .name = "hello_raytace"
                }).value(),
                .entryPoint = "traceMain"
            }
        }).value())
        .pushConstants(canta::Read(sphereBuffer), canta::Write(outputImage), static_cast<u32>(spheres.size()), camera, imageWidth, imageHeight)
        .dispatchThreads(imageWidth, imageHeight);

    renderGraph.addCopyPass("copy_to_buffer", outputImage, outputBufferIndex);

    renderGraph.setBackbuffer(outputBufferIndex);

    if (!renderGraph.compile()) return -1;
    device->updateBindlessDescriptors();
    if (!renderGraph.execute({}, {}, {}, true)) return -2;


    auto mapped = renderGraph.getBuffer(outputBufferIndex)->map();

    auto outFile = ende::fs::File::open("image.ppm", ende::fs::out).value();

    outFile.write(std::format("P3\n{} {}\n255\n", imageWidth, imageHeight));

    for (i32 y = imageHeight - 1; y >= 0; --y) {
        for (i32 x = 0; x < imageWidth; ++x) {
            auto index = y * imageWidth + x;
            auto colour = mapped.as<ende::math::Vec4f>()[index];

            auto r = colour.x();
            auto g = colour.y();
            auto b = colour.z();

            outFile.write(std::format("{} {} {}\n",
              static_cast<i32>(256 * std::clamp(r, 0.f, 0.999f)),
              static_cast<i32>(256 * std::clamp(g, 0.f, 0.999f)),
              static_cast<i32>(256 * std::clamp(b, 0.f, 0.999f))));
        }
    }
    outFile.close();

    device->endFrameCapture();

    return 0;
}
