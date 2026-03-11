#ifndef CANTA_RENDERGRAPHDEBUGGER_H
#define CANTA_RENDERGRAPHDEBUGGER_H

#include <string_view>
#include <Canta/RenderGraph.h>

namespace canta {

    class RenderGraphDebugger {
    public:

        struct CreateInfo {
            RenderGraph* renderGraph;
        };
        static auto create(const CreateInfo& info) -> RenderGraphDebugger;

        void render();

        void drawRenderGraph();

        void drawResourceUsage();

        // void debug();

        void setRenderGraph(RenderGraph *renderGraph) { _renderGraph = renderGraph; }

        void setRoot(const RenderPass& basePass) { _basePass = basePass.name(); }

        void setBaseResource(const ImageIndex baseResource) {
            _baseResourceId = baseResource.id;
            _baseResourceIndex = baseResource.index;
        }

        void setBaseResource(const BufferIndex baseResource) {
            _baseResourceId = baseResource.id;
            _baseResourceIndex = baseResource.index;
        }

    private:
        RenderGraph *_renderGraph = nullptr;

        std::string _basePass;
        i32 _baseResourceId = -1;
        u32 _baseResourceIndex = 0;

        std::string _selectedPass;
        RenderGraph::Edge _selectedResource = {};

        bool _copyResource = false;

        ende::math::Vec<2, i32> _viewportSize = {};
        ende::math::Vec<2, i32> _viewportOffset = {};
    };

}


#endif //CANTA_RENDERGRAPHDEBUGGER_H