#ifndef CANTA_IMGUICONTEXT_H
#define CANTA_IMGUICONTEXT_H

#include <Canta/CommandBuffer.h>
#include <imgui.h>

namespace canta {

    class Device;
    class SDLWindow;

    class ImGuiContext {
    public:

        struct CreateInfo {
            Device* device = nullptr;
            SDLWindow* window = nullptr;
        };
        static auto create(CreateInfo info) -> ImGuiContext;

        ImGuiContext() = default;

        ImGuiContext(ImGuiContext&& rhs) noexcept;
        auto operator=(ImGuiContext&& rhs) noexcept -> ImGuiContext&;

        void beginFrame();

        void render(ImDrawData* drawData, CommandBuffer& commandBuffer, Format format);

        bool createFontsTexture(CommandBuffer& commandBuffer);

        void processEvent(void* event);

    private:

        void setupRenderState(ImDrawData* drawData, CommandBuffer& commandBuffer, i32 width, i32 height, Format format);

        auto createPipeline(Format format) -> PipelineHandle;

        Device* _device = nullptr;
        PipelineHandle _pipeline = {};
        Format _pipelineFormat = Format::RGBA8_UNORM;

        BufferHandle _vertexBuffer = {};
        BufferHandle _indexBuffer = {};

        SamplerHandle _sampler = {};

        ImageHandle _fontImage = {};
        BufferHandle _uploadBuffer = {};

        SDLWindow* _window = nullptr;

    };

    class RenderGraph;
    void drawRenderGraph(RenderGraph& renderGraph);
    void renderGraphDebugUi(RenderGraph& graph);

}

#endif //CANTA_IMGUICONTEXT_H
