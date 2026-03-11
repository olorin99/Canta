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
    auto device = TRY_MAIN(canta::Device::create({
        .applicationName = "hello_rays",
        .headless = true,
        .enableMeshShading = false,
        .enableRenderDoc = true,
    }));

    const auto aspectRatio = 16.f / 9.f;
    u32 imageWidth = 400;
    u32 imageHeight = imageWidth / aspectRatio;

    device->triggerCapture();
    device->startFrameCapture();

    auto pipelineManager = canta::PipelineManager::create({
        .device = device.get(),
        .rootPath = CANTA_SRC_DIR
    });

    auto renderGraph = TRY_MAIN(canta::RenderGraph::create({
        .device = device.get(),
        .multiQueue = false,
        // .name = "graph"
    }));

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
        .width = imageWidth,
        .height = imageHeight,
        .format = canta::Format::RGBA32_SFLOAT,
        .name = "output_image"
    });

    auto outputBufferIndex = renderGraph.addBuffer({
        .buffer = outputBuffer,
        .name = "output_buffer"
    });

    auto uploadedData = TRY_MAIN(renderGraph.host("sphere_upload").upload(sphereBuffer, spheres));

    auto image = TRY_MAIN(renderGraph.compute("trace_rays", pipelineManager.getPipeline(canta::Pipeline::CreateInfo{
            .compute = {
                .path = CANTA_SRC_DIR"/examples/hello_raytrace.slang",
                .entryPoint = "traceMain"
            }
        }).value())
        .pushConstants(canta::Read(uploadedData), canta::Write(outputImage), static_cast<u32>(spheres.size()), camera, imageWidth, imageHeight)
        .dispatchThreads(imageWidth, imageHeight).output<canta::ImageIndex>());

    auto output = TRY_MAIN(renderGraph.transfer("copy_to_buffer").copy(image, outputBufferIndex, {}));

    renderGraph.setRoot(output);

    TRY_MAIN(renderGraph.compile());
    TRY_MAIN(renderGraph.run({}, {}, false));

    auto mapped = canta::Ptr<ende::math::Vec4f>(*renderGraph.getBuffer(outputBufferIndex));

    auto outFile = ende::fs::File::open("image.ppm", ende::fs::out).value();

    outFile.write(std::format("P3\n{} {}\n255\n", imageWidth, imageHeight));

    for (i32 y = imageHeight - 1; y >= 0; --y) {
        for (i32 x = 0; x < imageWidth; ++x) {
            auto index = y * imageWidth + x;
            auto colour = mapped[index];

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
