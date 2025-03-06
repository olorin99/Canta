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
        .logLevel = spdlog::level::off
    }).value();
    auto pipelineManager = canta::PipelineManager::create({
        .device = device.get(),
        .rootPath = CANTA_SRC_DIR
    });

    SECTION("build shader") {
        auto shader = pipelineManager.getShader({
            .glsl = R"(
//GLSL version to use
#version 460

//size of a workgroup for compute
layout (local_size_x = 16, local_size_y = 16) in;

//descriptor bindings for the pipeline
layout(rgba16f,set = 0, binding = 0) uniform image2D image;


void main()
{
    ivec2 texelCoord = ivec2(gl_GlobalInvocationID.xy);
	ivec2 size = imageSize(image);

    if(texelCoord.x < size.x && texelCoord.y < size.y)
    {
        vec4 color = vec4(0.0, 0.0, 0.0, 1.0);

        if(gl_LocalInvocationID.x != 0 && gl_LocalInvocationID.y != 0)
        {
            color.x = float(texelCoord.x)/(size.x);
            color.y = float(texelCoord.y)/(size.y);
        }

        imageStore(image, texelCoord, color);
    }
}

)",
            .stage = canta::ShaderStage::COMPUTE,
            .name = "test"
        });
        REQUIRE(shader.has_value());
    }

#ifdef CANTA_USE_SLANG
    SECTION("build slang") {
        auto shader = pipelineManager.getShader({
            .slang = R"(
// hello-world.slang
[[vk::binding(0, 1)]] StructuredBuffer<float> buffer0;
[[vk::binding(1, 1)]] StructuredBuffer<float> buffer1;
[[vk::binding(2, 1)]] RWStructuredBuffer<float> result;

[shader("compute")]
[numthreads(1,1,1)]
void computeMain(uint3 threadId : SV_DispatchThreadID)
{
    uint index = threadId.x;
    result[index] = buffer0[index] + buffer1[index];
}

)"
        });
        REQUIRE(shader.has_value());
    }
#endif

    SECTION("invalid path") {
        auto shader = pipelineManager.getShader({
            .path = "alkdfjalkdf",
            .stage = canta::ShaderStage::COMPUTE,
            .name = "test"
        });
        REQUIRE(!shader.has_value());
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