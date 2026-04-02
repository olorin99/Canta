
#include <Canta/RenderGraph.h>
#include <Canta/PipelineManager.h>
#include <Ende/math/random.h>

#include "Canta/util/prefixSum.h"
#include "Canta/util/random.h"

int main() {

    auto device = maybe_conv(i32, canta::Device::create({
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

    auto renderGraph = maybe_conv(i32, canta::RenderGraph::create({
        .device = device.get(),
        .multiQueue = true,
        // .name = "graph"
    }));

    constexpr auto N = 2 << 16;

    std::vector<u32> outputData = {};
    outputData.resize(N);

    const auto values = canta::randomListUint(renderGraph, 100, 5000, N);

    const auto summedValues = canta::prefixSumExclusive(renderGraph, values, N);
    // const auto summedValues = canta::prefixSumInclusive(renderGraph, values, N);

    const auto output = maybe_conv(i32, renderGraph.host("data_read").readback(summedValues, outputData));
    renderGraph.setRoot(output);

    maybe_conv(i32, renderGraph.compile());
    maybe_conv(i32, renderGraph.run({}, {}, false));

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
