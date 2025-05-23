#include "Canta/ImGuiContext.h"
#include <Canta/Device.h>
#include <Canta/util.h>
#include <Canta/SDLWindow.h>
#include <backends/imgui_impl_sdl2.h>
#include <imnodes.h>

const char* vertexGLSL = R"(
#version 450 core
layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aUV;
layout(location = 2) in vec4 aColor;

layout(push_constant) uniform uPushConstant {
    vec2 uScale;
    vec2 uTranslate;
} pc;

out gl_PerVertex {
    vec4 gl_Position;
};

layout(location = 0) out struct {
    vec4 Color;
    vec2 UV;
} Out;

void main()
{
    Out.Color = aColor;
    Out.UV = aUV;
    gl_Position = vec4(aPos * pc.uScale + pc.uTranslate, 0, 1);
}
)";

const char* fragmentGLSL = R"(
#version 450 core

#extension GL_EXT_nonuniform_qualifier : enable

layout(location = 0) out vec4 fColor;

layout (set = 0, binding = 0) uniform sampler bindlessSamplers[];
layout (set = 0, binding = 1) uniform texture2D bindlessSampledImages[];

layout(location = 0) in struct {
    vec4 Color;
    vec2 UV;
} In;

layout(push_constant) uniform uPushConstant {
    layout(offset = 16) int samplerIndex;
    layout(offset = 20) int sampledIndex;
} pc;

void main()
{
    vec4 colour = texture(sampler2D(bindlessSampledImages[pc.sampledIndex], bindlessSamplers[pc.samplerIndex]), In.UV.st);
    fColor = In.Color * colour;
}
)";

auto canta::ImGuiContext::create(canta::ImGuiContext::CreateInfo info) -> ImGuiContext {
    ImGuiContext context = {};

    context._device = info.device;
    context._sampler = info.device->createSampler({});

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImNodes::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        ImGui::GetStyle().WindowRounding = 0.f;
        ImGui::GetStyle().Colors[ImGuiCol_WindowBg].w = 1.f;
    }

    info.device->immediate([&](auto& cmd) {
        context.createFontsTexture(cmd);
    });

    if (info.window)
        ImGui_ImplSDL2_InitForVulkan(info.window->window());

    context._window = info.window;

    return context;
}

canta::ImGuiContext::ImGuiContext(canta::ImGuiContext &&rhs) noexcept {
    std::swap(_device, rhs._device);
    std::swap(_pipeline, rhs._pipeline);
    std::swap(_pipelineFormat, rhs._pipelineFormat);
    std::swap(_vertexBuffer, rhs._vertexBuffer);
    std::swap(_indexBuffer, rhs._indexBuffer);
    std::swap(_sampler, rhs._sampler);
    std::swap(_fontImage, rhs._fontImage);
    std::swap(_uploadBuffer, rhs._uploadBuffer);
    std::swap(_window, rhs._window);
}

auto canta::ImGuiContext::operator=(canta::ImGuiContext &&rhs) noexcept -> ImGuiContext & {
    std::swap(_device, rhs._device);
    std::swap(_pipeline, rhs._pipeline);
    std::swap(_pipelineFormat, rhs._pipelineFormat);
    std::swap(_vertexBuffer, rhs._vertexBuffer);
    std::swap(_indexBuffer, rhs._indexBuffer);
    std::swap(_sampler, rhs._sampler);
    std::swap(_fontImage, rhs._fontImage);
    std::swap(_uploadBuffer, rhs._uploadBuffer);
    std::swap(_window, rhs._window);
    return *this;
}

void canta::ImGuiContext::beginFrame() {
    if (_window)
        ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();
}

