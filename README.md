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
auto objectBuffer = renderGraph->addBuffer({
    .size = sizeof(Object) * numObjects,
    .name = "object_buffer"
});

auto backbuffer = renderGraph->addImage({
    .width = 1920,
    .height = 1080,
    .format = canta::Format::RGBA8_SRGB,
    .name = "backbuffer"
});

// Create graph passes
const auto objects = rendergraph->compute("run_objects", runPipeline)
    .pushConstants(Write(objectBuffer))
    .dispatchThreads(x, y).output<BufferIndex>();

const auto finalBackbuffer = renderGraph->compute("main_draw", drawPipeline)
    .pushConstants(Write(backbuffer), Read(objects))
    .dispatchThreads(x, y).output<ImageIndex>();

// Set output target
renderGraph->setRoot(finalBackbuffer);

// Compile and run
renderGraph.compile();
renderGraph.run({}, {}, false); // First arg is semaphores to wait on and second is semaphores to signal for use with frame sync, third is whether to run the graph asynchronously or not.

```

### Shaders
There are several ways to use shaders with canta. Either compiled and embedded through the build script or evaluated at runtime using a pipeline manager.

#### Embedded
Shaders can be embedded into the binary using cmake functions included with the library. The function takes two named parameters. `NAME` which designates a namespace the shaders will be available under. And `SHADERS` which is a list of paths to shaders to embed.
```cmake
embed_shaders(
        NAME project_name
        SHADERS path/to/shader/file.spv path/to/shader/file.slang
)
```

Additionally, the included slang compiler can be used to build slang shaders at build time which can then be embedded into the binary.
```cmake
compile_shaders(path/to/shader/file.slang path/to/shader/file1.slang)
embed_shaders(
        NAME project_name
        SHADERS ${COMPILED_SHADER_OUTPUTS}
)
```
Once embedded the shader data is available in the `embedded_shaders_${NAME}.h` header file. The shader data is contained in a namespace `NAME` in variables named `${file_name}_${file_type}_embedded` with `file_type` being either `slang` or `spv`. Embedded spirv compute shaders will also have a helper function generated which facilitates using the shader in a  render graph. These function are named using the file name.
```c++
#include "embedded_shaders_PROJECT.h"

auto shader_data = project_name::file_slang_embedded;

auto computePass = project_name::file(x, y, z)(graph, push, constants, go, here, with, Read(and), Write(depenencies));
```

#### Pipeline Manager
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
        .path = "can/also/just/provide/a/path/for/the/shader.slang",
        .entryPoint = "fragmentMain"
    },
    .name = "raster_pipeline"
});
```

## Dependencies
- SDL2
- Vulkan
- spdlog
- Ende
- volk
- VulkanMemoryAllocator
- Tessil/robin-map
- SPIRV-Cross
- slang
- imgui
- rapidjson
- imnodes

## Build
### Install dependencies
#### Arch
`sudo pacman -Syu libsdl2`
#### Ubuntu
`sudo apt install libsdl2-dev libspdlog-dev libvulkan-dev`

To build library standalone.
```bash
git clone https://git.melamar.xyz/olorin99/Canta
cd Canta
mkdir build
cd build
cmake -G Ninja -DCMAKE_BUILD_TYPE=Release ..
cmake --build . --target Canta -j8
```

To include in cmake project.
```cmake
FetchContent_Declare(
        Canta
        GIT_REPOSITORY  https://git.melamar.xyz/olorin99/Canta
        GIT_TAG         "main"
)
FetchContent_MakeAvailable(Canta)
```