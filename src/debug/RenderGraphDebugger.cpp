#include <../../include/Canta/debug/RenderGraphDebugger.h>
#include <imgui/imgui.h>

#include "imnodes.h"

auto canta::RenderGraphDebugger::create(const CreateInfo &info) -> RenderGraphDebugger {
    RenderGraphDebugger debugger;
    debugger.setRenderGraph(info.renderGraph);
    return debugger;
}

auto idToColour(const i32 id) -> ImColor {
    f32 hue = std::abs(id) * 1.71f;
    f32 tmp;
    hue = std::modf(hue, &tmp);
    f32 r = std::abs(hue * 6 - 3) - 1;
    f32 g = 2 - std::abs(hue * 6 - 2);
    f32 b = 2 - std::abs(hue * 6 - 4);
    return IM_COL32(
        std::clamp(r * 255, 0.0f, 255.0f),
        std::clamp(g * 255, 0.0f, 255.0f),
        std::clamp(b * 255, 0.0f, 255.0f),
        255
    );
}

auto queueToColour(const canta::QueueType queue) -> ImColor {
    switch (queue) {
        case canta::QueueType::NONE:
            return IM_COL32(255, 0, 0, 255);
        case canta::QueueType::GRAPHICS:
            return IM_COL32(0, 255, 0, 255);
        case canta::QueueType::COMPUTE:
            return IM_COL32(0, 0, 255, 255);
        case canta::QueueType::TRANSFER:
            return IM_COL32(127, 127, 0, 255);
        default:
            return IM_COL32(255, 255, 255, 255);
    }
}

auto groupToColour(const canta::RenderGroup& group) -> ImColor {
    return idToColour(group.id);
}

struct GetResourceIndex {

    auto operator()(canta::BufferIndex index) const -> i32 { return index.index; }
    auto operator()(canta::ImageIndex index) const -> i32 { return index.index; }

};

void canta::RenderGraphDebugger::drawRenderGraph() {
    static i32 frame = 0;
    if (ImGui::Begin("Render Graph", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse)) {
        if (ImGui::Button("Reload Graph")) {
            frame = 0;
        }

        ImNodes::BeginNodeEditor();

        std::vector<std::pair<i32, i32>> links = {};

        ImVec2 pos = ImGui::GetCursorScreenPos();
        ImVec2 originalPos = pos;

        for (i32 i = 0; const auto& pass : _renderGraph->passes()) {
            const i32 passId = 100 * i;

            if (frame < 10) {
                ImNodes::SetNodeScreenSpacePos(passId, pos);
                ImNodes::SnapNodeToGrid(passId);
            }

            ImNodes::BeginNode(passId);
            ImNodes::BeginNodeTitleBar();
            ImGui::Text("%s: %d, %d", pass.name().data(), i, passId);
            ImNodes::EndNodeTitleBar();

            pos.x += ImGui::CalcTextSize(std::format("{}", pass.name().data()).c_str()).x * 1.5;
            pos.y = (i % 2 == 0) ? originalPos.y : 100;


            ImNodes::PushColorStyle(ImNodesCol_TitleBar, queueToColour(pass.queue()));
            ImNodes::PushColorStyle(ImNodesCol_NodeBackground, groupToColour(pass.group()));

            for (auto& input : pass.inputs) {
                auto index = std::visit(GetResourceIndex{}, input);
                const auto resource = *_renderGraph->getResourceName(index);

                const auto id = *ende::graph::EdgeHelper<RenderGraph::Edge>::getId(input);
                ImNodes::BeginInputAttribute(passId + id);
                ImGui::Text("%s: %d, %d", resource.data(), id, passId + id);
                ImNodes::EndInputAttribute();
            }

            for (auto& output : pass.outputs) {
                auto index = std::visit(GetResourceIndex{}, output);
                const auto resource = *_renderGraph->getResourceName(index);

                const auto id = *ende::graph::EdgeHelper<RenderGraph::Edge>::getId(output);
                ImNodes::BeginOutputAttribute(passId + id);
                ImGui::Text("%s: %d, %d", resource.data(), id, passId + id);
                ImNodes::EndOutputAttribute();

                auto nextAccess = _renderGraph->getNextAccess(i, index);
                if (nextAccess.passIndex < 0)
                    continue;
                links.push_back({ passId + id, nextAccess.passIndex * 100 + nextAccess.access.id });
            }

            ImNodes::EndNode();
            ImNodes::PopColorStyle();
            ImNodes::PopColorStyle();
            i++;
        }

        for (i32 i = 0; i < links.size(); i++) {
            ImNodes::Link(i, links[i].first, links[i].second);
        }


        ImNodes::MiniMap();
        ImNodes::EndNodeEditor();
        frame++;
    }
    ImGui::End();
}


