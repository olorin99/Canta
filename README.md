# Canta
Canta is a Tolkien elvish word meaning [shape/framework](https://www.elfdict.com/w/canta).
This library acts as a framework for creating vulkan applications.

## Features
- Device creation.
- Resource management.
- Indexed ("bindless") resources.
- Automatic frame synchronisation.
- Render graph.
- Pipeline/Shader management.
- Included ImGui backend.

### Device Creation
Simple graphics device creation.

```c++
auto device = canta::Device::create({
    .applicationName = "hello world",
    .enableMeshShading = false,
    // ... enable other feature flags
});
```

### Render Graph
Includes a render graph to help with synchronisation dependencies between shader passes.

```c++
// Create graph
auto renderGraph = canta::RenderGraph::create({
    .device = device.get(),
    .name = "graph"
});

// Create graph resources
auto objectBuffer = renderGraph.addBuffer({
    .size = sizeof(Object) * numObjects,
    .name = "object_buffer"
});

auto backbuffer = renderGraph.addImage({
    .width = 1920,
    .height = 1080,
    .format = canta::Format::RGBA8_SRGB,
    .name = "backbuffer"
});

// Create graph passes
renderGraph.addPass({
    .name = "run_objects",
    .type = canta::PassType::COMPUTE
})
    .addStorageBufferWrite(objectBuffer, canta::PipelineStage::COMPUTE_SHADER) // add write dependency
    .setPipeline(runPipeline)
    .pushConstants(objectBuffer)
    .dispatch(x, y);

renderGraph.addPass({
    .name = "main_draw"
    .type = canta::PassType::COMPUTE
})
    .addStorageBufferRead(objectBuffer, canta::PipelineStage::COMPUTE_SHADER) // reads output from "run_objects" pass
    .addStorageImageWrite(backbuffer, canta::PipelineStage::COMPUTE_SHADER)
    .setPipeline(drawPipeline)
    .pushConstants(backbuffer, objectBuffer)
    .dispatch(x, y);

// Set output target
renderGraph.setBackbuffer(backbuffer);

// Compile and run
renderGraph.compile();
renderGraph.execute({}, {}); // First arg is semaphores to wait on and second is semaphores to signal for use with frame sync

```

### Pipeline Manager
Includes a pipeline manager which facilitates the creation and management of shaders and pipeline objects. Shaders/Pipelines are cached after the first use so passing the same arguments to getShader/getPipeline will return the same object.

```c++
auto pipelineManager = canta::PipelineManager::create({
    .device = device.get();
    .rootPath = "assets/shaders"
});

// Create and cache shader object
auto shader = pipelineManager.getShader({
    .path = "path/to/shader",
    .stage = canta::ShaderStage::COMPUTE,
    .name = "compute_shader"
});

// Create and cache pipeline.
auto pipeline  = pipelineManager.getPipeline({
    .vertex = {
        .module = pipelineManager.getShader({
            .path = "path/to/vertex/shader",
            .stage = canta::ShaderStage::VERTEX
            .name = "vertex_shader"
        }),
        .entryPoint = "main"
    },
    .fragment = {
        .module = pipelineManager.getShader({
            .path - "path/to/fragment/shader",
            .stage = canta::ShaderStage::FRAGMENT,
            .name = "fragment_shader"
        }),
        .entryPoint = "fragmentMain"
    },
    .name = "raster_pipeline"
});

```

## Dependencies
### System
- SDL2
- Vulkan
- spdlog

### Downloaded with cmake
- Ende
- volk
- VulkanMemoryAllocator
- Tessil/robin-map
- SPIRV-Cross
- slang

### Embeded in project
- imgui
- rapidjson
- imnodes