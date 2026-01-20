#include <Canta/Device.h>
#include <Canta/RenderGraph.h>
#include <Canta/PipelineManager.h>

#include "../cmake-build-debug-clang/_deps/ende-src/include/Ende/math/random.h"

int main() {

    auto device = TRY_MAIN(canta::Device::create({
        .applicationName = "sort",
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
        .multiQueue = false,
        .name = "graph"
    }));

    constexpr auto N = 1000;

    const auto buffer = renderGraph.addBuffer({
        .size = N * sizeof(u32),
        .name = "data"
    });
    const auto buffer1 = renderGraph.addBuffer({
        .size = N * sizeof(u32),
        .name = "pingpong"
    });

    u32 data[N] = {};
    u32 outputData[N] = {};
    for (auto& d : data) {
        d = ende::math::rand(0, N);
    }

    renderGraph.addUploadPass("data_upload", buffer, data);

    renderGraph.addPass({.name = "sort_pass"})
        .setPipeline(TRY_MAIN(pipelineManager.getPipeline(canta::Pipeline::CreateInfo{
            .compute = {
                .path = "sort.slang"
            },
            // .localSize = ende::math::Vec<3, u32>{256, 1, 1}
        })))
        .pushConstants(canta::Write(buffer), canta::Write(buffer1), N)
        .dispatchThreads(N);

    auto output = TRY_MAIN(renderGraph.addReadbackPass("data_read", buffer, outputData).aliasBufferOutput(0));

    renderGraph.setBackbuffer(output);

    if (!renderGraph.compile()) return -1;
    if (!renderGraph.execute({}, {}, {}, true)) return -2;

    TRY_MAIN(device->waitIdle());

    bool sorted = true;
    u32 prevValue = 0;
    for (i32 i = 0; i < N; ++i) {
        const auto& d = data[i];
        const auto& o = outputData[i];
        printf("(%d, %d), ", d, o);
        sorted = prevValue <= o;
        prevValue = o;
        if (!sorted) {
            printf("\nnot sorted properly\n");
        }
    }
    printf("\n");

    for (u32 i = 0; i < N; ++i) {
        auto& d = data[i];

        u32 count = 0;

        for (u32 j = 0; j < N; ++j) {
            auto& dd = data[j];
            if (d == dd)
                count++;
        }

        u32 outputCount = 0;
        for (u32 j = 0; j < N; ++j) {
            auto& o = outputData[j];
            if (d == o)
                outputCount++;
        }
        if (count != outputCount) {
            printf("different counts for index: %d, value: %d. %d in input and %d in output.\n", i, d, count, outputCount);
        }
    }

    device->endFrameCapture();

    return 0;
}
