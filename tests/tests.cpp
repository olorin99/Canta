#include <catch2/catch_test_macros.hpp>

#include <Canta/ResourceList.h>
#include <Canta/Buffer.h>
#include <Canta/RenderGraph.h>


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

TEST_CASE("RenderGraph", "[rendergraph]") {
    auto device = canta::Device::create({
        .applicationName = "tests",
        .logLevel = spdlog::level::off
    }).value();
    auto renderGraph = canta::RenderGraph::create({
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
        auto& pass1 = renderGraph.addPass("pass1")
                .addStorageImageWrite(image1, canta::PipelineStage::COMPUTE_SHADER);

        auto& pass2 = renderGraph.addPass("pass2")
                .addStorageImageRead(image1, canta::PipelineStage::COMPUTE_SHADER)
                .addStorageImageWrite(backbuffer, canta::PipelineStage::COMPUTE_SHADER);

        auto& pass3 = renderGraph.addPass("pass3")
                .addStorageImageRead(backbuffer, canta::PipelineStage::COMPUTE_SHADER);

        renderGraph.setBackbuffer(backbuffer);
        REQUIRE(renderGraph.compile());
    }

    SECTION("cyclic graph") {

        auto& pass1 = renderGraph.addPass("pass1")
                .addStorageImageWrite(image1, canta::PipelineStage::COMPUTE_SHADER);

        auto& pass2 = renderGraph.addPass("pass2")
                .addStorageImageRead(image1, canta::PipelineStage::COMPUTE_SHADER)
                .addStorageImageWrite(image2, canta::PipelineStage::COMPUTE_SHADER);

        auto& pass3 = renderGraph.addPass("pass3")
                .addStorageImageRead(image2, canta::PipelineStage::COMPUTE_SHADER)
                .addStorageImageWrite(image1, canta::PipelineStage::COMPUTE_SHADER);

        auto& pass4 = renderGraph.addPass("pass4")
                .addStorageImageRead(image1, canta::PipelineStage::COMPUTE_SHADER)
                .addStorageImageWrite(backbuffer, canta::PipelineStage::COMPUTE_SHADER);

        renderGraph.setBackbuffer(backbuffer);
        REQUIRE(!renderGraph.compile());
    }

}