void canta::ImGuiContext::render(ImDrawData *drawData, canta::CommandBuffer &commandBuffer, Format format) {
    if (!drawData)
        return;
    i32 width = drawData->DisplaySize.x * drawData->FramebufferScale.x;
    i32 height = drawData->DisplaySize.y * drawData->FramebufferScale.y;
    if (width <= 0 || height <= 0)
        return;

    if (drawData->TotalVtxCount > 0) {
        auto vertexSize = drawData->TotalVtxCount * sizeof(ImDrawVert);
        auto indexSize = drawData->TotalIdxCount * sizeof(ImDrawIdx);

        if (!_vertexBuffer || _vertexBuffer->size() < vertexSize) {
            _vertexBuffer = _device->createBuffer({
                .size = static_cast<u32>(vertexSize * 2),
                .usage = BufferUsage::VERTEX,
                .type = MemoryType::STAGING,
                .persistentlyMapped = true,
                .name = "imgui_vertex_buffer"
            });
        }
        if (!_indexBuffer || _indexBuffer->size() < indexSize) {
            _indexBuffer = _device->createBuffer({
                .size = static_cast<u32>(indexSize * 2),
                .usage = BufferUsage::INDEX,
                .type = MemoryType::STAGING,
                .persistentlyMapped = true,
                .name = "imgui_index_buffer"
            });
        }

        u32 vertexOffset = 0;
        u32 indexOffset = 0;
        for (i32 i = 0; i < drawData->CmdListsCount; i++) {
            const ImDrawList* drawList = drawData->CmdLists[i];
            vertexOffset += _vertexBuffer->data(std::span<const u8>(reinterpret_cast<const u8*>(drawList->VtxBuffer.Data), drawList->VtxBuffer.Size * sizeof(ImDrawVert)), vertexOffset);
            indexOffset += _indexBuffer->data(std::span<const u8>(reinterpret_cast<const u8*>(drawList->IdxBuffer.Data), drawList->IdxBuffer.Size * sizeof(ImDrawIdx)), indexOffset);
        }
    }

    setupRenderState(drawData, commandBuffer, width, height, format);

    ImVec2 clipOffset = drawData->DisplayPos;
    ImVec2 clipScale = drawData->FramebufferScale;

    u32 vertexOffset = 0;
    u32 indexOffset = 0;
    for (i32 i = 0; i < drawData->CmdListsCount; i++) {
        const ImDrawList* drawList = drawData->CmdLists[i];
        for (i32 j = 0; j < drawList->CmdBuffer.Size; j++) {
            const ImDrawCmd* cmd = &drawList->CmdBuffer[j];
            if (cmd->UserCallback != nullptr) {
                if (cmd->UserCallback == ImDrawCallback_ResetRenderState)
                    setupRenderState(drawData, commandBuffer, width, height, format);
                else
                    cmd->UserCallback(drawList, cmd);
            } else {
                ImVec2 clipMin((cmd->ClipRect.x - clipOffset.x) * clipScale.x, (cmd->ClipRect.y - clipOffset.y) * clipScale.y);
                ImVec2 clipMax((cmd->ClipRect.z - clipOffset.x) * clipScale.x, (cmd->ClipRect.w - clipOffset.y) * clipScale.y);

                if (clipMin.x < 0.0f) { clipMin.x = 0.0f; }
                if (clipMin.y < 0.0f) { clipMin.y = 0.0f; }
                if (clipMax.x > width) { clipMax.x = (float)width; }
                if (clipMax.y > height) { clipMax.y = (float)height; }
                if (clipMax.x <= clipMin.x || clipMax.y <= clipMin.y)
                    continue;

                commandBuffer.setScissor({ static_cast<u32>(clipMax.x - clipMin.x), static_cast<u32>(clipMax.y - clipMin.y) }, { static_cast<i32>(clipMin.x), static_cast<i32>(clipMin.y) });

                struct Push {
                    i32 samplerIndex;
                    i32 sampledIndex;
                };
                commandBuffer.pushConstants(ShaderStage::FRAGMENT, Push{
                    .samplerIndex = _sampler.index(),
                    .sampledIndex = (i32)(u64)cmd->TextureId
                }, sizeof(f32) * 4);

                commandBuffer.draw(cmd->ElemCount, 1, cmd->IdxOffset + indexOffset, cmd->VtxOffset + vertexOffset, 0, true);
            }
        }
        vertexOffset += drawList->VtxBuffer.Size;
        indexOffset += drawList->IdxBuffer.Size;
    }
}

