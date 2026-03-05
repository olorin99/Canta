
#include <random>
#include <Canta/Device.h>
#include <Canta/RenderGraph.h>
#include <Canta/PipelineManager.h>

#include "embedded_shaders_Matrix.h"

void genMatrix(u32 N, f32* data) {
    std::random_device rd;
    std::mt19937 rng(rd());
    std::uniform_real_distribution<f32> dis(0, 1000);


    for (int x = 0; x < N; x++) {
        for (int y = 0; y < N; y++) {
            data[x * N + y] = dis(rng);
        }
    }
}

void printMatrix(u32 N, f32* data) {
    for (int x = 0; x < N; x++) {
        for (int y = 0; y < N; y++) {
            printf("%f, ", data[x * N + y]);
        }
        printf("\n");
    }
}

void multiplyMatrix(const u32 N, const f32* lhs, const f32* rhs, f32* output) {
    for (int x = 0; x < N; x++) {
        for (int y = 0; y < N; y++) {
            f32 tmp = 0.0;
            for (int i = 0; i < N; i++) {
                tmp += lhs[y * N + i] * rhs[x + N * i];
            }
            output[y * N + x] = tmp;
        }
    }
}

int main() {

    auto device = TRY_MAIN(canta::Device::create({
        .applicationName = "matrix multiplication example",
        .headless = true,
        .enableMeshShading = false,
        .enableRenderDoc = true,
    }));

    device->triggerCapture();
    device->startFrameCapture();

    auto pipelineManager = canta::PipelineManager::create({
        .device = device.get(),
        .rootPath = CANTA_SRC_DIR"/examples"
    });

    // auto renderGraph = TRY_MAIN(canta::RenderGraph::create({
    //     .device = device.get(),
    //     .multiQueue = false,
    //     .name = "graph"
    // }));


    constexpr int N = 64;

    f32 lhsData[N * N] = {};
    genMatrix(N, lhsData);
    f32 rhsData[N * N] = {};
    genMatrix(N, rhsData);

    const auto computePipeline = pipelineManager.getPipeline({
        .compute = {
            .module = pipelineManager.getShader({
                .slang = R"(
import canta;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(
    uint3 threadId: SV_DispatchThreadID,
    uniform uint* buffer,
    uniform canta.Image2D<float4> image,
) {
    buffer[0] = 0;
}
)",
                .stage = canta::ShaderStage::COMPUTE,
                .name = "compute_0",
            }).value()
        }
    }).value();

    const auto computePipeline1 = pipelineManager.getPipeline({
    .compute = {
        .module = pipelineManager.getShader({
            .slang = R"(
import canta;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(
    uint3 threadId: SV_DispatchThreadID,
    uniform uint* buffer,
) {
    buffer[0] = 0;
}
)",
            .stage = canta::ShaderStage::COMPUTE,
            .name = "compute_0",
        }).value()
    }
}).value();

    const auto graphicsPipeline = pipelineManager.getPipeline({
.vertex = {
    .module = pipelineManager.getShader({
        .slang = R"(
import canta;

[shader("vertex")]
void main(
    uint3 threadId: SV_DispatchThreadID,
    uniform canta.Image2D<float4> image,
) {
    float4 value = image[uint2(0, 1)];
}
)",
        .stage = canta::ShaderStage::VERTEX,
        .name = "vertex_0",
    }).value()
},
        .fragment = {
            .module = pipelineManager.getShader({
        .slang = R"(
import canta;

[shader("fragment")]
float4 main(
    uint3 threadId: SV_DispatchThreadID,
    uniform canta.Image2D<float4> image,
) {
    return float4(1, 1, 1, 1);
}
)",
        .stage = canta::ShaderStage::FRAGMENT,
        .name = "fragment_0",
    }).value()
        },
        .colourFormats = {
            canta::Format::RGBA8_UNORM
        }
}).value();

    auto graph = TRY_MAIN(canta::RenderGraph::create({
        .device = device.get(),
    }));

    const auto buffer = graph.addBuffer({ .size = 1000, .name = "buffer_0" });
    const auto hostBuffer = graph.addBuffer({ .size = 1000, .name = "buffer_1" });
    const auto image = graph.addImage({ .width = 512, .height = 512, .name = "image_0" });
    const auto swapImage = graph.addImage({ .width = 512, .height = 512, .name = "swap_image" });

    auto computePass = graph.compute("pass_0", computePipeline)
        .addStorageBufferWrite(buffer)
        .addStorageImageWrite(image)
        .pushConstants(buffer, image)
        .dispatchThreads(1, 1, 1);

    const auto outputBuffer = TRY_MAIN(computePass.output<canta::BufferIndex>());
    const auto outputImage = TRY_MAIN(computePass.output<canta::ImageIndex>(1));


    auto geometryPass = [&] (canta::RenderGraph& g, int index) -> std::expected<canta::BufferIndex, canta::RenderGraphError> {
        auto p1 = g.compute(std::format("compute_pass_1_{}", index), computePipeline1)
            .addStorageBufferRead(outputBuffer)
            .addStorageBufferWrite(outputBuffer)
            .pushConstants(outputBuffer)
            .dispatchThreads(1, 1, 1);

        auto p2 = g.compute(std::format("compute_pass_2_{}", index), computePipeline1)
            .addStorageBufferRead(TRY(p1.output<canta::BufferIndex>()))
            .addStorageBufferWrite(outputBuffer)
            .pushConstants(outputBuffer)
            .dispatchThreads(1, 1, 1);

        auto p3 = g.compute(std::format("compute_pass_3_{}", index), computePipeline1)
            .addStorageBufferRead(TRY(p2.output<canta::BufferIndex>()))
            .addStorageBufferWrite(outputBuffer)
            .pushConstants(outputBuffer)
            .dispatchThreads(1, 1, 1);

        return p3.output<canta::BufferIndex>();
    };

    const auto b = TRY_MAIN(geometryPass(graph, 0));
    const auto c = TRY_MAIN(geometryPass(graph, 1));

    auto computePass2 = graph.compute("pass_1", computePipeline);
    computePass2.addStorageBufferRead(b);
    computePass2.addStorageBufferRead(c);
    computePass2.addStorageImageRead(outputImage);
    computePass2.addStorageImageWrite(outputImage);
    computePass2.pushConstants(b, outputImage);
    computePass2.dispatchThreads();

    auto graphicsPass = graph.graphics("pass_2", graphicsPipeline)
        .addSampledRead(TRY_MAIN(computePass2.output<canta::ImageIndex>()), canta::PipelineStage::FRAGMENT_SHADER)
        .addColourWrite(swapImage)
        .pushConstants(outputImage)
        .draw(10);

    auto transferPass = graph.graphics("pass_4").blit(TRY_MAIN(graphicsPass.output<canta::ImageIndex>()), outputImage);


    const auto l = TRY_MAIN(transferPass.output<canta::ImageIndex>());
    auto hostPass = graph.host<canta::ImageIndex, canta::BufferIndex, u32, std::string>("pass_3")
        .pushConstants(canta::Read(l), canta::Write(hostBuffer), 100, "hello")
        .setCallback([] (canta::RenderGraph&, const canta::ImageIndex& image, const canta::BufferIndex& buffer, const u32& argA, const std::string& argB) {
            printf("Host pass\n");
            printf("Image index: %d\n", image.index);
            printf("Buffer index: %d\n", buffer.index);
            printf("Arg a: %d\n", argA);
            printf("Arg b: %s\n", argB.c_str());
        });

    const auto rootEdge = TRY_MAIN(hostPass.output<canta::BufferIndex>());

    printf("Pass count: %d\n", graph.vertexCount());
    printf("Edge count: %d\n", graph.edgeCount());

    graph.setRoot(rootEdge);
    TRY_MAIN(graph.compile());
    // TRY_MAIN(graph.compile());

    TRY_MAIN(graph.run());

    // auto basePass = computePass.clone();




    // const auto lhs = renderGraph.addBuffer({
    //     .size = N * N * sizeof(f32),
    //     .name = "lhs_matrix"
    // });
    //
    // const auto rhs = renderGraph.addBuffer({
    //     .size = N * N * sizeof(f32),
    //     .name = "rhs_matrix"
    // });
    //
    // const auto outputBuffer = device->createBuffer({
    //     .size = N * N * sizeof(f32),
    //     .usage = canta::BufferUsage::STORAGE,
    //     .type = canta::MemoryType::READBACK,
    //     .persistentlyMapped = true,
    //     .name = "output"
    // });
    // const auto output = renderGraph.addBuffer({
    //     .handle = outputBuffer,
    //     .name = "output"
    // });
    //
    // renderGraph.addUploadPass("lhs_upload", lhs, lhsData);
    // renderGraph.addUploadPass("rhs_upload", rhs, rhsData);
    //
    // const auto outputAlias = TRY_MAIN(matrix::matrix_multiply(N, N)(pipelineManager)(renderGraph, canta::Read(lhs), canta::Read(rhs), canta::Write(output), N).aliasBufferOutput(0));
    //
    //
    // canta::BufferIndex lhsIndex = rhs;
    // canta::BufferIndex rhsIndex = outputAlias;
    // canta::BufferIndex outputIndex = lhs;
    // for (i32 i = 0; i < 60; i++ ) {
    //     outputIndex = TRY_MAIN(matrix::matrix_multiply(N, N)(pipelineManager)(renderGraph, canta::Read(lhsIndex), canta::Read(rhsIndex), canta::Write(outputIndex), N).aliasBufferOutput(0));
    //     lhsIndex = rhsIndex;
    //     rhsIndex = outputIndex;
    // }
    //
    // f32 outputData[N * N] = {};
    // const auto backbuffer = TRY_MAIN(renderGraph.addReadbackPass("output_read", outputIndex, outputData).aliasBufferOutput(0));
    //
    // renderGraph.setBackbuffer(backbuffer);
    //
    // if (!renderGraph.compile()) return -1;
    // if (!renderGraph.execute({}, {}, {}, true)) return -3;

    // printf("RenderGraph\n");
    // printf("Passes: %lu\n", renderGraph.orderedPasses().size());

    // printf("Validate output\n");

    // f32 outputData2[N * N] = {};
    // multiplyMatrix(N, lhsData, rhsData, outputData2);
    // printMatrix(N, outputData2);

    // f32 avgErr = 0.0;
    //
    // for (u32 i = 0; i < N * N; i++) {
    //     f32 diff = std::abs(outputData[i] - outputData2[i]);
    //     if (diff > 2.00000001) {
    //         printf("Error at index (%d) with diff of %f. LHS: %f vs RHS: %f\n", i, diff, outputData[i], outputData2[i]);
    //     }
    //     avgErr += diff;
    // }
    //
    // avgErr /= (N * N);
    // printf("Average error: %f\n", avgErr);

    // u64 totalTime = 0;
    // for (auto timers = renderGraph.timers(); auto&[name, timer] : timers) {
    //     const auto time = TRY_MAIN(timer.result());
    //     printf("Pass: %s (%fms)\n", name.c_str(), time / 1e6);
    //     totalTime += time;
    // }
    // printf("Total time: %fms\n", totalTime / 1e6);


    device->endFrameCapture();

    return 0;
}