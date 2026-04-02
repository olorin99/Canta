
#include "Canta/PipelineManager.h"
#include "Canta/UploadBuffer.h"
#include <Canta/Enums.h>
#include <Canta/Device.h>
#include <array>
#include <cstring>


int main() {

    auto device = maybe_conv(i32, canta::Device::create({
        .applicationName = "upload",
        .headless = true,
        .enableMeshShading = false,
        .enableAsyncComputeQueue = false,
        .frameBasedResourceLifetime = false,
        .resourceDestructionDelay = 0,
        .enableRenderDoc = true,
    }));

    device->triggerCapture();
    device->startFrameCapture();


    const auto uploadSize = 1 << 18;
    auto uploadBuffer = maybe_conv(i32, canta::UploadBuffer::create({ .device = device.get(), .size = uploadSize }));

    const auto bufferSize = 1 << 20;
    auto dstBuffer = device->createBuffer({ .size = bufferSize, .usage = canta::BufferUsage::TRANSFER_DST, .type = canta::MemoryType::STAGING, .persistentlyMapped = true, .name = "dst_buffer" });

    std::vector<u32> bufferData = {};
    bufferData.reserve(bufferSize / sizeof(u32));
    for (auto i = 0; i < bufferSize / sizeof(u32); i++) {
        bufferData.emplace_back(i);
    }

    uploadBuffer.upload(dstBuffer, bufferData);

    uploadBuffer.flushStagedData();
    uploadBuffer.wait();


    const auto& address = dstBuffer->mapped();
    const auto matched = 0 == std::memcmp(bufferData.data(), address.address(), bufferSize);
    
    printf("gpu and cpu data %s.\n", matched ? "match" : "doesn't match");


    const auto imageWidth = 1 << 10;
    const auto imageHeight = imageWidth;
    auto dstImage = device->createImage({
        .width = imageWidth,
        .height = imageHeight,
        .format = canta::Format::RG32_UINT,
        .usage = canta::ImageUsage::STORAGE | canta::ImageUsage::TRANSFER_DST
    });

    std::vector<u32> imageData = {};
    imageData.reserve(imageWidth * imageHeight * sizeof(u32) * 2);
    for (u32 y = 0; y < imageHeight; y++) {
        for (u32 x = 0; x < imageWidth; x++) {
            imageData.emplace_back(x);
            imageData.emplace_back(y);
        }
    }

    uploadBuffer.upload(dstImage, imageData, {
        .width = imageWidth,
        .height = imageHeight,
        .format = canta::Format::RG32_UINT,
    });

    uploadBuffer.flushStagedData();
    uploadBuffer.wait();

    device->waitIdle();

    auto pipelineManager = canta::PipelineManager::create({
        .device = device.get(),
    });

    auto readbackBuffer = device->createBuffer({
        .size = sizeof(u32) * 2,
        .type = canta::MemoryType::READBACK,
        .persistentlyMapped = true,
        .name = "readback_buffer",
    });

    device->updateBindlessDescriptors();

    auto pool = device->createCommandPool({ .queueType = canta::QueueType::COMPUTE, .name = "validate_pool" });

    auto commands = pool->getBuffer();

    commands->begin();

    commands->barrier({
        .buffer = dstBuffer,
        .dstStage = canta::PipelineStage::COMPUTE_SHADER,
        .dstAccess = canta::Access::SHADER_READ,
    });

    const auto validate_buffer_shader = R"(
[shader("compute")]
[numthreads(32, 1, 1)]
void main(
    uint3 threadId : SV_DispatchThreadID,
    uniform uint* buffer,
    uniform uint* readback,
    uniform uint size,
    uniform uint padding,
) {
    if (threadId.x >= size) return;
    
    readback[0] = threadId.x == buffer[threadId.x];
    // if (threadId.x != buffer[threadId.x]) {
    //     readback[0] = threadId.x;
    // }
}
)";

    commands->bindPipeline(maybe_conv(i32, pipelineManager.getPipeline({
        .compute = { .slang = validate_buffer_shader }
    })));

    struct Push {
        u64 buffer;
        u64 readback;
        u32 size;
    };

    commands->pushConstants(canta::ShaderStage::COMPUTE, Push{ dstBuffer->address(), readbackBuffer->address(), bufferSize });
    commands->dispatchThreads(bufferSize / sizeof(u32));

    commands->barrier({
        .image = dstImage,
        .dstStage = canta::PipelineStage::COMPUTE_SHADER,
        .dstAccess = canta::Access::SHADER_READ,
        .srcLayout = canta::ImageLayout::SHADER_READ_ONLY,
        .dstLayout = canta::ImageLayout::GENERAL,
    });

        const auto validate_image_shader = R"(
import canta;

[shader("compute")]
[numthreads(32, 32, 1)]
void main(
    uint3 threadId : SV_DispatchThreadID,
    uniform uint* readback,
    uniform canta.RWImage2D<uint2> image,
    uniform uint size,
) {
    if (any(threadId.xy >= image.size()))
        return;
    
    readback[1] = all(image[threadId.xy] == threadId.xy);
}
)";

    commands->bindPipeline(maybe_conv(i32, pipelineManager.getPipeline({
        .compute = { .slang = validate_image_shader }
    })));

    struct Push1 {
        u64 readback;
        i32 image;
        u32 size;
    };

    commands->pushConstants(canta::ShaderStage::COMPUTE, Push1{  readbackBuffer->address(), dstImage->defaultView().index(), bufferSize });
    commands->dispatchThreads(imageWidth, imageHeight);


    commands->end();


    auto waits = std::to_array({
        canta::SemaphorePair(uploadBuffer.timeline())
    });
    auto signals = std::to_array({
        canta::SemaphorePair(pool->queue()->timeline(), pool->queue()->timeline()->increment()),
    });

    pool->queue()->submit({ &commands, 1 }, waits, signals);

    pool->queue()->timeline()->wait(pool->queue()->timeline()->value());

    device->endFrameCapture();

    device->waitIdle();

    
    const auto& readbackAddress = readbackBuffer->mapped();

    auto bufferMatch = readbackAddress.as<u32>()[0] != 0;
    auto imageMatch = readbackAddress.as<u32>()[1] != 0;

    printf("gpu and cpu buffer data %s.\n", bufferMatch ? "match" : "doesn't match");
    printf("gpu and cpu image data %s.\n", imageMatch ? "match" : "doesn't match");
    
    return 0;
}