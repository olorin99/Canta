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
        std::string_view entryPoint = "main";
    };

    struct RasterState {
        CullMode cullMode = CullMode::BACK;
        FrontFace frontFace = FrontFace::CW;
        PolygonMode polygonMode = PolygonMode::FILL;
        f32 lineWidth = 1.f;
        bool depthClamp = false;
        bool rasterDiscard = false;
        bool depthBias = false;

        auto operator<=>(const RasterState&) const = default;
    };

    struct DepthState {
        bool test = false;
        bool write = false;
        CompareOp compareOp = CompareOp::LESS;

        auto operator<=>(const DepthState&) const = default;
    };

    struct BlendState {
        bool blend = false;
        BlendFactor srcFactor = BlendFactor::ONE;
        BlendFactor dstFactor = BlendFactor::ONE;

        auto operator<=>(const BlendState&) const = default;
    };

    struct VertexInputBinding {
        u32 binding = 0;
        u32 stride = 0;
        u32 inputRate = 0;

        auto operator<=>(const VertexInputBinding&) const = default;
    };

    struct VertexInputAttribute {
        u32 location = 0;
        u32 binding = 0;
        Format format = Format::RGBA32_SFLOAT;
        u32 offset = 0;

        auto operator<=>(const VertexInputAttribute&) const = default;
    };

    enum PipelineMode {
        GRAPHICS = 1,
        COMPUTE = 2
    };

    class Pipeline {
    public:


        struct CreateInfo {
            ShaderInfo vertex = {};
            ShaderInfo tesselationControl = {};
            ShaderInfo tesselationEvaluation = {};
            ShaderInfo geometry = {};
            ShaderInfo fragment = {};
            ShaderInfo compute = {};
            ShaderInfo rayGen = {};
            ShaderInfo anyHit = {};
            ShaderInfo closestHit = {};
            ShaderInfo miss = {};
            ShaderInfo intersection = {};
            ShaderInfo callable = {};
            ShaderInfo task = {};
            ShaderInfo mesh = {};
            RasterState rasterState = {};
            DepthState depthState = {};
            BlendState blendState = {};
            std::vector<VertexInputBinding> inputBindings = {};
            std::vector<VertexInputAttribute> inputAttributes = {};
            PrimitiveTopology topology = PrimitiveTopology::TRIANGLE_LIST;
            bool primitiveRestart = false;
            std::vector<Format> colourFormats = {};
            Format depthFormat = Format::UNDEFINED;
            std::string_view name = {};
        };

        Pipeline() = default;

        ~Pipeline();

        Pipeline(Pipeline&& rhs) noexcept;
        auto operator=(Pipeline&& rhs) noexcept -> Pipeline&;

        auto pipeline() const -> VkPipeline { return _pipeline; }
        auto layout() const -> VkPipelineLayout { return _layout; }
        auto mode() const -> PipelineMode { return _mode; }
        auto interface() const -> const ShaderInterface& { return _interface; }
        auto info() const -> const CreateInfo& { return _info; }

    private:
        friend Device;

        Device* _device = nullptr;
        VkPipeline _pipeline = VK_NULL_HANDLE;
        VkPipelineLayout _layout = VK_NULL_HANDLE;
        PipelineMode _mode = PipelineMode::GRAPHICS;
        ShaderInterface _interface = {};
        std::string _name = {};
        CreateInfo _info = {};


    };

}

#endif //CANTA_PIPELINE_H