void canta::RenderGraphDebugger::render() {

    if (ImGui::Begin("Render Graph Debug")) {
        {
            ImGui::BeginChild("Passes", ImVec2(ImGui::GetContentRegionAvail().x * 0.5f, 150), ImGuiChildFlags_None, ImGuiWindowFlags_HorizontalScrollbar);
            ImGui::Text("Passes");
            ImGui::Separator();
            for (u32 i = 0; i < _renderGraph->passes().size(); i++) {
                const auto pass = _renderGraph->passes()[i];

                const bool isBase = pass.name() == _basePass;
                if (isBase) {
                    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(255, 0, 0, 1));
                }

                switch (pass.queue()) {
                    case QueueType::GRAPHICS:
                        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 0, 0, 1));
                        break;
                    case QueueType::COMPUTE:
                        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0, 1, 0, 1));
                        break;
                    case QueueType::TRANSFER:
                        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0, 0, 1, 1));
                        break;
                    default:
                        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0, 0, 0, 1));
                        break;
                }
                if (ImGui::Selectable(pass.name().data(), _selectedPass == pass.name())) {
                    _selectedPass = pass.name();
                    _copyResource = false;
                }
                ImGui::PopStyleColor();

                if (isBase) {
                    ImGui::PopStyleColor();
                }
            }
            ImGui::EndChild();
        }
        ImGui::SameLine();
        if (!_selectedPass.empty()) {
            ImGui::BeginChild("Resources", ImVec2(ImGui::GetContentRegionAvail().x, 150), ImGuiChildFlags_None, ImGuiWindowFlags_HorizontalScrollbar);
            ImGui::Text("Pass Resources");
            ImGui::Separator();
            const auto pass = _renderGraph->getPass(_selectedPass);
            if (!pass) {
                ImGui::EndChild();
                ImGui::End();
                return;
            };
            ImGui::Text("Inputs:");
            for (auto& input : pass->inputs) {
                auto index = std::visit(GetResourceIndex{}, input);
                if (const auto resource = _renderGraph->getResourceName(index); ImGui::Selectable(std::format("\t{}_input", resource->data()).c_str(), std::visit(GetResourceIndex{}, _selectedResource) == index)) {
                    _selectedResource = input;
                }
            }
            ImGui::Text("Outputs:");
            for (auto& output : pass->outputs) {
                auto index = std::visit(GetResourceIndex{}, output);
                if (const auto resource = _renderGraph->getResourceName(index); ImGui::Selectable(std::format("\t{}_output", resource->data()).c_str(), std::visit(GetResourceIndex{}, _selectedResource) == index)) {
                    _selectedResource = output;
                }
            }
            ImGui::EndChild();
        }
        const auto id = *ende::graph::EdgeHelper<RenderGraph::Edge>::getId(_selectedResource);
        auto index = std::visit(GetResourceIndex{}, _selectedResource);
        if (id > -1) {
            ImGui::BeginChild("Info");
            ImGui::Text("Resource Info");
            ImGui::Separator();
            const auto resource = *_renderGraph->getResource(index);
            ImGui::Text("Name: %s", resource.name.data());
            if (std::holds_alternative<ImageInfo>(resource.info)) {
                const auto image = _renderGraph->getImageInfo({ .index = index });
                ImGui::Text("Image");
                ImGui::Text("Id: %u, Index: %u", id, index);
                ImGui::Text("Is swapchain: %b", image->swapchainImage);
                ImGui::Text("Width: %u", image->width);
                ImGui::Text("Height: %u", image->height);
                ImGui::Text("Depth: %u", image->depth);
                ImGui::Text("Mips: %u", image->mips);
                ImGui::Text("Format: %s", formatString(image->format));
                ImGui::Text("Usage: %s", "Unimplemented");
                ImGui::Text("Initial Layout: %s", "Unimplemented");

                ImGui::SameLine();

                ImGui::Checkbox("Copy", &_copyResource);
                ImGui::Text("Bounds");

                ImGui::SliderInt("Offset X", &_viewportOffset[0], 0, 1920);
                ImGui::SliderInt("Offset Y", &_viewportOffset[1], 0, 1080);
                ImGui::SliderInt("Width X", &_viewportSize[0], 0, 1920);
                ImGui::SliderInt("Height Y", &_viewportSize[1], 0, 1080);

            } else {
                const auto buffer = _renderGraph->getBufferInfo({ .index = index });
                ImGui::Text("Buffer");
                ImGui::Text("Id: %u, Index: %u", id, index);
                ImGui::Text("Size: %u b", buffer->size);
                ImGui::Text("Usage: %u", buffer->usage);
                ImGui::Text("Memory Type: %u", buffer->type);
            }

            ImGui::EndChild();
        }

        ImGui::End();
    }
}