bool canta::ImGuiContext::createFontsTexture(canta::CommandBuffer &commandBuffer) {
    ImGuiIO& io = ImGui::GetIO();

    unsigned char* pixels;
    int width, height;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
    size_t upload_size = width * height * 4 * sizeof(char);

    VkResult err;

    // Create the Image:
    _fontImage = _device->createImage({
        .width = static_cast<u32>(width),
        .height = static_cast<u32>(height),
        .format = Format::RGBA8_UNORM,
        .usage = ImageUsage::SAMPLED | ImageUsage::TRANSFER_DST
    });

    // Create the Upload Buffer:
    _uploadBuffer = _device->createBuffer({
        .size = static_cast<u32>(upload_size),
        .usage = BufferUsage::TRANSFER_SRC,
        .type = MemoryType::STAGING,
        .persistentlyMapped = true
    });

    // Upload to Buffer:
    _uploadBuffer->data(std::span<const u8>(pixels, upload_size));

    // Copy to Image:
    auto copyBarrier = ImageBarrier{
        .image = _fontImage,
        .srcStage = PipelineStage::HOST,
        .dstStage = PipelineStage::TRANSFER,
        .srcAccess = Access::NONE,
        .dstAccess = Access::TRANSFER_WRITE,
        .srcLayout = ImageLayout::UNDEFINED,
        .dstLayout = ImageLayout::TRANSFER_DST
    };
    commandBuffer.barrier(copyBarrier);
    commandBuffer.copyBufferToImage({
        .buffer = _uploadBuffer,
        .image = _fontImage,
        .dstLayout = ImageLayout::TRANSFER_DST
    });
    copyBarrier = {
        .image = _fontImage,
        .srcStage = PipelineStage::TRANSFER,
        .dstStage = PipelineStage::FRAGMENT_SHADER,
        .srcAccess = Access::TRANSFER_WRITE,
        .dstAccess = Access::SHADER_READ,
        .srcLayout = ImageLayout::TRANSFER_DST,
        .dstLayout = ImageLayout::SHADER_READ_ONLY
    };
    commandBuffer.barrier(copyBarrier);

    // Store our identifier
    io.Fonts->SetTexID((ImTextureID)(u64)_fontImage->defaultView().index());

    return true;
}

void canta::ImGuiContext::processEvent(void *event) {
    if (_window)
        ImGui_ImplSDL2_ProcessEvent(static_cast<SDL_Event*>(event));
}

void canta::ImGuiContext::setupRenderState(ImDrawData *drawData, canta::CommandBuffer &commandBuffer, i32 width, i32 height, Format format) {
    if (!_pipeline || _pipelineFormat != format) {
        _pipeline = createPipeline(format);
        _pipelineFormat = format;
    }

    commandBuffer.bindPipeline(_pipeline);
    if (drawData->TotalVtxCount > 0) {
        commandBuffer.bindVertexBuffer(_vertexBuffer);
        commandBuffer.bindIndexBuffer(_indexBuffer, 0, sizeof(ImDrawIdx) == 2 ? VK_INDEX_TYPE_UINT16 : VK_INDEX_TYPE_UINT32);
    }

    commandBuffer.setViewport({ static_cast<f32>(width), static_cast<f32>(height) }, { 0, 0 }, false);

    struct Push {
        f32 scale[2] = {};
        f32 translate[2] = {};
    } push;
    push.scale[0] = 2.f / drawData->DisplaySize.x;
    push.scale[1] = 2.f / drawData->DisplaySize.y;
    push.translate[0] = -1 - drawData->DisplayPos.x * push.scale[0];
    push.translate[1] = -1 - drawData->DisplayPos.y * push.scale[1];
    commandBuffer.pushConstants(ShaderStage::VERTEX, push);
}

