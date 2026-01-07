
#include <random>
#include <Canta/Device.h>
#include <Canta/RenderGraph.h>
#include <Canta/PipelineManager.h>

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

void multiplyMatrix(u32 N, f32* lhs, f32* rhs, f32* output) {
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

    auto device = canta::Device::create({
        .applicationName = "matrix multiplication example",
        .headless = true,
        .enableMeshShading = false,
        .enableRenderDoc = true,
    }).value();

    device->triggerCapture();
    device->startFrameCapture();

    auto pipelineManager = canta::PipelineManager::create({
        .device = device.get(),
        .rootPath = CANTA_SRC_DIR"/examples"
    });

    auto renderGraph = canta::RenderGraph::create({
        .device = device.get(),
        .multiQueue = false,
        .name = "graph"
    });


    constexpr int N = 64;

    f32 lhsData[N * N] = {};
    genMatrix(N, lhsData);
    f32 rhsData[N * N] = {};
    genMatrix(N, rhsData);

    auto lhs = renderGraph.addBuffer({
        .size = N * N * sizeof(f32),
        .name = "lhs_matrix"
    });

    auto rhs = renderGraph.addBuffer({
        .size = N * N * sizeof(f32),
        .name = "rhs_matrix"
    });

    auto outputBuffer = device->createBuffer({
        .size = N * N * sizeof(f32),
        .usage = canta::BufferUsage::STORAGE,
        .type = canta::MemoryType::READBACK,
        .persistentlyMapped = true,
        .name = "output"
    });
    auto output = renderGraph.addBuffer({
        .handle = outputBuffer,
        .name = "output"
    });

    renderGraph.addPass({.name = "lhs_upload", .type = canta::PassType::HOST})
        .addStorageBufferWrite(lhs)
        .setExecuteFunction([&] (auto& buffer, auto& graph) {
            graph.getBuffer(lhs)->data(lhsData);
        });

    renderGraph.addPass({.name = "rhs_upload", .type = canta::PassType::HOST})
        .addStorageBufferWrite(rhs)
        .setExecuteFunction([&] (auto& buffer, auto& graph) {
            graph.getBuffer(rhs)->data(rhsData);
        });

    renderGraph.addPass({
        .name = "multiply"
    })
        // .addStorageBufferRead(lhs)
        // .addStorageBufferRead(rhs)
        // .addStorageBufferWrite(output)
        .setPipeline(pipelineManager.getPipeline({
            .compute = {
                .module = pipelineManager.getShader({
                    .path = "matrix_multiply.slang",
                    .stage = canta::ShaderStage::COMPUTE,
                    .name = "matrix_multiply"
                }).value(),
                .entryPoint = "main"
            }
        }).value())
        .pushConstants(canta::Read(lhs), canta::Read(rhs), canta::Write(output), N)
        .dispatchThreads(N, N);

    renderGraph.setBackbuffer(output);

    renderGraph.compile();
    renderGraph.execute({}, {}, {}, true);


    f32 outputData[N * N] = {};
    const auto mapped = outputBuffer->map();
    std::memcpy(outputData, mapped.address(), N * N * sizeof(f32));

    printf("LHS: \n");
    printMatrix(N, lhsData);

    printf("RHS: \n");
    printMatrix(N, rhsData);

    printf("Output: \n");
    printMatrix(N, outputData);

    printf("Validate output\n");

    f32 outputData2[N * N] = {};
    multiplyMatrix(N, lhsData, rhsData, outputData2);
    printMatrix(N, outputData2);

    f32 avgErr = 0.0;

    for (u32 i = 0; i < N * N; i++) {
        f32 diff = std::abs(outputData[i] - outputData2[i]);
        if (diff > 2.00000001) {
            printf("Error at index (%d) with diff of %f. LHS: %f vs RHS: %f\n", i, diff, outputData[i], outputData2[i]);
        }
        avgErr += diff;
    }

    avgErr /= (N * N);
    printf("Average error: %f\n", avgErr);

    for (auto timers = renderGraph.timers(); auto&[name, timer] : timers) {
        printf("Timer: %s\n", name.c_str());
        printf("Milliseconds: %f\n", timer.result().value() / 1e6);
    }


    device->endFrameCapture();

    return 0;
}