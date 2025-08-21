#include <Canta/RenderGraphDebugger.h>
#include <imgui/imgui.h>

auto canta::RenderGraphDebugger::create(const CreateInfo &info) -> RenderGraphDebugger {
    RenderGraphDebugger debugger;
    debugger.setRenderGraph(info.renderGraph);
    return debugger;
}


void canta::RenderGraphDebugger::render() {

    if (ImGui::Begin("Render Graph Debug")) {
        {
            ImGui::BeginChild("Passes", ImVec2(ImGui::GetContentRegionAvail().x * 0.5f, 150), ImGuiChildFlags_None, ImGuiWindowFlags_HorizontalScrollbar);
            ImGui::Text("Passes");
            ImGui::Separator();
            for (u32 i = 0; i < _renderGraph->orderedPasses().size(); i++) {
                const auto pass = _renderGraph->orderedPasses()[i];

                const bool isBase = pass->name() == _basePass;
                if (isBase) {
                    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(255, 0, 0, 1));
                }

                switch (pass->getQueue()) {
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
                if (ImGui::Selectable(pass->name().data(), _selectedPass == pass->name())) {
                    _selectedPass = pass->name();
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
            for (auto& input : (*pass)->inputs()) {
                if (const auto resource = _renderGraph->resources()[input.index].get(); ImGui::Selectable(std::format("\t{}_input", resource->name).c_str(), _selectedResource.index == input.index)) {
                    _selectedResource = input;
                }
            }
            ImGui::Text("Outputs:");
            for (auto& output : (*pass)->output()) {
                if (const auto resource = _renderGraph->resources()[output.index].get(); ImGui::Selectable(std::format("\t{}_output", resource->name).c_str(), _selectedResource.index == output.index)) {
                    _selectedResource = output;
                }
            }
            ImGui::EndChild();
        }
        if (_selectedResource.id > -1) {
            ImGui::BeginChild("Info");
            ImGui::Text("Resource Info");
            ImGui::Separator();
            const auto resource = _renderGraph->resources()[_selectedResource.index].get();
            ImGui::Text("Name: %s", resource->name.c_str());
            if (resource->type == ResourceType::IMAGE) {
                const auto& image = dynamic_cast<ImageResource*>(resource);
                ImGui::Text("Image");
                ImGui::Text("Id: %u, Index: %u", _selectedResource.id, _selectedResource.index);
                ImGui::Text("Matches Backbuffer: %b", image->matchesBackbuffer);
                ImGui::Text("Width: %u", image->width);
                ImGui::Text("Height: %u", image->height);
                ImGui::Text("Depth: %u", image->depth);
                ImGui::Text("Mips: %u", image->mipLevels);
                ImGui::Text("Format: %s", formatString(image->format));
                ImGui::Text("Usage: %s", "Unimplemented");
                ImGui::Text("Initial Layout: %s", "Unimplemented");

                _copyResource = true;

            } else {
                const auto& buffer = dynamic_cast<BufferResource*>(resource);
                ImGui::Text("Buffer");
                ImGui::Text("Id: %u, Index: %u", _selectedResource.id, _selectedResource.index);
                ImGui::Text("Size: %u b", buffer->size);
                ImGui::Text("Usage: %u", buffer->usage);
                ImGui::Text("Memory Type: %u", buffer->memoryType);
            }

            ImGui::EndChild();
        }

        ImGui::End();
    }

}

void canta::RenderGraphDebugger::debug() {
    const auto pass = _renderGraph->getPass(_selectedPass);
    if (!pass) return;

    const auto index = ImageIndex{.id = _selectedResource.id, .index = _selectedResource.index};
    if (!(*pass)->isOutput(index)) return;

    const auto alias = (*pass)->aliasImageOutput(index).value();
    auto resource = dynamic_cast<ImageResource*>(_renderGraph->resources()[index.index].get());
    auto transientImage = _renderGraph->addImage({
        .matchesBackbuffer = resource->matchesBackbuffer,
        .width = resource->width,
        .height = resource->height,
        .format = resource->format,
        .name = "transient"
    });

    _renderGraph->addPass({.name = "dummy", .type = canta::PassType::TRANSFER, .manualPipeline = true})
        .addTransferRead(alias)
        .addDummyWrite(index)
        .addTransferWrite(transientImage)
        .setExecuteFunction([alias, transientImage] (auto& cmd, auto& graph) {
            auto src = graph.getImage(alias);
            auto dst = graph.getImage(transientImage);

            auto blitInfo = CommandBuffer::BlitInfo{
                .src = src,
                .dst = dst,
                .srcLayout = ImageLayout::TRANSFER_SRC,
                .dstLayout = ImageLayout::TRANSFER_DST,
                .filter = Filter::LINEAR,
            };
            cmd.blit(blitInfo);
        });

    if (_basePass.empty() || _baseResourceId < 0 || _basePass == _selectedPass) return;

    const auto basePass = _renderGraph->getPass(_basePass);
    if (!basePass) return;

    const auto baseResource = ImageIndex{ .id = _baseResourceId, .index = _baseResourceIndex };
    if (!(*basePass)->isOutput(baseResource)) return;

    const auto baseAlias = (*basePass)->aliasImageOutput(baseResource).value();

    _renderGraph->addPass({.name = "composite", .type = canta::PassType::TRANSFER, .manualPipeline = true})
        .addDummyRead(baseAlias)
        .addTransferRead(transientImage)
        .addTransferWrite(baseResource)
        .setExecuteFunction([transientImage, baseResource, resource] (auto& cmd, auto& graph) {
            auto src = graph.getImage(transientImage);
            auto dst = graph.getImage(baseResource);

            auto width = resource->width;
            auto height = resource->height;

            auto blitInfo = CommandBuffer::BlitInfo{
                .src = src,
                .srcSize = {
                    static_cast<i32>(width / 2), static_cast<i32>(height / 2), 1
                },
                .srcOffset = {
                    static_cast<i32>(width / 2), 0, 0
                },
                .dst = dst,
                .dstSize = {
                    static_cast<i32>(width / 2), static_cast<i32>(height / 2), 1
                },
                .dstOffset = {
                    // static_cast<i32>(width / 2), static_cast<i32>(height / 2), 0
                    static_cast<i32>(0), static_cast<i32>(0), 0
                },
                .srcLayout = ImageLayout::TRANSFER_SRC,
                .dstLayout = ImageLayout::TRANSFER_DST,
                .filter = Filter::LINEAR,
            };
            cmd.blit(blitInfo);
        });
}
