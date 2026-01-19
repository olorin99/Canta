#include "Canta/ImGuiContext.h"
#include <Canta/Device.h>
#include <Canta/PipelineManager.h>
#include <Canta/SDLWindow.h>
#include <backends/imgui_impl_sdl2.h>
#include <imnodes.h>
#include <Ende/filesystem/File.h>


const char* vertexSlang = R"(

struct VSOutput {
    float4 position : SV_Position;
    float2 uv : TEXCOORD;
    float4 colour: COLOUR;
};

[shader("vertex")]
VSOutput main(
    float2 pos : POSITION,
    float2 uv : TEXCOORD,
    float4 colour,
    uniform float2 scale,
    uniform float2 translate,
) {
    VSOutput out;
    out.position = float4(pos * scale + translate, 0, 1);
    out.uv = uv;
    out.colour = colour;
    return out;
}
)";

const char* fragmentSlang = R"(
import canta;

struct VSOutput {
    float4 position : SV_Position;
    float2 uv : TEXCOORD;
    float4 colour: COLOUR;
};

struct Push {
    [[vk::offset(16)]] canta.Sampler sampler;
    [[vk::offset(20)]] canta.Image2D<float4> image;
}

[[vk::push_constant]]
Push push;

[shader("fragment")]
float4 main(
    in VSOutput input,
) {
    float4 colour = push.image.get().Sample(push.sampler.get(), input.uv);
    return input.colour * colour;
}
)";

auto canta::ImGuiContext::create(canta::ImGuiContext::CreateInfo info) -> ImGuiContext {
    ImGuiContext context = {};

    assert(info.device);
    assert(info.pipelineManager);

    context._device = info.device;
    context._pipelineManager = info.pipelineManager;
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

    return _pipelineManager->getPipeline({
        .vertex = {
            .module = _pipelineManager->getShader({
                .slang = vertexSlang,
                .stage = ShaderStage::VERTEX
            }).value()
        },
        .fragment = {
            .module = _pipelineManager->getShader({
                .slang = fragmentSlang,
                .stage = ShaderStage::FRAGMENT
            }).value()
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
    }).value();
}