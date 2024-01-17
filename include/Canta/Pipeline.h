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
        CullMode cullMode = CullMode::BACK;
        FrontFace frontFace = FrontFace::CW;
        PolygonMode polygonMode = PolygonMode::FILL;
        f32 lineWidth = 1.f;
        bool depthClamp = false;
        bool rasterDiscard = false;
        bool depthBias = false;
    };

    struct DepthState {
        bool test = false;
        bool write = false;
        CompareOp compareOp = CompareOp::LESS;
    };

    struct BlendState {
        bool blend = false;
        BlendFactor srcFactor = BlendFactor::ONE;
        BlendFactor dstFactor = BlendFactor::ONE;
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
            BlendState blendState = {};
            std::span<Format> colourFormats = {};
            Format depthFormat = Format::UNDEFINED;
            PipelineMode mode = PipelineMode::GRAPHICS;
        };

        Pipeline() = default;

        ~Pipeline();

        Pipeline(Pipeline&& rhs) noexcept;
        auto operator=(Pipeline&& rhs) noexcept -> Pipeline&;

        auto pipeline() const -> VkPipeline { return _pipeline; }
        auto layout() const -> VkPipelineLayout { return _layout; }
        auto mode() const -> PipelineMode { return _mode; }
        auto interface() const -> const ShaderInterface& { return _interface; }

    private:
        friend Device;

        Device* _device = nullptr;
        VkPipeline _pipeline = VK_NULL_HANDLE;
        VkPipelineLayout _layout = VK_NULL_HANDLE;
        PipelineMode _mode = PipelineMode::GRAPHICS;
        ShaderInterface _interface = {};


    };

}

#endif //CANTA_PIPELINE_H
