#include <Canta/Device.h>
#include <Canta/RenderGraph.h>
#include <Canta/PipelineManager.h>
#include <Canta/util/sort.h>
#include <Ende/math/random.h>

struct Value {
    u32 value1;
    u32 value2;
    u32 value3;
};

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

    constexpr auto N = 1000000;

    const auto buffer = renderGraph.addBuffer({
        .size = N * sizeof(u32),
        .name = "keys"
    });
    const auto buffer1 = renderGraph.addBuffer({
        .size = N * sizeof(Value),
        .name = "values"
    });

    std::vector<u32> data = {};
    data.resize(N);
    std::vector<Value> valueData = {};
    valueData.resize(N);
    std::vector<u32> outputData = {};
    outputData.resize(N);
    for (auto& d : data) {
        d = ende::math::rand(0, N);
    }
    for (u32 i = 0; i < N; ++i) {
        auto& v = valueData[i];
        v.value1 = data[i];
        v.value2 = data[i];
        v.value3 = data[i];
    }

    const auto keys = TRY_MAIN(renderGraph.addUploadPass("data_upload", buffer, data).aliasBufferOutput(0));
    const auto values = TRY_MAIN(renderGraph.addUploadPass("data_upload_second", buffer1, valueData).aliasBufferOutput(0));

    // const auto sortOutputs = canta::sort<Value>(renderGraph, keys, values);

    const auto sortOutputs = canta::multiSort<Value>(renderGraph, keys, values);

    // auto index = keys;
    // for (u32 iteration = 0; iteration < 4; iteration++) {
    //     u32 shift = 8 * iteration;
    //     auto histogramOutput = TRY_MAIN(renderGraph.addPass({.name = "histogram_pass"})
    //         .addStorageBufferRead(index)
    //         .setPipeline(pipelineManager.getPipeline({
    //             .compute = {
    //                 .path = "multi_sort_histograms.slang"
    //             }
    //         }).value())
    //         .pushConstants(canta::Read(iteration % 2 == 0 ? keys : values), canta::Read(iteration % 2 == 0 ? values : keys), canta::Write(histograms), N, shift, numWorkgroups, numBlocksPerWorkgroup)
    //         .dispatchThreads(totalThreads).aliasBufferOutput(0));
    //
    //     auto sortOutput = renderGraph.addPass({.name = "sort_pass"})
    //         .setPipeline(pipelineManager.getPipeline({
    //             .compute = {
    //                 .path = "multi_sort.slang"
    //             }
    //         }).value())
    //         .pushConstants(canta::Write(iteration % 2 == 0 ? keys : values), canta::Write(iteration % 2 == 0 ? values : keys), canta::Read(histogramOutput), N, shift, numWorkgroups, numBlocksPerWorkgroup)
    //         .dispatchThreads(totalThreads).aliasBufferOutputs();
    //     index = sortOutput[0];
    // }


    const auto output = TRY_MAIN(renderGraph.addReadbackPass("data_read", sortOutputs.keys, outputData).aliasBufferOutput(0));
    // const auto output = TRY_MAIN(renderGraph.addReadbackPass("data_read", histogramOutput, outputData).aliasBufferOutput(0));
    // const auto output = TRY_MAIN(renderGraph.addReadbackPass("data_read", index, outputData).aliasBufferOutput(0));
    renderGraph.setBackbuffer(output);

    if (!renderGraph.compile()) return -1;
    if (!renderGraph.execute({}, {}, {}, true)) return -2;

    // TRY_MAIN(device->waitIdle());

    bool totalSorted = true;
    bool sorted = true;
    u32 prevValue = 0;
    for (i32 i = 0; i < N; ++i) {
        const auto& d = data[i];
        const auto& o = outputData[i];
        // printf("(%d, %d), ", d, o);
        sorted = prevValue <= o;
        prevValue = o;
        if (!sorted) {
            totalSorted = sorted;
            printf("\nnot sorted properly\n");
        }
    }
    printf("\n");
    printf("%s\n", totalSorted ? "Sorted" : "Unsorted");

    // for (u32 i = 0; i < N; ++i) {
    //     auto& d = data[i];
    //
    //     u32 count = 0;
    //
    //     for (u32 j = 0; j < N; ++j) {
    //         auto& dd = data[j];
    //         if (d == dd)
    //             count++;
    //     }
    //
    //     u32 outputCount = 0;
    //     for (u32 j = 0; j < N; ++j) {
    //         auto& o = outputData[j];
    //         if (d == o)
    //             outputCount++;
    //     }
    //     if (count != outputCount) {
    //         printf("different counts for index: %d, value: %d. %d in input and %d in output.\n", i, d, count, outputCount);
    //     }
    // }

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
