
#include <random>
#include <Canta/Device.h>
// #include <Canta/RenderGraph.h>
#include <Canta/RenderGraphV2.h>
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

    auto graph = canta::V2::RenderGraph();

    const auto buffer = graph.addBuffer();
    const auto hostBuffer = graph.addBuffer();
    const auto image = graph.addImage();
    const auto swapImage = graph.addImage();

    auto computePass = graph.compute("pass_0")
        .addStorageBufferWrite(buffer)
        .addStorageImageWrite(image)
        .pushConstants(buffer, image)
        .dispatchThreads(64, 64, 1);

    const auto outputBuffer = TRY_MAIN(computePass.output<canta::V2::BufferIndex>());
    const auto outputImage = TRY_MAIN(computePass.output<canta::V2::ImageIndex>(1));


    auto computePass2 = graph.compute("pass_1");
    computePass2.addStorageBufferRead(outputBuffer);
    computePass2.addStorageImageRead(outputImage);
    computePass2.addStorageImageWrite(outputImage);

    auto graphicsPass = graph.graphics("pass_2")
        .addSampledRead(outputImage)
        .addColourWrite(swapImage)
        .pushConstants(outputImage)
        .draw(10);

    auto hostPass = graph.host("pass_3")
        .read(TRY_MAIN(graphicsPass.output<canta::V2::ImageIndex>()))
        .write(hostBuffer)
        .setCallback([] (canta::V2::RenderGraph&) {
            printf("Host pass\n");
        });

    const auto rootEdge = TRY_MAIN(hostPass.output<canta::V2::BufferIndex>());

    printf("Pass count: %d\n", graph.vertexCount());
    printf("Edge count: %d\n", graph.edgeCount());

    graph.setRoot(rootEdge);
    TRY_MAIN(graph.compile());

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