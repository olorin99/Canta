#include <catch2/catch_test_macros.hpp>

#include <Canta/ResourceList.h>
#include <Canta/Buffer.h>
#include <Canta/RenderGraph.h>
#include <Canta/PipelineManager.h>


TEST_CASE("Resource reference counting", "[refcount]") {
    canta::ResourceList<canta::Buffer> list;
    auto handle = list.allocate();
    auto handle1 = list.allocate();

    SECTION("copy") {
        auto tmp = handle;

        REQUIRE(tmp.count() == 2);
        REQUIRE(tmp.index() == handle.index());
    }

    SECTION("move") {
        auto tmp = std::move(handle);

        REQUIRE(tmp.count() == 1);
        REQUIRE(handle.index() == -1);
    }

    SECTION("destroy") {
        handle = {};

        REQUIRE(handle.count() == 0);
    }

    SECTION("swap") {
        auto oldIndex = handle.index();
        auto newIndex = handle1.index();
        auto tmp = list.swap(handle, handle1);

        REQUIRE(handle.index() == newIndex);
        REQUIRE(handle1.index() == oldIndex);
    }

}

TEST_CASE("PipelineManager", "[pipelinemanager]") {
    auto device = canta::Device::create({
        .applicationName = "tests",
        .headless = true,
        .logLevel = spdlog::level::err
    }).value();
    auto pipelineManager = canta::PipelineManager::create({
        .device = device.get(),
        .rootPath = CANTA_SRC_DIR
    });

    SECTION("build pipeline") {
        auto shader = pipelineManager.getPipeline({
            .compute = {
            .slang = R"(
import canta;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(
    uint3 threadId: SV_DispatchThreadID,
    uniform uint* buffer,
    uniform uint count,
) {
    const uint idx = threadId.x;
    if (idx >= count)
        return;

    buffer[idx] = idx;
}
)",
        }});
        if (!shader.has_value()) {
            printf("the error is error: %d", shader.error());
        }
        REQUIRE(shader.has_value());
    }


    SECTION("invalid path") {
        auto shader = pipelineManager.getPipeline({
            .compute = {
                .path = "alkdfjalkdf",
            }
        });
        REQUIRE(!shader.has_value());
    }
}

TEST_CASE("RenderGraph", "[rendergraph]") {
    auto device = canta::Device::create({
        .applicationName = "tests",
        .logLevel = spdlog::level::off
    }).value();
    auto renderGraph = *canta::RenderGraph::create({
        .device = device.get(),
        .name = "tests"
    });

    auto backbuffer = renderGraph.addImage({ .name = "backbuffer" });
    auto image1 = renderGraph.addImage({ .name = "image1" });
    auto image2 = renderGraph.addImage({ .name = "image2" });
    auto image3 = renderGraph.addImage({ .name = "image3" });
    auto image4 = renderGraph.addImage({ .name = "image4" });
    auto image5 = renderGraph.addImage({ .name = "image5" });
    auto image6 = renderGraph.addImage({ .name = "image6" });


    SECTION("basic graph") {
        auto pass1 = renderGraph.compute("pass1")
                .addStorageImageWrite(image1);

        auto pass2 = renderGraph.compute("pass2")
                .addStorageImageRead(*pass1.output<canta::ImageIndex>())
                .addStorageImageWrite(backbuffer);

        auto pass3 = renderGraph.compute("pass3")
                .addStorageImageRead(backbuffer);

        renderGraph.setRoot(*pass2.output<canta::ImageIndex>());
        REQUIRE(renderGraph.compile());
    }

    SECTION("cyclic graph") {

        auto pass1 = renderGraph.compute("pass1")
                .addStorageImageWrite(image1);

        auto pass2 = renderGraph.compute("pass2")
                .addStorageImageRead(*pass1.output<canta::ImageIndex>())
                .addStorageImageWrite(image2);

        auto pass3 = renderGraph.compute("pass3")
                .addStorageImageRead(*pass2.output<canta::ImageIndex>())
                .addStorageImageWrite(image3);

        pass2.addStorageImageRead(*pass3.output<canta::ImageIndex>());
        pass2.addStorageImageRead(*pass2.output<canta::ImageIndex>());

        auto pass4 = renderGraph.compute("pass4")
                .addStorageImageRead(*pass3.output<canta::ImageIndex>())
                .addStorageImageWrite(backbuffer);

        renderGraph.setRoot(*pass4.output<canta::ImageIndex>());
        REQUIRE(!renderGraph.compile().has_value());
    }

}