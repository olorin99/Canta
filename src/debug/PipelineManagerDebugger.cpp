#include "../../include/Canta/debug/PipelineManagerDebugger.h"

#include "imgui.h"

auto canta::PipelineManagerDebugger::create(const CreateInfo& info) -> PipelineManagerDebugger {
    PipelineManagerDebugger debugger;
    debugger._pipelineManager = info.pipelineManager;
    return debugger;
}

void canta::PipelineManagerDebugger::render() {
    if (ImGui::Begin("Pipeline Manager")) {
        if (ImGui::Button("Reload All")) {
            _pipelineManager->reloadAll(true);
        }

        for (auto& pipeline : _pipelineManager->pipelines()) {
            ImGui::PushID(pipeline.second.index());
            if (pipeline.first.name.empty())
                ImGui::Text("%s", "Unknown pipeline name");
            else
                ImGui::Text("%s", pipeline.first.name.data());
            ImGui::SameLine();
            if (ImGui::Button("Reload")) {
                _pipelineManager->reload(pipeline.second);
            }
            ImGui::PopID();
        }
    }
    ImGui::End();
}