void canta::RenderGraphDebugger::drawResourceUsage() {
    auto passes = _renderGraph->passes();
    auto& resources = _renderGraph->_resources;
    auto indices = _renderGraph->getResourceIndices(passes);

    if (passes.size() == 0 || resources.size() == 0) return;

    if (ImGui::Begin("Render Graph Resource Usage")) {

        if (ImGui::BeginTable("Resource Layout", passes.size() + 1)) {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            for (u32 passIndex = 0; passIndex < passes.size(); passIndex++) {
                ImGui::TableSetColumnIndex(passIndex + 1);
                ImGui::Text("%s", passes[passIndex].name().data());
            }
            for (u32 resourceIndex = 0; resourceIndex < resources.size(); resourceIndex++) {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);

                auto resourceName = *_renderGraph->getResourceName(resourceIndex);

                ImGui::Text("%s", resourceName.data());

                auto& resourceIndices = indices[resourceIndex];

                for (i32 i = resourceIndices.first; i <= resourceIndices.second; i++) {
                    ImGui::TableSetColumnIndex(i + 1);
                    ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, static_cast<ImU32>(idToColour(resourceIndex)));
                }
            }
        }
        ImGui::EndTable();

    }
    ImGui::End();
}

// void canta::RenderGraphDebugger::debug() {
//     if (!_copyResource) return;
//
//     const auto pass = _renderGraph->passes()[_selectedPass];
//     if (!pass) return;
//
//     const auto index = ImageIndex{.id = _selectedResource.id, .index = _selectedResource.index};
//     if (!(*pass)->isOutput(index)) return;
//
//     const auto alias = (*pass)->aliasImageOutput(index).value();
//     auto resource = _renderGraph->duplicate(index);
//     // auto resource = dynamic_cast<ImageResource*>(_renderGraph->resources()[index.index].get());
//     // auto transientImage = _renderGraph->addImage({
//         // .matchesBackbuffer = resource->matchesBackbuffer,
//         // .width = resource->width,
//         // .height = resource->height,
//         // .format = resource->format,
//         // .name = "transient"
//     // });
//
//     _renderGraph->addPass({.name = "dummy", .type = canta::PassType::TRANSFER, .manualPipeline = true})
//         .addTransferRead(alias)
//         .addDummyWrite(index)
//         .addTransferWrite(transientImage)
//         .setExecuteFunction([alias, transientImage] (auto& cmd, auto& graph) {
//             auto src = graph.getImage(alias);
//             auto dst = graph.getImage(transientImage);
//
//             auto blitInfo = CommandBuffer::BlitInfo{
//                 .src = src,
//                 .dst = dst,
//                 .srcLayout = ImageLayout::TRANSFER_SRC,
//                 .dstLayout = ImageLayout::TRANSFER_DST,
//                 .filter = Filter::LINEAR,
//             };
//             cmd.blit(blitInfo);
//         });
//
//     if (_basePass.empty() || _baseResourceId < 0 || _basePass == _selectedPass) return;
//
//     const auto basePass = _renderGraph->getPass(_basePass);
//     if (!basePass) return;
//
//     const auto baseResource = ImageIndex{ .id = _baseResourceId, .index = _baseResourceIndex };
//     if (!(*basePass)->isOutput(baseResource)) return;
//
//     const auto baseAlias = (*basePass)->aliasImageOutput(baseResource).value();
//
//     auto topLeft = _viewportOffset;
//     auto bottomRight = topLeft + _viewportSize;
//
//     _renderGraph->addPass({.name = "composite", .type = canta::RenderPass::Type::TRANSFER, .manualPipeline = true})
//         .addDummyRead(baseAlias)
//         .addTransferRead(transientImage)
//         .addTransferWrite(baseResource)
//         .setExecuteFunction([transientImage, baseResource, topLeft, bottomRight] (auto& cmd, auto& graph) {
//             auto src = graph.getImage(transientImage);
//             auto dst = graph.getImage(baseResource);
//
//             auto width = topLeft.x();
//             auto height = topLeft.y();
//             auto x = bottomRight.x();
//             auto y = bottomRight.y();
//
//             auto blitInfo = CommandBuffer::BlitInfo{
//                 .src = src,
//                 .srcSize = {
//                     width, height, 1
//                 },
//                 .srcOffset = {
//                     x, y, 0
//                 },
//                 .dst = dst,
//                 .dstSize = {
//                     width, height, 1
//                 },
//                 .dstOffset = {
//                     x, y, 0
//                 },
//                 .srcLayout = ImageLayout::TRANSFER_SRC,
//                 .dstLayout = ImageLayout::TRANSFER_DST,
//                 .filter = Filter::LINEAR,
//             };
//             cmd.blit(blitInfo);
//         });
// }
