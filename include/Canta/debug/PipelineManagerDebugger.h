#ifndef CANTA_PIPELINEMANAGERDEBUGGER_H
#define CANTA_PIPELINEMANAGERDEBUGGER_H

#include <Canta/PipelineManager.h>

namespace canta {

    class PipelineManagerDebugger {
    public:

        struct CreateInfo {
            PipelineManager* pipelineManager;
        };

        static auto create(const CreateInfo& info) -> PipelineManagerDebugger;

        void render();

    private:
        PipelineManager* _pipelineManager = nullptr;

    };
}

#endif //CANTA_PIPELINEMANAGERDEBUGGER_H