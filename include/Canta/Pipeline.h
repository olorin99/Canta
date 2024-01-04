#ifndef CANTA_PIPELINE_H
#define CANTA_PIPELINE_H

#include <string_view>
#include <span>


#include <Canta/ResourceList.h>
#include <Canta/ShaderModule.h>

namespace canta {

    class Device;

    using ShaderHandle = Handle<ShaderModule, ResourceList<ShaderModule>>;

    struct ShaderInfo {
        ShaderHandle module = {};
        std::string_view entryPoint = {};
    };

    struct RasterState {

    };

    struct DepthState {

    };

    enum PipelineMode {
        GRAPHICS = 1,
        COMPUTE = 2
    };

    class Pipeline {
    public:


        struct CreateInfo {
            std::span<ShaderInfo> shaders = {};
            RasterState rasterState = {};
            DepthState depthState = {};
            PipelineMode mode = PipelineMode::GRAPHICS;
        };

        Pipeline() = default;

    private:
        friend Device;

        VkPipeline _pipeline;


    };

}

#endif //CANTA_PIPELINE_H
