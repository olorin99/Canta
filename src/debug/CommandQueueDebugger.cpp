#include <Canta/debug/CommandQueueDebugger.h>
#include <Canta/Device.h>
#include <imgui.h>

auto canta::CommandQueueDebugger::create(const CreateInfo &info) -> CommandQueueDebugger {
    CommandQueueDebugger debugger;
    debugger._device = info.device;
    return debugger;
}

void canta::CommandQueueDebugger::render() {
    if (ImGui::Begin("Command Queue")) {
        auto frameIndex = _device->prevFlyingIndex();

        auto debugMarkers = _device->getFrameDebugMarkers(frameIndex);

        u32 id = 0;
        for (auto& markerData : debugMarkers) {
            auto type = util::getMarkerType(markerData);
            auto stage = util::getMarkerStage(markerData);

            ImGui::PushID(id++);
            if (ImGui::TreeNode(std::format("{} {}", pipelineStageString(stage), util::markerString(type)).c_str())) {

                switch (type) {
                case util::FunctionMarker::None: {
                }
                    break;
                case util::FunctionMarker::Draw: {
                    const auto marker = reinterpret_cast<util::Draw*>(markerData.data());
                    ImGui::Text("count: %d, instanceCount: %d, firstVertex: %d, firstInstance: %d", marker->count, marker->instanceCount, marker->firstVertex, marker->firstInstance);
                }
                    break;
                case util::FunctionMarker::DrawIndexed: {
                    const auto marker = reinterpret_cast<util::DrawIndexed*>(markerData.data());
                    ImGui::Text("count: %d, instanceCount: %d, firstVertex: %d, firstIndex: %d, firstInstance: %d", marker->count, marker->instanceCount, marker->firstVertex, marker->firstIndex, marker->firstInstance);
                }
                    break;
                case util::FunctionMarker::DrawIndirect: {
                    const auto marker = reinterpret_cast<util::DrawIndirect*>(markerData.data());
                    ImGui::Text("bufferIndex: %d, offset: %d, drawCount: %d, stride: %d", marker->bufferIndex, marker->offset, marker->drawCount, marker->stride);
                }
                    break;
                case util::FunctionMarker::DrawIndirectCount: {
                    const auto marker = reinterpret_cast<util::DrawIndirectCount*>(markerData.data());
                    ImGui::Text("bufferIndex: %d, offset: %d, countBufferIndex: %d, countOffset: %d, maxDrawCount: %d, stride: %d", marker->bufferIndex, marker->offset, marker->countBufferIndex, marker->countOffset, marker->maxDrawCount, marker->stride);
                }
                    break;
                case util::FunctionMarker::DrawIndexedIndirect: {
                    const auto marker = reinterpret_cast<util::DrawIndexedIndirect*>(markerData.data());
                    ImGui::Text("bufferIndex: %d, offset: %d, drawCount: %d, stride: %d", marker->bufferIndex, marker->offset, marker->drawCount, marker->stride);
                }
                    break;
                case util::FunctionMarker::DrawIndexedIndirectCount: {
                    const auto marker = reinterpret_cast<util::DrawIndexedIndirectCount*>(markerData.data());
                    ImGui::Text("bufferIndex: %d, offset: %d, countBufferIndex: %d, countOffset: %d, maxDrawCount: %d, stride: %d", marker->bufferIndex, marker->offset, marker->countBufferIndex, marker->countOffset, marker->maxDrawCount, marker->stride);
                }
                    break;
                case util::FunctionMarker::MeshTasks: {
                    const auto marker = reinterpret_cast<util::MeshTasks*>(markerData.data());
                    ImGui::Text("x: %d, y: %d, z: %d", marker->x, marker->y, marker->z);
                }
                    break;
                case util::FunctionMarker::MeshTasksIndirect: {
                    const auto marker = reinterpret_cast<util::MeshTasksIndirect*>(markerData.data());
                    ImGui::Text("bufferIndex: %d, offset: %d, drawCount: %d", marker->bufferIndex, marker->offset, marker->drawCount);
                }
                    break;
                case util::FunctionMarker::MeshTasksIndirectCount: {
                    const auto marker = reinterpret_cast<util::MeshTasksIndirectCount*>(markerData.data());
                    ImGui::Text("bufferIndex: %d, offset: %d, countBufferIndex: %d, countOffset: %d, maxDrawCount: %d, stride: %d", marker->bufferIndex, marker->offset, marker->countBufferIndex, marker->countOffset, marker->maxDrawCount, marker->stride);
                }
                    break;
                case util::FunctionMarker::Dispatch: {
                    const auto marker = reinterpret_cast<util::Dispatch*>(markerData.data());
                    ImGui::Text("x: %d, y: %d, z: %d", marker->x, marker->y, marker->z);
                }
                    break;
                case util::FunctionMarker::DispatchIndirect: {
                    const auto marker = reinterpret_cast<util::DispatchIndirect*>(markerData.data());
                    ImGui::Text("bufferIndex: %d, offset: %d", marker->bufferIndex, marker->offset);
                }
                    break;
            }

                ImGui::TreePop();
            }

            ImGui::PopID();
        }
    }
    ImGui::End();
}
