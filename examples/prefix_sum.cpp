
#include <Canta/RenderGraph.h>
#include <Canta/PipelineManager.h>
#include <Ende/math/random.h>

#include "Canta/util/prefixSum.h"

int main() {

    auto device = TRY_MAIN(canta::Device::create({
        .applicationName = "prefix_sum",
        .headless = true,
        .enableMeshShading = false,
        .enableAsyncComputeQueue = true,
        .enableAsyncTransferQueue = false,
        .enableRenderDoc = true,
    }));

    device->triggerCapture();
    device->startFrameCapture();

    auto pipelineManager = canta::PipelineManager::create({
        .device = device.get(),
        .rootPath = CANTA_SRC_DIR"/examples",
    });

    auto renderGraph = TRY_MAIN(canta::RenderGraph::create({
        .device = device.get(),
        .multiQueue = true,
        .name = "graph"
    }));

    constexpr auto N = 2 << 16;

    auto buffer = renderGraph.addBuffer({
        .size = N * sizeof(u32),
        .name = "values"
    });

    std::vector<u32> data = {};
    data.resize(N);
    std::vector<u32> outputData = {};
    data.resize(N);
    for (auto& d : data) {
        d = ende::math::rand(0, N);
    }

    const auto values = TRY_MAIN(renderGraph.addUploadPass("data_upload", buffer, data).aliasBufferOutput(0));


    const auto summedValues = canta::prefixSumExclusive(renderGraph, values, N);
    // const auto summedValues = canta::prefixSumInclusive(renderGraph, values, N);

    const auto output = TRY_MAIN(renderGraph.addReadbackPass("data_read", summedValues, outputData).aliasBufferOutput(0));
    renderGraph.setBackbuffer(output);

    TRY_MAIN(renderGraph.compile());
    TRY_MAIN(renderGraph.execute({}, {}, {}, true));

    u64 totalTime = 0;
    for (auto timers = renderGraph.timers(); auto&[name, timer] : timers) {
        const auto time = TRY_MAIN(timer.result());
        printf("Pass: %s (%fms)\n", name.c_str(), time / 1e6);
        totalTime += time;
    }
    printf("Total time: %fms\n", totalTime / 1e6);

    device->endFrameCapture();

    return 0;
}