auto canta::ImGuiContext::createPipeline(canta::Format format) -> PipelineHandle {
    static auto vertexSpirv = util::compileGLSLToSpirv("imgui_vertex", vertexGLSL, ShaderStage::VERTEX).transform_error([](const auto& error) {
        std::printf("%s", error.c_str());
        return error;
    }).value();
    static auto fragmentSpirv = util::compileGLSLToSpirv("imgui_fragment", fragmentGLSL, ShaderStage::FRAGMENT).transform_error([](const auto& error) {
        std::printf("%s", error.c_str());
        return error;
    }).value();


    auto inputBindings = std::vector{
        VertexInputBinding {
            .binding = 0,
            .stride = sizeof(ImDrawVert),
            .inputRate = VK_VERTEX_INPUT_RATE_VERTEX
        }
    };
    auto inputAttributes = std::vector{
        VertexInputAttribute {
            .location = 0,
            .binding = 0,
            .format = Format::RG32_SFLOAT,
            .offset = IM_OFFSETOF(ImDrawVert, pos)
        },
        VertexInputAttribute {
            .location = 1,
            .binding = 0,
            .format = Format::RG32_SFLOAT,
            .offset = IM_OFFSETOF(ImDrawVert, uv)
        },
        VertexInputAttribute {
            .location = 2,
            .binding = 0,
            .format = Format::RGBA8_UNORM,
            .offset = IM_OFFSETOF(ImDrawVert, col)
        },
    };
    auto colourFormats = std::vector{ format };

    return _device->createPipeline({
        .vertex = {
            .module = _device->createShaderModule({
                .spirv = vertexSpirv,
                .stage = ShaderStage::VERTEX
            })
        },
        .fragment = {
            .module = _device->createShaderModule({
                .spirv = fragmentSpirv,
                .stage = ShaderStage::FRAGMENT
            })
        },
        .rasterState = {
            .cullMode = CullMode::NONE,
            .frontFace = FrontFace::CCW,
            .polygonMode = PolygonMode::FILL,
            .lineWidth = 1.f
        },
        .blendState = {
            .blend = true,
            .srcFactor = BlendFactor::SRC_ALPHA,
            .dstFactor = BlendFactor::ONE_MINUS_SRC_ALPHA
        },
        .inputBindings = inputBindings,
        .inputAttributes = inputAttributes,
        .topology = PrimitiveTopology::TRIANGLE_LIST,
        .colourFormats = colourFormats
    });
}

#include <Canta/RenderGraph.h>

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

