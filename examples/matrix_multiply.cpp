
#include <Canta/Device.h>
#include <Canta/RenderGraph.h>
#include <Canta/PipelineManager.h>

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


    const int N = 64;

    f32 lhsData[N * N] = {};
    f32 rhsData[N * N] = {};

    auto lhs = renderGraph.addBuffer({
        .size = N * N * sizeof(f32),
        .name = "lhs"
    });

    auto rhs = renderGraph.addBuffer({
        .size = N * N * sizeof(f32),
        .name = "rhs"
    });

    auto output = renderGraph.addBuffer({
        .size = N * N * sizeof(f32),
        .name = "output"
    });

    renderGraph.addPass({.name = "lhs_upload", .type = canta::PassType::HOST})
        .addStorageBufferWrite(lhs, canta::PipelineStage::HOST)
        .setExecuteFunction([&] (auto& buffer, auto& graph) {
            graph.getBuffer(lhs)->data(lhsData);
        });

    renderGraph.addPass({.name = "rhs_upload", .type = canta::PassType::HOST})
        .addStorageBufferWrite(rhs, canta::PipelineStage::HOST)
        .setExecuteFunction([&] (auto& buffer, auto& graph) {
            graph.getBuffer(rhs)->data(rhsData);
        });

    renderGraph.addPass({
        .name = "multiply"
    })
        .addStorageBufferRead(lhs, canta::PipelineStage::COMPUTE_SHADER)
        .addStorageBufferRead(rhs, canta::PipelineStage::COMPUTE_SHADER)
        .addStorageBufferWrite(output, canta::PipelineStage::COMPUTE_SHADER)
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
        .pushConstants(lhs, rhs, output)
        .dispatchThreads(N, N);

    renderGraph.setBackbuffer(output);

    renderGraph.compile();
    renderGraph.execute({}, {}, {}, true);

    device->endFrameCapture();

    return 0;
}