auto groupToColour(canta::RenderGroup group) -> ImColor {
    f32 hue = std::abs(group.id) * 1.71f;
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

void canta::drawRenderGraph(canta::RenderGraph& renderGraph) {
    static i32 frame = 0;
    if (ImGui::Begin("Render Graph", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse)) {
        if (ImGui::Button("Reload Graph")) {
            frame = 0;
        }
        ImNodes::BeginNodeEditor();

        std::vector<std::pair<i32, i32>> links = {};
        std::vector<i32> validTargets = {};

        ImVec2 pos = ImGui::GetCursorScreenPos();
        ImVec2 originalPos = pos;

        for (i32 i = 0; auto& pass : renderGraph.orderedPasses()) {

            i32 passId = 100 * i++;

            if (frame < 10) {
                ImNodes::SetNodeScreenSpacePos(passId, pos);
                ImNodes::SnapNodeToGrid(passId);
            }

            ImNodes::PushColorStyle(ImNodesCol_TitleBar, queueToColour(pass->getQueue()));
            ImNodes::PushColorStyle(ImNodesCol_NodeBackground, groupToColour(pass->getGroup()));
            ImNodes::BeginNode(passId);
            ImNodes::BeginNodeTitleBar();
            ImGui::Text("%s", pass->name().data());
            ImNodes::EndNodeTitleBar();
            pos.x += ImGui::CalcTextSize(std::format("{}", pass->name().data()).c_str()).x * 1.5;
            pos.y = (i % 2 == 0) ? originalPos.y : 100;

            for (auto& input : pass->inputs()) {
                const auto resource = renderGraph.resources()[input.index].get();

                ImNodes::BeginInputAttribute(passId + input.id);
                ImGui::Text("%s", resource->name.data());
                ImNodes::EndInputAttribute();
                validTargets.push_back(passId + input.id);

                for (auto& barrier : pass->barriers()) {
                    if (barrier.passIndex < 0) continue;
                    if (barrier.index == resource->index) {
                        if (std::find(validTargets.begin(), validTargets.end(), barrier.passIndex * 100 + input.id) != validTargets.end() &&
                            std::find(validTargets.begin(), validTargets.end(), passId + input.id) != validTargets.end()) {
                            links.push_back({barrier.passIndex * 100 + input.id, passId + input.id});
                        }
                    }
                }
            }
            for (auto& output : pass->output()) {
                const auto resource = renderGraph.resources()[output.index].get();

                ImNodes::BeginOutputAttribute(passId + output.id);
                ImGui::Text("%s", resource->name.data());
                ImNodes::EndOutputAttribute();
                validTargets.push_back(passId + output.id);


                for (auto& barrier : pass->barriers()) {
                    if (barrier.passIndex < 0) continue;
                    if (barrier.index == resource->index) {
                        if (std::find(validTargets.begin(), validTargets.end(), barrier.passIndex * 100 + output.id) != validTargets.end() &&
                            std::find(validTargets.begin(), validTargets.end(), passId + output.id) != validTargets.end()) {
                            links.push_back({barrier.passIndex * 100 + output.id, passId + output.id});
                        }
                    }
                }
            }
            ImNodes::EndNode();
            ImNodes::PopColorStyle();
            ImNodes::PopColorStyle();

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

void canta::renderGraphDebugUi(RenderGraph& graph) {
    static i32 selectedPass = -1;
    static i32 selectedResource = -1;
    if (ImGui::Begin("Render Graph Debug")) {

        {
            ImGui::BeginChild("Passes", ImVec2(ImGui::GetContentRegionAvail().x * 0.5f, 150), ImGuiChildFlags_None, ImGuiWindowFlags_HorizontalScrollbar);
            ImGui::Text("Passes");
            ImGui::Separator();
            for (u32 i = 0; i < graph.orderedPasses().size(); i++) {
                auto& pass = graph.orderedPasses()[i];

                if (ImGui::Selectable(pass->name().data(), selectedPass == i))
                    selectedPass = i;
            }
            ImGui::EndChild();
        }
        ImGui::SameLine();
        if (selectedPass > -1) {
            ImGui::BeginChild("Resources", ImVec2(ImGui::GetContentRegionAvail().x, 150), ImGuiChildFlags_None, ImGuiWindowFlags_HorizontalScrollbar);
            ImGui::Text("Pass Resources");
            ImGui::Separator();
            auto& pass = graph.orderedPasses()[selectedPass];
            ImGui::Text("Inputs:");
            for (auto& input : pass->inputs()) {
                auto resource = graph.resources()[input.index].get();
                if (ImGui::Selectable(std::format("\t{}_input", resource->name).c_str(), selectedResource == input.index))
                    selectedResource = input.index;
            }
            ImGui::Text("Outputs:");
            for (auto& output : pass->output()) {
                auto resource = graph.resources()[output.index].get();
                if (ImGui::Selectable(std::format("\t{}_output", resource->name).c_str(), selectedResource == output.index))
                    selectedResource = output.index;
            }
            ImGui::EndChild();
        }
        if (selectedResource > -1) {
            ImGui::BeginChild("Info");
            ImGui::Text("Resource Info");
            ImGui::Separator();
            const auto& resource = graph.resources()[selectedResource];
            ImGui::Text("Name: %s", resource->name.c_str());
            if (resource->type == ResourceType::IMAGE) {
                const auto& image = dynamic_cast<ImageResource*>(resource.get());
                ImGui::Text("Image");
                ImGui::Text("Matches Backbuffer: %b", image->matchesBackbuffer);
                ImGui::Text("Width: %u", image->width);
                ImGui::Text("Height: %u", image->height);
                ImGui::Text("Depth: %u", image->depth);
                ImGui::Text("Mips: %u", image->mipLevels);
                ImGui::Text("Format: %s", formatString(image->format));
                ImGui::Text("Usage: %s", "Unimplemented");
                ImGui::Text("Initial Layout: %s", "Unimplemented");
            } else {
                const auto& buffer = dynamic_cast<BufferResource*>(resource.get());
                ImGui::Text("Buffer");
                ImGui::Text("Size: %u b", buffer->size);
                ImGui::Text("Usage: %u", buffer->usage);
                ImGui::Text("Memory Type: %u", buffer->memoryType);
            }

            ImGui::EndChild();
        }

        ImGui::End();
    }
}