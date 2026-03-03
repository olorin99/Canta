#include <ranges>
#include <Canta/RenderGraphV2.h>

#include "Canta/ImGuiContext.h"

constexpr auto defaultPassStage(const canta::V2::RenderPass::Type type) -> canta::PipelineStage {
    switch (type) {
        case canta::V2::RenderPass::Type::GRAPHICS:
            return canta::PipelineStage::ALL_GRAPHICS;
        case canta::V2::RenderPass::Type::COMPUTE:
            return canta::PipelineStage::COMPUTE_SHADER;
        case canta::V2::RenderPass::Type::TRANSFER:
            return canta::PipelineStage::TRANSFER;
        case canta::V2::RenderPass::Type::HOST:
            return canta::PipelineStage::HOST;
    }
    return canta::PipelineStage::ALL_COMMANDS;
}

constexpr auto checkPassStageMatch(const canta::V2::RenderPass::Type type, const canta::PipelineStage stage) -> bool {
    switch (type) {
        case canta::V2::RenderPass::Type::GRAPHICS:
            switch (stage) {
            case canta::PipelineStage::TOP:
                    return true;
            }
            break;
        case canta::V2::RenderPass::Type::COMPUTE:
            switch (stage) {
            case canta::PipelineStage::COMPUTE_SHADER:
                    return true;
            }
            break;
        case canta::V2::RenderPass::Type::TRANSFER:
            switch (stage) {
            case canta::PipelineStage::COMPUTE_SHADER:
                    return true;
            }
            break;
        case canta::V2::RenderPass::Type::HOST:
            if (stage != canta::PipelineStage::HOST) return false;
            break;
    }
    return false;
}

auto canta::V2::mapGraphErrorToRenderGraphError(const ende::graph::Error error) -> RenderGraphError {
    switch (error) {
        case ende::graph::Error::IS_CYCLICAL:
            return RenderGraphError::IS_CYCLICAL;
        default:
            return RenderGraphError::IS_CYCLICAL;
    }
}


auto canta::V2::RenderPass::clone() -> RenderPass {
    return *this;
}

void canta::V2::RenderPass::unpack(PushData &dst, i32 &i, const BufferIndex &index) {
    _deferredPushConstants.push_back(DeferredPushConstant{
        .type = 0,
        .value = index,
        .offset = i
    });
    i += sizeof(u64);
    dst.size += sizeof(u64);
    assert(i <= 128);
}

void canta::V2::RenderPass::unpack(PushData &dst, i32 &i, const ImageIndex &index) {
    _deferredPushConstants.emplace_back(DeferredPushConstant{
        .type = 1,
        .value = index,
        .offset = i,
    });
    i += sizeof(u32);
    dst.size += sizeof(u32);
    assert(i <= 128);
}

void canta::V2::RenderPass::unpack(PushData &dst, i32 &i, const BufferHandle &handle) {
    const auto addr = handle->address();
    unpack(dst, i, addr);
}

void canta::V2::RenderPass::unpack(PushData &dst, i32 &i, const ImageHandle &handle) {
    const auto id = handle->defaultView().index();
    unpack(dst, i, id);
}

void unpack(canta::V2::RenderPass::PushData &dst, i32 &i, const canta::BufferHandle &handle) {
    const auto addr = handle->address();
    auto* data = reinterpret_cast<const u8*>(&addr);
    for (auto j = 0; j < sizeof(addr); j++) {
        dst.data[i + j] = data[j];
    }
    i += sizeof(addr);
    // dst.size += sizeof(addr);
}

void unpack(canta::V2::RenderPass::PushData &dst, i32 &i, const canta::ImageHandle &handle) {
    const auto id = handle->defaultView().index();
    auto* data = reinterpret_cast<const u8*>(&id);
    for (auto j = 0; j < sizeof(id); j++) {
        dst.data[i + j] = data[j];
    }
    i += sizeof(id);
    // dst.size += sizeof(id);
}

auto canta::V2::RenderPass::setPipeline(PipelineHandle pipeline) -> RenderPass & {
    _pipeline = pipeline;
    return *this;
}

// auto canta::V2::RenderPass::setCallback(const std::function<void(CommandBuffer&, RenderGraph&, const PushData&)> &callback) -> RenderPass & {
//     _callback = [callback] (CommandBuffer& commands, RenderGraph& graph, const PushData& data) -> std::expected<bool, RenderGraphError> {
//         callback(commands, graph, data);
//         return true;
//     };
//     return *this;
// }

auto canta::V2::RenderPass::setCallback(const std::function<std::expected<bool, RenderGraphError>(CommandBuffer&, RenderGraph&, const PushData&)> &callback) -> RenderPass & {
    _callback = callback;
    return *this;
}

auto canta::V2::RenderPass::run(RenderGraph &graph, CommandBuffer &commands) const -> std::expected<bool, RenderGraphError> {
    const auto pushData = TRY(resolvePushConstants(graph, _pushData, _deferredPushConstants));
    if (_pipeline)
        commands.bindPipeline(_pipeline);
    try {
        TRY(_callback(commands, graph, pushData));
    } catch (std::exception& e) {
        return std::unexpected(RenderGraphError::PASS_RUN);
    }
    return true;
}

auto canta::V2::RenderPass::inputResourceIndex(const u32 i) const -> u32 {
    const auto input = inputs[i];
    if (std::holds_alternative<BufferIndex>(input)) {
        return std::get<BufferIndex>(input).index;
    } else {
        return std::get<ImageIndex>(input).index;
    }
}

auto canta::V2::RenderPass::outputResourceIndex(u32 i) const -> u32 {
    const auto output = outputs[i];
    if (std::holds_alternative<ImageIndex>(output)) {
        return std::get<ImageIndex>(output).index;
    } else {
        return std::get<BufferIndex>(output).index;
    }
}

auto canta::V2::RenderPass::resolvePushConstants(RenderGraph& graph, PushData data, const std::span<const DeferredPushConstant> deferredConstants) -> std::expected<PushData, RenderGraphError> {
    for (auto& deferredConstant : deferredConstants) {
        auto offset = deferredConstant.offset;
        if (deferredConstant.type == 0)
            ::unpack(data, offset, TRY(graph.getBuffer(std::get<BufferIndex>(deferredConstant.value))));
        else
            ::unpack(data, offset, TRY(graph.getImage(std::get<ImageIndex>(deferredConstant.value))));
    }
    return data;
}

void canta::V2::RenderPass::mergeAccesses() {
    for (i32 accessIndex = 0; accessIndex < _accesses.size(); accessIndex++) {
        auto& access = _accesses[accessIndex];

        for (i32 nextIndex = accessIndex + 1; nextIndex < _accesses.size(); nextIndex++) {
            auto& nextAccess = _accesses[nextIndex];

            if (access.index == nextAccess.index) {

                access.access = access.access | nextAccess.access;
                access.stage = std::min(access.stage, nextAccess.stage);
                access.layout = nextAccess.layout;

                _accesses.erase(_accesses.begin() + nextIndex--);
            }
        }

    }
}

canta::V2::PassBuilder::PassBuilder(RenderGraph *graph, const u32 index) : _graph(graph), _vertexIndex(index) {}

auto canta::V2::PassBuilder::pass() -> RenderPass& {
    return _graph->getVertices()[_vertexIndex];
}

auto canta::V2::PassBuilder::pipeline(const PipelineHandle &pipeline) -> PassBuilder & {
    pass().setPipeline(pipeline);
    return *this;
}

auto canta::V2::PassBuilder::setManualPipeline(const bool state) -> PassBuilder & {
    pass().setManualPipeline(state);
    return *this;
}

auto canta::V2::PassBuilder::read(const BufferIndex index, const Access access, const PipelineStage stage) -> std::expected<BufferInfo, RenderGraphError> {
    pass().inputs.emplace_back(index);
    pass()._accesses.emplace_back(ResourceAccess{
        .id = index.id,
        .index = index.index,
        .access = access,
        .stage = stage
    });
    return _graph->getBufferInfo(index);
}

auto canta::V2::PassBuilder::read(const ImageIndex index, const Access access, const PipelineStage stage, const ImageLayout layout) -> std::expected<ImageInfo, RenderGraphError> {
    pass().inputs.emplace_back(index);
    pass()._accesses.emplace_back(ResourceAccess{
        .id = index.id,
        .index = index.index,
        .access = access,
        .stage = stage,
        .layout = layout,
    });
    return _graph->getImageInfo(index);
}

auto canta::V2::PassBuilder::write(const BufferIndex index, const Access access, const PipelineStage stage) -> std::expected<BufferInfo, RenderGraphError> {
    const auto alias = _graph->alias(index);
    pass().outputs.emplace_back(alias);
    pass()._accesses.emplace_back(ResourceAccess{
        .id = alias.id,
        .index = alias.index,
        .access = access,
        .stage = stage
    });
    return _graph->getBufferInfo(index);
}

auto canta::V2::PassBuilder::write(const ImageIndex index, const Access access, const PipelineStage stage, const ImageLayout layout) -> std::expected<ImageInfo, RenderGraphError> {
    const auto alias = _graph->alias(index);
    pass().outputs.emplace_back(alias);
    pass()._accesses.emplace_back(ResourceAccess{
        .id = alias.id,
        .index = alias.index,
        .access = access,
        .stage = stage,
        .layout = layout,
    });
    return _graph->getImageInfo(index);
}

void canta::V2::PassBuilder::unpack(RenderPass::PushData &dst, i32 &i, const BufferIndex &index) {
    pass()._deferredPushConstants.push_back(RenderPass::DeferredPushConstant{
        .type = 0,
        .value = index,
        .offset = i
    });
    i += sizeof(u64);
    dst.size += sizeof(u64);
    assert(i <= 128);
}

void canta::V2::PassBuilder::unpack(RenderPass::PushData &dst, i32 &i, const ImageIndex &index) {
    pass()._deferredPushConstants.emplace_back(RenderPass::DeferredPushConstant{
        .type = 1,
        .value = index,
        .offset = i,
    });
    i += sizeof(u32);
    dst.size += sizeof(u32);
    assert(i <= 128);
}

void canta::V2::PassBuilder::unpack(RenderPass::PushData &dst, i32 &i, const BufferHandle &handle) {
    const auto addr = handle->address();
    unpack(dst, i, addr);
}

void canta::V2::PassBuilder::unpack(RenderPass::PushData &dst, i32 &i, const ImageHandle &handle) {
    const auto id = handle->defaultView().index();
    unpack(dst, i, id);
}

void canta::V2::PassBuilder::unpack(RenderPass::PushData& dst, i32& i, const Read<ImageIndex>& image) {
    addStorageImageRead(image.resource);
    unpack(dst, i, image.resource);
}

void canta::V2::PassBuilder::unpack(RenderPass::PushData& dst, i32& i, const Read<BufferIndex>& buffer) {
    addStorageBufferRead(buffer.resource);
    unpack(dst, i, buffer.resource);
}

void canta::V2::PassBuilder::unpack(RenderPass::PushData& dst, i32& i, const Write<ImageIndex>& image) {
    addStorageImageWrite(image.resource);
    unpack(dst, i, image.resource);
}

void canta::V2::PassBuilder::unpack(RenderPass::PushData& dst, i32& i, const Write<BufferIndex>& buffer) {
    addStorageBufferWrite(buffer.resource);
    unpack(dst, i, buffer.resource);
}

auto canta::V2::PassBuilder::setCallback(const std::function<std::expected<bool, RenderGraphError>(CommandBuffer &, RenderGraph &, const RenderPass::PushData &)> &callback) -> PassBuilder& {
    pass().setCallback(callback);
    return *this;
}


auto canta::V2::PassBuilder::addColourRead(const ImageIndex index) -> PassBuilder& {
    auto info = *read(index, Access::COLOUR_READ, PipelineStage::COLOUR_OUTPUT, ImageLayout::COLOUR_ATTACHMENT);

    auto dimensions = pass().dimensions();
    pass()._dimensions = {
        std::max(dimensions.x(), info.width),
        std::max(dimensions.y(), info.height),
    };

    info.usage |= ImageUsage::COLOUR_ATTACHMENT;
    _graph->updateImageInfo(index, info);
    return *this;
}

auto canta::V2::PassBuilder::addColourWrite(const ImageIndex index, const ClearValue& clearColour) -> PassBuilder& {
    auto info = *write(index, Access::COLOUR_READ | Access::COLOUR_WRITE, PipelineStage::COLOUR_OUTPUT, ImageLayout::COLOUR_ATTACHMENT);
    pass()._colourAttachments.push_back({
        .index = index.index,
        .layout = ImageLayout::COLOUR_ATTACHMENT,
        .clearColor = clearColour,
    });
    auto dimensions = pass().dimensions();
    pass()._dimensions = {
        std::max(dimensions.x(), info.width),
        std::max(dimensions.y(), info.height),
    };

    info.usage |= ImageUsage::COLOUR_ATTACHMENT;
    _graph->updateImageInfo(index, info);
    return *this;
}

auto canta::V2::PassBuilder::addDepthRead(const ImageIndex index) -> PassBuilder& {
    auto info = *read(index, Access::DEPTH_STENCIL_READ, PipelineStage::EARLY_FRAGMENT_TEST | PipelineStage::LATE_FRAGMENT_TEST | PipelineStage::FRAGMENT_SHADER, ImageLayout::DEPTH_STENCIL_ATTACHMENT);
    pass()._depthAttachment = {
        .index = index.index,
        .layout = ImageLayout::DEPTH_STENCIL_ATTACHMENT,
    };
    auto dimensions = pass().dimensions();
    pass()._dimensions = {
        std::max(dimensions.x(), info.width),
        std::max(dimensions.y(), info.height),
    };

    info.usage |= ImageUsage::DEPTH_STENCIL_ATTACHMENT;
    _graph->updateImageInfo(index, info);
    return *this;
}

auto canta::V2::PassBuilder::addDepthWrite(const ImageIndex index, const ClearValue& clearColour) -> PassBuilder& {
    auto info = *write(index, Access::DEPTH_STENCIL_READ | Access::DEPTH_STENCIL_WRITE, PipelineStage::EARLY_FRAGMENT_TEST | PipelineStage::LATE_FRAGMENT_TEST | PipelineStage::FRAGMENT_SHADER, ImageLayout::DEPTH_STENCIL_ATTACHMENT);
    pass()._depthAttachment = {
        .index = index.index,
        .layout = ImageLayout::DEPTH_STENCIL_ATTACHMENT,
        .clearColor = clearColour,
    };

    info.usage |= ImageUsage::DEPTH_STENCIL_ATTACHMENT;
    _graph->updateImageInfo(index, info);
    return *this;
}

auto canta::V2::PassBuilder::addStorageImageRead(const ImageIndex index, PipelineStage stage) -> PassBuilder & {
    if (stage == PipelineStage::NONE)
        stage = defaultPassStage(pass()._type);
    auto info = *read(index, stage == PipelineStage::HOST ? Access::HOST_READ : Access::SHADER_READ, stage, ImageLayout::GENERAL);

    info.usage |= ImageUsage::STORAGE;
    _graph->updateImageInfo(index, info);
    return *this;
}

auto canta::V2::PassBuilder::addStorageImageWrite(const ImageIndex index, PipelineStage stage) -> PassBuilder & {
    if (stage == PipelineStage::NONE)
        stage = defaultPassStage(pass()._type);
    auto info = *write(index, stage == PipelineStage::HOST ? Access::HOST_READ | Access::HOST_WRITE : Access::SHADER_READ | Access::SHADER_WRITE, stage, ImageLayout::GENERAL);

    info.usage |= ImageUsage::STORAGE;
    _graph->updateImageInfo(index, info);
    return *this;
}

auto canta::V2::PassBuilder::addStorageBufferRead(const BufferIndex index, PipelineStage stage) -> PassBuilder & {
    if (stage == PipelineStage::NONE)
        stage = defaultPassStage(pass()._type);
    auto info = *read(index, stage == PipelineStage::HOST ? Access::HOST_READ : Access::SHADER_READ, stage);

    info.usage |= BufferUsage::STORAGE;
    _graph->updateBufferInfo(index, info);
    return *this;
}

auto canta::V2::PassBuilder::addStorageBufferWrite(const BufferIndex index, PipelineStage stage) -> PassBuilder & {
    if (stage == PipelineStage::NONE)
        stage = defaultPassStage(pass()._type);
    auto info = *write(index, stage == PipelineStage::HOST ? Access::HOST_READ | Access::HOST_WRITE : Access::SHADER_READ | Access::SHADER_WRITE, stage);

    info.usage |= BufferUsage::STORAGE;
    _graph->updateBufferInfo(index, info);
    return *this;
}

auto canta::V2::PassBuilder::addSampledRead(const ImageIndex index, PipelineStage stage) -> PassBuilder& {
    if (stage == PipelineStage::NONE)
        stage = defaultPassStage(pass()._type);
    auto info = *read(index, Access::SHADER_READ, stage, ImageLayout::SHADER_READ_ONLY);

    info.usage |= ImageUsage::SAMPLED;
    _graph->updateImageInfo(index, info);
    return *this;
}

auto canta::V2::PassBuilder::addBlitRead(const ImageIndex index) -> PassBuilder& {
    auto info = *read(index, Access::TRANSFER_READ, PipelineStage::TRANSFER, ImageLayout::TRANSFER_SRC);

    info.usage |= ImageUsage::TRANSFER_SRC;
    _graph->updateImageInfo(index, info);
    return *this;
}

auto canta::V2::PassBuilder::addBlitWrite(const ImageIndex index) -> PassBuilder& {
    auto info = *write(index, Access::TRANSFER_READ | Access::TRANSFER_WRITE, PipelineStage::TRANSFER, ImageLayout::TRANSFER_DST);

    info.usage |= ImageUsage::TRANSFER_DST;
    _graph->updateImageInfo(index, info);
    return *this;
}

auto canta::V2::PassBuilder::addTransferRead(const ImageIndex index) -> PassBuilder& {
    auto info = *read(index, Access::TRANSFER_READ, PipelineStage::TRANSFER, ImageLayout::TRANSFER_SRC);

    info.usage |= ImageUsage::TRANSFER_SRC;
    _graph->updateImageInfo(index, info);
    return *this;
}

auto canta::V2::PassBuilder::addTransferWrite(const ImageIndex index) -> PassBuilder& {
    auto info = *write(index, Access::TRANSFER_READ | Access::TRANSFER_WRITE, PipelineStage::TRANSFER, ImageLayout::TRANSFER_DST);

    info.usage |= ImageUsage::TRANSFER_DST;
    _graph->updateImageInfo(index, info);
    return *this;
}

auto canta::V2::PassBuilder::addTransferRead(const BufferIndex index) -> PassBuilder& {
    auto info = *read(index, Access::TRANSFER_READ, PipelineStage::TRANSFER);

    info.usage |= BufferUsage::TRANSFER_SRC;
    _graph->updateBufferInfo(index, info);
    return *this;
}

auto canta::V2::PassBuilder::addTransferWrite(const BufferIndex index) -> PassBuilder& {
    auto info = *write(index, Access::TRANSFER_READ | Access::TRANSFER_WRITE, PipelineStage::TRANSFER);

    info.usage |= BufferUsage::TRANSFER_DST;
    _graph->updateBufferInfo(index, info);
    return *this;
}

auto canta::V2::PassBuilder::addIndirectRead(const BufferIndex index) -> PassBuilder& {
    auto info = *read(index, Access::INDIRECT, PipelineStage::DRAW_INDIRECT);

    info.usage |= BufferUsage::INDIRECT;
    _graph->updateBufferInfo(index, info);
    return *this;
}

auto canta::V2::PassBuilder::addDummyRead(const ImageIndex index) -> PassBuilder& {
    read(index, Access::NONE, PipelineStage::NONE, ImageLayout::GENERAL);
    return *this;
}

auto canta::V2::PassBuilder::addDummyWrite(const ImageIndex index) -> PassBuilder& {
    write(index, Access::NONE, PipelineStage::NONE, ImageLayout::GENERAL);
    return *this;
}

auto canta::V2::PassBuilder::addDummyRead(const BufferIndex index) -> PassBuilder& {
    read(index, Access::NONE, PipelineStage::NONE);
    return *this;
}

auto canta::V2::PassBuilder::addDummyWrite(const BufferIndex index) -> PassBuilder& {
    write(index, Access::NONE, PipelineStage::NONE);
    return *this;
}

canta::V2::ComputePass::ComputePass(RenderGraph *graph, const u32 index) : PassBuilder(graph, index) {}

auto canta::V2::ComputePass::addStorageImageRead(const ImageIndex index) -> ComputePass & {
    PassBuilder::addStorageImageRead(index, PipelineStage::COMPUTE_SHADER);
    return *this;
}

auto canta::V2::ComputePass::addStorageImageWrite(const ImageIndex index) -> ComputePass & {
    PassBuilder::addStorageImageWrite(index, PipelineStage::COMPUTE_SHADER);
    return *this;
}

auto canta::V2::ComputePass::addStorageBufferRead(const BufferIndex index) -> ComputePass & {
    PassBuilder::addStorageBufferRead(index, PipelineStage::COMPUTE_SHADER);
    return *this;
}

auto canta::V2::ComputePass::addStorageBufferWrite(const BufferIndex index) -> ComputePass & {
    PassBuilder::addStorageBufferWrite(index, PipelineStage::COMPUTE_SHADER);
    return *this;
}

auto canta::V2::ComputePass::addSampledRead(const ImageIndex index) -> ComputePass& {
    PassBuilder::addSampledRead(index, PipelineStage::COMPUTE_SHADER);
    return *this;
}

auto canta::V2::ComputePass::dispatchThreads(u32 x, u32 y, u32 z) -> ComputePass & {
    pass().setCallback([x, y, z] (auto& cmd, auto& graph, const RenderPass::PushData& push) {
        cmd.pushConstants(ShaderStage::COMPUTE, { push.data.data(), push.size }, 0);
        cmd.dispatchThreads(x, y, z);
        return true;
    });
    return *this;
}

auto canta::V2::ComputePass::dispatchWorkgroups(u32 x, u32 y, u32 z) -> ComputePass & {
    pass().setCallback([x, y, z] (auto& cmd, auto& graph, const RenderPass::PushData& push) {
        cmd.pushConstants(ShaderStage::COMPUTE, { push.data.data(), push.size }, 0);
        cmd.dispatchWorkgroups(x, y, z);
        return true;
    });
    return *this;
}

auto canta::V2::ComputePass::dispatchIndirect(BufferHandle commandBuffer, u32 offset) -> ComputePass & {
    pass().setCallback([commandBuffer, offset] (auto& cmd, auto& graph, const RenderPass::PushData& push) {
        cmd.pushConstants(ShaderStage::COMPUTE, { push.data.data(), push.size }, 0);
        cmd.dispatchIndirect(commandBuffer, offset);
        return true;
    });
    return *this;
}


canta::V2::GraphicsPass::GraphicsPass(RenderGraph *graph, const u32 index) : PassBuilder(graph, index) {}

auto canta::V2::GraphicsPass::addColourRead(const ImageIndex index) -> GraphicsPass& {
    PassBuilder::addColourRead(index);
    return *this;
}

auto canta::V2::GraphicsPass::addColourWrite(const ImageIndex index, const ClearValue& clearColour) -> GraphicsPass& {
    PassBuilder::addColourWrite(index, clearColour);
    return *this;
}

auto canta::V2::GraphicsPass::addDepthRead(const ImageIndex index) -> GraphicsPass& {
    PassBuilder::addDepthRead(index);
    return *this;
}

auto canta::V2::GraphicsPass::addDepthWrite(const ImageIndex index, const ClearValue& clearColour) -> GraphicsPass& {
    PassBuilder::addDepthWrite(index, clearColour);
    return *this;
}

auto canta::V2::GraphicsPass::addStorageImageRead(const ImageIndex index, const PipelineStage stage) -> GraphicsPass & {
    PassBuilder::addStorageImageRead(index, stage);
    return *this;
}

auto canta::V2::GraphicsPass::addStorageImageWrite(const ImageIndex index, const PipelineStage stage) -> GraphicsPass & {
    PassBuilder::addStorageImageWrite(index, stage);
    return *this;
}

auto canta::V2::GraphicsPass::addStorageBufferRead(const BufferIndex index, const PipelineStage stage) -> GraphicsPass & {
    PassBuilder::addStorageBufferRead(index, stage);
    return *this;
}

auto canta::V2::GraphicsPass::addStorageBufferWrite(const BufferIndex index, const PipelineStage stage) -> GraphicsPass & {
    PassBuilder::addStorageBufferWrite(index, stage);
    return *this;
}

auto canta::V2::GraphicsPass::addSampledRead(const ImageIndex index, const PipelineStage stage) -> GraphicsPass& {
    PassBuilder::addSampledRead(index, stage);
    return *this;
}

auto canta::V2::GraphicsPass::draw(u32 count, u32 instanceCount, u32 firstVertex, u32 firstIndex, u32 firstInstance, bool indexed) -> GraphicsPass & {
    auto dimensions = pass().dimensions();
    pass().setCallback([dimensions, count, instanceCount, firstVertex, firstIndex, firstInstance, indexed] (auto& cmd, auto& graph, const RenderPass::PushData& push) {
        cmd.setViewport({ static_cast<f32>(dimensions.x()), static_cast<f32>(dimensions.y()) });
        cmd.pushConstants(ShaderStage::VERTEX | ShaderStage::FRAGMENT, { push.data.data(), push.size }, 0);
        cmd.draw(count, instanceCount, firstVertex, firstIndex, firstInstance, indexed);
        return true;
    });
    return *this;
}

auto canta::V2::GraphicsPass::drawIndirect(BufferHandle commands, u32 offset, u32 drawCount, bool indexed, u32 stride) -> GraphicsPass & {
    auto dimensions = pass().dimensions();
    pass().setCallback([dimensions, commands, offset, drawCount, indexed, stride] (auto& cmd, auto& graph, const RenderPass::PushData& push) {
        cmd.setViewport({ static_cast<f32>(dimensions.x()), static_cast<f32>(dimensions.y()) });
        cmd.pushConstants(ShaderStage::VERTEX | ShaderStage::FRAGMENT, { push.data.data(), push.size }, 0);
        cmd.drawIndirect(commands, offset, drawCount, indexed, stride);
        return true;
    });
    return *this;
}

auto canta::V2::GraphicsPass::drawIndirectCount(BufferHandle commands, u32 offset, BufferHandle countBuffer, u32 countOffset, bool indexed, u32 stride) -> GraphicsPass & {
    auto dimensions = pass().dimensions();
    pass().setCallback([dimensions, commands, offset, countBuffer, countOffset, indexed, stride] (auto& cmd, auto& graph, const RenderPass::PushData& push) {
        cmd.setViewport({ static_cast<f32>(dimensions.x()), static_cast<f32>(dimensions.y()) });
        cmd.pushConstants(ShaderStage::VERTEX | ShaderStage::FRAGMENT, { push.data.data(), push.size }, 0);
        cmd.drawIndirectCount(commands, offset, countBuffer, countOffset, indexed, stride);
        return true;
    });
    return *this;
}

auto canta::V2::GraphicsPass::drawMeshTasksThreads(u32 x, u32 y, u32 z) -> GraphicsPass & {
    auto dimensions = pass().dimensions();
    pass().setCallback([dimensions, x, y, z] (auto& cmd, auto& graph, const RenderPass::PushData& push) {
        cmd.setViewport({ static_cast<f32>(dimensions.x()), static_cast<f32>(dimensions.y()) });
        cmd.pushConstants(ShaderStage::MESH | ShaderStage::FRAGMENT, { push.data.data(), push.size }, 0);
        cmd.drawMeshTasksThreads(x, y, z);
        return true;
    });
    return *this;
}

auto canta::V2::GraphicsPass::drawMeshTasksWorkgroups(u32 x, u32 y, u32 z) -> GraphicsPass & {
    auto dimensions = pass().dimensions();
    pass().setCallback([dimensions, x, y, z] (auto& cmd, auto& graph, const RenderPass::PushData& push) {
        cmd.setViewport({ static_cast<f32>(dimensions.x()), static_cast<f32>(dimensions.y()) });
        cmd.pushConstants(ShaderStage::MESH | ShaderStage::FRAGMENT, { push.data.data(), push.size }, 0);
        cmd.drawMeshTasksWorkgroups(x, y, z);
        return true;
    });
    return *this;
}

auto canta::V2::GraphicsPass::drawMeshTasksIndirect(BufferHandle commands, u32 offset, u32 drawCount, u32 stride) -> GraphicsPass & {
    auto dimensions = pass().dimensions();
    pass().setCallback([dimensions, commands, offset, drawCount, stride] (auto& cmd, auto& graph, const RenderPass::PushData& push) {
        cmd.setViewport({ static_cast<f32>(dimensions.x()), static_cast<f32>(dimensions.y()) });
        cmd.pushConstants(ShaderStage::MESH | ShaderStage::FRAGMENT, { push.data.data(), push.size }, 0);
        cmd.drawMeshTasksIndirect(commands, offset, drawCount, stride);
        return true;
    });
    return *this;
}

auto canta::V2::GraphicsPass::drawMeshTasksIndirectCount(BufferHandle commands, u32 offset, BufferHandle countBuffer, u32 countOffset, u32 stride) -> GraphicsPass & {
    auto dimensions = pass().dimensions();
    pass().setCallback([dimensions, commands, offset, countBuffer, countOffset, stride] (auto& cmd, auto& graph, const RenderPass::PushData& push) {
        cmd.setViewport({ static_cast<f32>(dimensions.x()), static_cast<f32>(dimensions.y()) });
        cmd.pushConstants(ShaderStage::MESH | ShaderStage::FRAGMENT, { push.data.data(), push.size }, 0);
        cmd.drawMeshTasksIndirectCount(commands, offset, countBuffer, countOffset, stride);
        return true;
    });
    return *this;
}

auto canta::V2::GraphicsPass::blit(const ImageIndex src, const ImageIndex dst, const Filter filter) -> GraphicsPass& {
    addTransferRead(src);
    addTransferRead(dst);
    addTransferWrite(dst);
    pass().setCallback([src, dst, filter] (auto& cmd, auto& graph, const auto& push) -> std::expected<bool, RenderGraphError> {
        cmd.blit({
            .src = TRY(graph.getImage(src)),
            .dst = TRY(graph.getImage(dst)),
            .srcLayout = ImageLayout::TRANSFER_SRC,
            .dstLayout = ImageLayout::TRANSFER_DST,
            .filter = filter,
        });
        return true;
    });
    return *this;
}

auto canta::V2::GraphicsPass::imgui(ImGuiContext &context, ImageIndex image) -> std::expected<ImageIndex, RenderGraphError> {
    addColourRead(image);
    addColourWrite(image);
    setManualPipeline(true);
    pass().setCallback([&context, image] (auto& cmd, auto& graph, const auto& push) {
        auto imageInfo = graph.getImageInfo(image);
        context.render(ImGui::GetDrawData(), cmd, imageInfo->format);
        return true;
    });
    return output<ImageIndex>();
}

canta::V2::TransferPass::TransferPass(RenderGraph *graph, const u32 index) : PassBuilder(graph, index) {}

auto canta::V2::TransferPass::addTransferRead(const BufferIndex index) -> TransferPass& {
    PassBuilder::addTransferRead(index);
    return *this;
}

auto canta::V2::TransferPass::addTransferWrite(const BufferIndex index) -> TransferPass& {
    PassBuilder::addTransferWrite(index);
    return *this;
}

auto canta::V2::TransferPass::addTransferRead(const ImageIndex index) -> TransferPass& {
    PassBuilder::addTransferRead(index);
    return *this;
}

auto canta::V2::TransferPass::addTransferWrite(const ImageIndex index) -> TransferPass& {
    PassBuilder::addTransferWrite(index);
    return *this;
}

auto canta::V2::TransferPass::copy(BufferIndex src, BufferIndex dst, u32 srcOffset, u32 dstOffset, u32 size) -> std::expected<BufferIndex, RenderGraphError> {
    addTransferRead(src);
    addTransferWrite(dst);
    pass().setCallback([src, dst, srcOffset, dstOffset, size] (auto& cmd, auto& graph, const auto& push) -> std::expected<bool, RenderGraphError> {
        cmd.copyBuffer({
            .src = TRY(graph.getBuffer(src)),
            .dst = TRY(graph.getBuffer(dst)),
            .srcOffset = srcOffset,
            .dstOffset = dstOffset,
            .size = size,
        });
        return true;
    });
    return output<BufferIndex>();
}

auto canta::V2::TransferPass::copy(BufferIndex src, ImageIndex dst, ImageCopy info) -> std::expected<ImageIndex, RenderGraphError> {
    addTransferRead(src);
    addTransferWrite(dst);
    pass().setCallback([src, dst, info] (auto& cmd, auto& graph, const auto& push) -> std::expected<bool, RenderGraphError> {
        auto copyInfo = info;
        if (copyInfo.size == 0 || copyInfo.dimensions.x() == 0 || copyInfo.dimensions.y() == 0 || copyInfo.dimensions.z() == 0) {
            auto bufferInfo = TRY(graph.getBufferInfo(src));
            auto imageInfo = TRY(graph.getImageInfo(dst));

            copyInfo.dimensions = {
                imageInfo.image->width(),
                imageInfo.image->height(),
                imageInfo.image->depth(),
            };
            copyInfo.size = bufferInfo.size;
        }

        cmd.copyBufferToImage({
            .buffer = TRY(graph.getBuffer(src)),
            .image = TRY(graph.getImage(dst)),
            .dstLayout = copyInfo.layout,
            .dstDimensions = copyInfo.dimensions,
            .dstOffsets = copyInfo.offsets,
            .dstMipLevel = copyInfo.mipLevel,
            .dstLayer = copyInfo.layer,
            .dstLayerCount = copyInfo.layerCount,
            .size = copyInfo.size,
            .srcOffset = copyInfo.offset,
        });
        return true;
    });
    return output<ImageIndex>();
}

auto canta::V2::TransferPass::copy(ImageIndex src, BufferIndex dst, ImageCopy info) -> std::expected<BufferIndex, RenderGraphError> {
    addTransferRead(src);
    addTransferWrite(dst);
    pass().setCallback([src, dst, info] (auto& cmd, auto& graph, const auto& push) -> std::expected<bool, RenderGraphError> {
        auto copyInfo = info;
        if (copyInfo.size == 0 || copyInfo.dimensions.x() == 0 || copyInfo.dimensions.y() == 0 || copyInfo.dimensions.z() == 0) {
            auto bufferInfo = TRY(graph.getBufferInfo(dst));
            auto imageInfo = TRY(graph.getImageInfo(src));

            copyInfo.dimensions = {
                imageInfo.image->width(),
                imageInfo.image->height(),
                imageInfo.image->depth(),
            };
            copyInfo.size = bufferInfo.size;
        }

        cmd.copyImageToBuffer({
            .buffer = TRY(graph.getBuffer(dst)),
            .image = TRY(graph.getImage(src)),
            .dstLayout = copyInfo.layout,
            .dstDimensions = copyInfo.dimensions,
            .dstOffsets = copyInfo.offsets,
            .dstMipLevel = copyInfo.mipLevel,
            .dstLayer = copyInfo.layer,
            .dstLayerCount = copyInfo.layerCount,
            .size = copyInfo.size,
            .srcOffset = copyInfo.offset,
        });
        return true;
    });
    return output<BufferIndex>();
}

auto canta::V2::TransferPass::clear(const ImageIndex index, const ClearValue &value) -> std::expected<ImageIndex, RenderGraphError> {
    addTransferWrite(index);
    pass().setCallback([value, index] (auto& cmd, auto& graph, const auto& push) -> std::expected<bool, RenderGraphError> {
        cmd.clearImage(TRY(graph.getImage(index)), ImageLayout::TRANSFER_DST, value);
        return true;
    });
    return output<ImageIndex>();
}

auto canta::V2::TransferPass::clear(BufferIndex index, u32 value, u32 offset, u32 size) -> std::expected<BufferIndex, RenderGraphError> {
    addTransferWrite(index);
    pass().setCallback([index, value, offset, size] (auto& cmd, auto& graph, const auto& push) -> std::expected<bool, RenderGraphError> {
        cmd.clearBuffer(TRY(graph.getBuffer(index)), value, offset, size);
        return true;
    });
    return output<BufferIndex>();
}

canta::V2::HostPass::HostPass(RenderGraph *graph, const u32 index) : PassBuilder(graph, index) {}

auto canta::V2::HostPass::read(const BufferIndex index) -> HostPass & {
    PassBuilder::read(index, Access::HOST_READ, PipelineStage::HOST);
    return *this;
}

auto canta::V2::HostPass::read(const ImageIndex index) -> HostPass & {
    PassBuilder::read(index, Access::HOST_READ, PipelineStage::HOST, ImageLayout::GENERAL);
    return *this;
}

auto canta::V2::HostPass::write(const BufferIndex index) -> HostPass & {
    PassBuilder::write(index, Access::HOST_READ, PipelineStage::HOST);
    return *this;
}

auto canta::V2::HostPass::write(const ImageIndex index) -> HostPass & {
    PassBuilder::write(index, Access::HOST_READ, PipelineStage::HOST, ImageLayout::GENERAL);
    return *this;
}

auto canta::V2::HostPass::setCallback(const std::function<void(RenderGraph &)> &callback) -> HostPass & {
    pass().setCallback([callback] (auto& cmd, auto& graph, const RenderPass::PushData& push) -> std::expected<bool, RenderGraphError> {
        callback(graph);
        return true;
    });
    return *this;
}

auto canta::V2::HostPass::upload(BufferIndex dst, std::span<const u8> data) -> std::expected<BufferIndex, RenderGraphError> {
    addStorageBufferWrite(dst);
    auto dstInfo = TRY(_graph->getBufferInfo(dst));
    dstInfo.type = MemoryType::STAGING;
    TRY(_graph->updateBufferInfo(dst, dstInfo));
    setCallback([dst, data] (auto& graph) {
        auto buffer = *graph.getBuffer(dst);
        buffer->data(data);
    });
    return output<BufferIndex>();
}

auto canta::V2::HostPass::readback(const BufferIndex src, std::span<u8> data) -> std::expected<BufferIndex, RenderGraphError> {
    addStorageBufferRead(src);
    addStorageBufferWrite(src);
    auto srcInfo = TRY(_graph->getBufferInfo(src));
    srcInfo.type = MemoryType::READBACK;
    TRY(_graph->updateBufferInfo(src, srcInfo));
    setCallback([src, data] (auto& graph) {
        auto buffer = *graph.getBuffer(src);
        auto mapped = buffer->map();
        std::memcpy(data.data(), mapped.address(), data.size());
    });
    return output<BufferIndex>();
}

canta::V2::PresentPass::PresentPass(RenderGraph *graph, const u32 index) : PassBuilder(graph, index) {}

auto canta::V2::PresentPass::acquire(Swapchain* swapchain) -> std::expected<ImageIndex, RenderGraphError> {
    auto swapIndex = _graph->addImage({
        .width = swapchain->width(),
        .height = swapchain->height(),
        .format = swapchain->format(),
        .swapchainImage = true,
        .name = "swapchain_index",
    });
    write(swapIndex, Access::MEMORY_READ | Access::MEMORY_WRITE, PipelineStage::BOTTOM, ImageLayout::UNDEFINED);
    pass().setCallback([swapIndex, swapchain] (auto& cmd, auto& graph, const auto& push) -> std::expected<bool, RenderGraphError> {
        auto image = TRY(swapchain->acquire(graph.timeline()).transform_error([] (VulkanError error) {
            return RenderGraphError::DEVICE_ERROR;
        }));
        auto info = TRY(graph.getImageInfo(swapIndex));
        info.image = image;
        TRY(graph.updateImageInfo(swapIndex, info));
        return true;
    });

    return output<ImageIndex>();
}

//TODO: will need to handle special. chain timeline semaphore with acquire/present semaphores
auto canta::V2::PresentPass::present(Swapchain* swapchain, const ImageIndex index) -> std::expected<ImageIndex, RenderGraphError> {
    auto info = TRY(_graph->getImageInfo(index));
    if (!info.swapchainImage)
        return std::unexpected(RenderGraphError::INVALID_RESOURCE);

    read(index, Access::MEMORY_READ, PipelineStage::BOTTOM, ImageLayout::PRESENT);
    write(index, Access::MEMORY_READ | Access::MEMORY_WRITE, PipelineStage::BOTTOM, ImageLayout::PRESENT);
    auto timeline = _graph->timeline();
    pass().setCallback([timeline, swapchain] (auto& cmd, auto& graph, const auto& push) -> std::expected<bool, RenderGraphError> {
        const uint* waits = reinterpret_cast<const uint*>(push.data.data());
        u32 count = waits[0];

        std::vector<SemaphoreHandle> semaphores = {};
        for (u32 i = 1; i < count + 1; ++i) {
            auto index = waits[i];
            auto queueType = static_cast<QueueType>(index);

            if (queueType == QueueType::NONE)
                semaphores.emplace_back(graph.timeline());
            else
                semaphores.emplace_back(graph.device()->queue(queueType)->timeline());
        }

        TRY(swapchain->present(semaphores).transform_error([] (VulkanError error) {
            // _graph->device->logger().error("Present error: {}", static_cast<u32>(error));
            return RenderGraphError::DEVICE_ERROR;
        }));
        return true;
    });
    return output<ImageIndex>();
}


auto canta::V2::RenderGraph::create(CreateInfo info) -> std::expected<RenderGraph, RenderGraphError> {
    auto graph = RenderGraph();
    graph._device = info.device;
    graph._threadPool = info.threadPool;
    graph._multiQueue = info.multiQueue;
    if (!graph._threadPool)
        graph._threadPool = std::make_shared<ende::thread::ThreadPool>();

    for (auto& poolGroup : graph._commandPools) {
        poolGroup[0] = *info.device->createCommandPool({
            .queueType = QueueType::GRAPHICS,
        });
        poolGroup[1] = *info.device->createCommandPool({
            .queueType = QueueType::COMPUTE,
        });
        poolGroup[2] = *info.device->createCommandPool({
            .queueType = QueueType::TRANSFER,
        });
    }

    graph._cpuTimeline = TRY(info.device->createSemaphore({
        .initialValue = 0,
        .name = "cpu_timeline"
    }).transform_error([] (VulkanError e) { return RenderGraphError::DEVICE_ERROR; }));

    return graph;
}

auto canta::V2::RenderGraph::addBuffer(BufferInfo info) -> BufferIndex {
    _resources.emplace_back(info);
    auto& edge = addEdge<BufferIndex>();
    std::get<BufferIndex>(edge).index = _resources.size() - 1;
    return std::get<BufferIndex>(edge);
}

auto canta::V2::RenderGraph::addImage(ImageInfo info) -> ImageIndex {
    _resources.emplace_back(info);
    auto& edge = addEdge<ImageIndex>();
    std::get<ImageIndex>(edge).index = _resources.size() - 1;
    return std::get<ImageIndex>(edge);
}

template<class... Ts> struct overload : Ts... { using Ts::operator()...; };

auto canta::V2::RenderGraph::alias(BufferIndex index) -> BufferIndex {
    auto& edge = addEdge<BufferIndex>();
    std::visit(overload{
        [index] (BufferIndex& buffer){ buffer.index = index.index; },
            [index] (ImageIndex& image) { static_assert("unreachable"); }
    }, edge);
    return std::get<BufferIndex>(edge);
}

auto canta::V2::RenderGraph::alias(ImageIndex index) -> ImageIndex {
    auto& edge = addEdge<ImageIndex>();
    std::visit(overload{
        [index] (BufferIndex& buffer){ static_assert("unreachable"); },
            [index] (ImageIndex& image) { image.index = index.index; },
    }, edge);
    return std::get<ImageIndex>(edge);
}

auto canta::V2::RenderGraph::duplicate(const BufferIndex index) -> std::expected<BufferIndex, RenderGraphError> {
    auto info = TRY(getBufferInfo(index));
    info.buffer = {};
    info.usage = BufferUsage::STORAGE;
    info.type = MemoryType::DEVICE;
    return addBuffer(info);
}

auto canta::V2::RenderGraph::duplicate(ImageIndex index) -> std::expected<ImageIndex, RenderGraphError> {
    auto info = TRY(getImageInfo(index));
    info.image = {};
    info.usage = ImageUsage::STORAGE;
    info.swapchainImage = false;
    return addImage(info);
}

auto canta::V2::RenderGraph::getBuffer(const BufferIndex index) const -> std::expected<BufferHandle, RenderGraphError> {
    if (index.index >= _resources.size())
        return std::unexpected(RenderGraphError::INVALID_RESOURCE);

    if (const auto resource = _resources[index.index]; std::holds_alternative<BufferInfo>(resource)) {
        auto buffer = std::get<BufferInfo>(resource).buffer;
        if (!buffer)
            return std::unexpected(RenderGraphError::INVALID_RESOURCE);
        return buffer;
    }

    return std::unexpected(RenderGraphError::INVALID_RESOURCE);
}

auto canta::V2::RenderGraph::getImage(const ImageIndex index) const -> std::expected<ImageHandle, RenderGraphError> {
    if (index.index >= _resources.size())
        return std::unexpected(RenderGraphError::INVALID_RESOURCE);

    if (const auto resource = _resources[index.index]; std::holds_alternative<ImageInfo>(resource)) {
        auto image = std::get<ImageInfo>(resource).image;
        if (!image)
            return std::unexpected(RenderGraphError::INVALID_RESOURCE);
        return image;
    }

    return std::unexpected(RenderGraphError::INVALID_RESOURCE);
}

auto canta::V2::RenderGraph::getBufferInfo(const BufferIndex index) const -> std::expected<BufferInfo, RenderGraphError> {
    if (index.index >= _resources.size() || !std::holds_alternative<BufferInfo>(_resources[index.index]))
        return std::unexpected(RenderGraphError::INVALID_RESOURCE);
    return std::get<BufferInfo>(_resources[index.index]);
}

auto canta::V2::RenderGraph::getImageInfo(const ImageIndex index) const -> std::expected<ImageInfo, RenderGraphError> {
    if (index.index >= _resources.size() || !std::holds_alternative<ImageInfo>(_resources[index.index]))
        return std::unexpected(RenderGraphError::INVALID_RESOURCE);
    return std::get<ImageInfo>(_resources[index.index]);
}

auto canta::V2::RenderGraph::updateBufferInfo(const BufferIndex index, const BufferInfo info) -> std::expected<BufferInfo, RenderGraphError> {
    if (index.index >= _resources.size() || !std::holds_alternative<BufferInfo>(_resources[index.index]))
        return std::unexpected(RenderGraphError::INVALID_RESOURCE);
    _resources[index.index] = info;
    return std::get<BufferInfo>(_resources[index.index]);
}

auto canta::V2::RenderGraph::updateImageInfo(const ImageIndex index, const ImageInfo info) -> std::expected<ImageInfo, RenderGraphError> {
    if (index.index >= _resources.size() || !std::holds_alternative<ImageInfo>(_resources[index.index]))
        return std::unexpected(RenderGraphError::INVALID_RESOURCE);
    _resources[index.index] = info;
    return std::get<ImageInfo>(_resources[index.index]);
}

auto canta::V2::RenderGraph::pass(const std::string_view name, const RenderPass::Type type, const PipelineHandle &pipeline) -> PassBuilder {
    auto& pass = addVertex();
    pass._type = type;
    pass._name = name;
    pass.setPipeline(pipeline);
    const auto builder = PassBuilder(this, vertexCount() - 1);
    return builder;
}

auto canta::V2::RenderGraph::compute(const std::string_view name, const PipelineHandle &pipeline) -> ComputePass {
    auto& pass = addVertex();
    pass._type = RenderPass::Type::COMPUTE;
    pass._name = name;
    pass.setPipeline(pipeline);
    const auto builder = ComputePass(this, vertexCount() - 1);
    return builder;
}

auto canta::V2::RenderGraph::graphics(const std::string_view name, const PipelineHandle &pipeline) -> GraphicsPass {
    auto& pass = addVertex();
    pass._type = RenderPass::Type::GRAPHICS;
    pass._name = name;
    pass.setPipeline(pipeline);
    const auto builder = GraphicsPass(this, vertexCount() - 1);
    return builder;
}

auto canta::V2::RenderGraph::transfer(const std::string_view name, const PipelineHandle &pipeline) -> TransferPass {
    auto& pass = addVertex();
    pass._type = RenderPass::Type::TRANSFER;
    pass._name = name;
    pass.setPipeline(pipeline);
    const auto builder = TransferPass(this, vertexCount() - 1);
    return builder;
}

auto canta::V2::RenderGraph::host(const std::string_view name) -> HostPass {
    auto& pass = addVertex();
    pass._type = RenderPass::Type::HOST;
    pass._name = name;
    const auto builder = HostPass(this, vertexCount() - 1);
    return builder;
}

auto canta::V2::RenderGraph::acquire(Swapchain *swapchain) -> std::expected<ImageIndex, RenderGraphError> {
    auto& pass = addVertex();
    pass._type = RenderPass::Type::PRESENT;
    pass._name = "acquire_pass";
    auto builder = PresentPass(this, vertexCount() - 1);
    return builder.acquire(swapchain);
}

auto canta::V2::RenderGraph::present(Swapchain *swapchain, const ImageIndex index) -> std::expected<ImageIndex, RenderGraphError> {
    auto& pass = addVertex();
    pass._type = RenderPass::Type::PRESENT;
    pass._name = "present_pass";
    auto builder = PresentPass(this, vertexCount() - 1);
    return builder.present(swapchain, index);
}

void canta::V2::RenderGraph::setRoot(const BufferIndex index) {
    _rootEdge = index.id;
}

void canta::V2::RenderGraph::setRoot(const ImageIndex index) {
    _rootEdge = index.id;
}

auto canta::V2::RenderGraph::compile() -> std::expected<bool, RenderGraphError> {
    if (_rootEdge < 0) return std::unexpected(RenderGraphError::NO_ROOT);

    auto sorted = TRY(sort(getEdges()[_rootEdge]).transform_error(mapGraphErrorToRenderGraphError));

    std::vector<std::pair<u32, u32>> indices = getResourceIndices(sorted);

    if (_multiQueue) {
        const auto sortedSpan = std::span(sorted.data(), sorted.size());
        const auto dependencyLevels = TRY(buildDependencyLevels(sortedSpan));

        for (auto& level : dependencyLevels) {
            for (auto& passIndex : level) {
                auto& pass = sorted[passIndex];

                switch (pass._type) {
                    case RenderPass::Type::COMPUTE:
                        pass._queueType = level.size() > 1 && passIndex == level.back() ? QueueType::COMPUTE : QueueType::GRAPHICS;
                        break;
                    case RenderPass::Type::GRAPHICS:
                        pass._queueType = QueueType::GRAPHICS;
                        break;
                    case RenderPass::Type::TRANSFER:
                        pass._queueType = QueueType::TRANSFER;
                        break;
                    case RenderPass::Type::HOST:
                        pass._queueType = QueueType::NONE;
                        break;
                    case RenderPass::Type::NONE:
                        return std::unexpected(RenderGraphError::INVALID_PASS);
                    case RenderPass::Type::PRESENT:
                        pass._queueType = QueueType::GRAPHICS;
                        break;
                }
            }
        }
    }


    _orderedPasses = sorted;

    buildBarriers();
    buildResources();
    TRY(buildRenderAttachments());

    return true;
}

inline auto getQueueIndex(const canta::QueueType queue) -> u32 {
    switch (queue) {
        case canta::QueueType::NONE:
            return 3;
        case canta::QueueType::GRAPHICS:
            return 0;
        case canta::QueueType::COMPUTE:
            return 1;
        case canta::QueueType::TRANSFER:
            return 2;
        case canta::QueueType::SPARSE_BINDING:
            break;
        case canta::QueueType::PRESENT:
            break;
    }
    return 0;
}

auto canta::V2::RenderGraph::run(std::span<SemaphorePair> waits, std::span<SemaphorePair> signals, const bool async) -> std::expected<bool, RenderGraphError> {
    _device->updateBindlessDescriptors();
    // for each sorted pass
    // if current queue different than last end current command list and start a new one.
    // new command list waits on timeline from previous queue.
    // host passes get added to the threadpool.

    for (auto& pool : _commandPools[_device->flyingIndex()])
        pool.reset();

    CommandBuffer* currentCommandBuffer = nullptr;
    QueueType currentQueue = QueueType::NONE;

    struct SubmissionInfo {
        i32 commands;
        std::size_t queueIndex;
        std::size_t waitIndex;
        i32 hostIndex;
    };

    std::vector<SubmissionInfo> submissions = {};
    std::vector<std::vector<QueueType>> internalWaits = {};

    std::vector<SemaphorePair> waitHandles = {};
    std::vector<SemaphorePair> signalHandles = {};

    std::array<std::vector<CommandBuffer*>, 4> _queues = {};

    const auto submitCommands = [this, async] (CommandBuffer* commands, const QueueType queueType, std::span<SemaphorePair> waits, std::span<SemaphorePair> signals, const bool final) -> std::expected<bool, RenderGraphError> {
        const auto queue = _device->queue(queueType);

        std::vector<SemaphorePair> passWaits = {};
        passWaits.insert(passWaits.end(), waits.begin(), waits.end());

        std::vector<SemaphorePair> passSignals = {};
        passSignals.emplace_back(queue->timeline(), queue->timeline()->increment());
        passSignals.insert(passSignals.end(), signals.begin(), signals.end());
        if (async && final)
            passSignals.emplace_back(_cpuTimeline, _cpuTimeline->increment());

        TRY(queue->submit({ commands, 1 }, passWaits, passSignals).transform_error([this] (VulkanError error) {
            _device->logger().error("Invalid queue submit: {}", static_cast<u32>(error));
            return RenderGraphError::DEVICE_ERROR;
        }));
        return true;
    };

    for (u32 passIndex = 0; passIndex < _orderedPasses.size(); ++passIndex) {
        if (passIndex == 0)
            waitHandles.insert(waitHandles.end(), waits.begin(), waits.end());
        if (passIndex == _orderedPasses.size() - 1)
            signalHandles.insert(signalHandles.end(), signals.begin(), signals.end());


        auto& pass = _orderedPasses[passIndex];

        if (pass._type == RenderPass::Type::PRESENT) {
            if (currentCommandBuffer) {

                submitBarriers(*currentCommandBuffer, pass._barriers);

                currentCommandBuffer->end();

                TRY(submitCommands(currentCommandBuffer, currentQueue, waitHandles, signalHandles, passIndex == _orderedPasses.size() - 1));
            }

            auto dummyCommands = CommandBuffer();
            u32 count = pass._queueWaits.size();
            std::vector<u32> queueIndices = {};
            for (auto& wait : pass._queueWaits)
                queueIndices.emplace_back(static_cast<u32>(wait.second));

            pass.pushConstants(count, queueIndices);
            TRY(pass.run(*this, dummyCommands));

            currentCommandBuffer = nullptr;
            continue;
        }

        if (pass._type == RenderPass::Type::HOST) {
            if (currentCommandBuffer) {
                currentCommandBuffer->end();

                TRY(submitCommands(currentCommandBuffer, currentQueue, waitHandles, signalHandles, passIndex == _orderedPasses.size() - 1));
            }

            auto semaphores = waitHandles;

            auto cpuValue = _cpuTimeline->increment();
            _threadPool->addJob([this, semaphores, cpuValue, pass] () -> std::expected<bool, RenderGraphError> {
                TRY(_cpuTimeline->wait(cpuValue - 1).transform_error([this] (VulkanError error) -> RenderGraphError {
                        _device->logger().error("Wait on invalid timeline: {}", static_cast<u32>(error));
                        return RenderGraphError::DEVICE_ERROR;
                    }));
                for (auto& wait : semaphores) {
                    TRY(wait.semaphore->wait(wait.value).transform_error([this] (VulkanError error) -> RenderGraphError {
                        _device->logger().error("Wait on invalid timeline: {}", static_cast<u32>(error));
                        return RenderGraphError::DEVICE_ERROR;
                    }));
                }
                auto dummyCommands = CommandBuffer();
                TRY(pass.run(*this, dummyCommands));

                TRY(_cpuTimeline->signal(cpuValue).transform_error([this] (VulkanError error) -> RenderGraphError {
                    _device->logger().error("Signal on invalid timeline: {}", static_cast<u32>(error));
                    return RenderGraphError::DEVICE_ERROR;
                }));
                return false;
            });

            currentCommandBuffer = nullptr;
            continue;
        }

        if (pass._queueType != currentQueue && currentCommandBuffer) {
            currentCommandBuffer->end();

            TRY(submitCommands(currentCommandBuffer, currentQueue, waitHandles, signalHandles, passIndex == _orderedPasses.size() - 1));

            currentCommandBuffer = nullptr;
        }

        if (currentCommandBuffer == nullptr) {
            currentQueue = pass._queueType;
            const auto queueIndex = getQueueIndex(pass._queueType);
            currentCommandBuffer = &_commandPools[_device->flyingIndex()][queueIndex].getBuffer();

            waitHandles.clear();

            currentCommandBuffer->begin();
        }

        submitBarriers(*currentCommandBuffer, pass._barriers);

        if (pass._type == RenderPass::Type::GRAPHICS && (pass._pipeline || pass._manualPipeline)) {
            auto beginInfo = RenderingInfo{};

            beginInfo.size = pass.dimensions();

            for (u32 attachmentIndex = 0; attachmentIndex < pass._colourAttachments.size(); attachmentIndex++) {
                auto& attachment = pass._colourAttachments[attachmentIndex];
                auto& renderingAttachment = pass._renderingColourAttachments[attachmentIndex];

                if (!renderingAttachment.image) {
                    auto resource = _resources[attachment.index];
                    if (!std::holds_alternative<ImageInfo>(resource))
                        return std::unexpected(RenderGraphError::INVALID_RESOURCE);
                    auto image = std::get<ImageInfo>(resource).image;

                    renderingAttachment.image = image;
                }
            }

            beginInfo.colourAttachments = pass._renderingColourAttachments;
            if (pass._depthAttachment.index >= 0) {
                beginInfo.depthAttachment = pass._renderingDepthAttachment;
            }

            currentCommandBuffer->beginRendering(beginInfo);
        }

        for (auto& wait : pass._queueWaits) {
            if (wait.second == QueueType::NONE) {
                waitHandles.emplace_back(_cpuTimeline);
                continue;
            }
            auto waitedQueue = _device->queue(wait.second);
            waitHandles.emplace_back(waitedQueue->timeline());
        }

        TRY(pass.run(*this, *currentCommandBuffer));

        if (pass._type == RenderPass::Type::GRAPHICS && (pass._pipeline || pass._manualPipeline))
            currentCommandBuffer->endRendering();
    }
    if (currentCommandBuffer) {
        currentCommandBuffer->end();

        TRY(submitCommands(currentCommandBuffer, currentQueue, waitHandles, signalHandles, true));
    }

    if (async)
        TRY(_cpuTimeline->wait(_cpuTimeline->value()).transform_error([] (VulkanError error) { return RenderGraphError::DEVICE_ERROR; }));

    return true;
}

void canta::V2::RenderGraph::reset() {
    _resources.clear();
    Graph::reset();
}

auto canta::V2::RenderGraph::getResourceIndices(const std::span<const RenderPass> passes) const -> std::vector<std::pair<u32, u32> > {
    std::vector<std::pair<u32, u32>> indices = {};
    for (u32 i = 0; i < _resources.size(); i++) {
        indices.emplace_back(passes.size(), 0);
    }
    for (u32 passIndex = 0; passIndex < passes.size(); ++passIndex) {
        auto& pass = passes[passIndex];
        for (u32 i = 0; i < pass.inputs.size(); i++) {
            const auto index = pass.inputResourceIndex(i);
            indices[index].first = std::min(passIndex, indices[index].first);
            indices[index].second = std::max(passIndex, indices[index].second);
        }
        for (u32 i = 0; i < pass.outputs.size(); i++) {
            const auto index = pass.outputResourceIndex(i);
            indices[index].first = std::min(passIndex, indices[index].first);
            indices[index].second = std::max(passIndex, indices[index].second);
        }
    }

    return indices;
}

auto canta::V2::RenderGraph::buildDependencyLevels(std::span<const RenderPass> passes) const -> std::expected<std::vector<std::vector<u32>>, RenderGraphError> {
    const auto distances = TRY(ende::graph::longestPath(passes, edgeCount()).transform_error(mapGraphErrorToRenderGraphError));
    if (distances.empty()) return std::unexpected(RenderGraphError::INVALID_PASS_COUNT);

    const auto maxDistance = std::max_element(distances.begin(), distances.end());

    std::vector<std::vector<u32>> levels = {};
    levels.resize(*maxDistance + 1);
    for (u32 passIndex = 0; passIndex < passes.size(); passIndex++) {
        auto& distance = distances[passIndex];
        levels[distance].emplace_back(passIndex);
    }

    return levels;
}

auto compareBuffer(const canta::V2::BufferInfo& info, canta::BufferHandle handle) -> bool {
    return info.size == handle->size() &&
        info.type == handle->type() &&
        (info.usage & handle->usage()) == handle->usage();
}

auto compareImage(const canta::V2::ImageInfo& info, canta::ImageHandle handle) -> bool {
    return info.width == handle->width() &&
        info.height == handle->height() &&
        info.depth == handle->depth() &&
        info.mips == handle->mips() &&
        info.format == handle->format() &&
        (info.usage & handle->usage()) == handle->usage();

}

auto canta::V2::RenderGraph::getNextAccess(const std::span<const RenderPass> passes, const i32 startIndex, const i32 resource) -> Access {
    for (i32 passIndex = startIndex + 1; passIndex < passes.size(); passIndex++) {
        const auto& pass = passes[passIndex];

        for (auto& access : pass._accesses) {
            if (access.index == resource)
                return { passIndex, access };
        }
    }
    return { -1, {} };
}

auto canta::V2::RenderGraph::getCurrAccess(const std::span<const RenderPass> passes, const i32 startIndex, const i32 resource) -> Access {
    const auto& pass = passes[startIndex];

    for (auto& access : pass._accesses) {
        if (access.index == resource)
            return { startIndex, access };
    }
    return { -1, {} };
}

auto canta::V2::RenderGraph::getPrevAccess(const std::span<const RenderPass> passes, const i32 startIndex, const i32 resource) -> Access {
    for (i32 passIndex = startIndex - 1; passIndex > 0; passIndex--) {
        const auto& pass = passes[passIndex];

        for (auto& access : pass._accesses) {
            if (access.index == resource)
                return { passIndex, access };
        }
    }
    return { -1, {
        .index = resource,
        .access = canta::Access::MEMORY_READ | canta::Access::MEMORY_WRITE,
        .stage = PipelineStage::TOP,
        .layout = ImageLayout::UNDEFINED,
    }};
}

void canta::V2::RenderGraph::submitBarriers(CommandBuffer &commands, std::span<const RenderPass::Barrier> barriers) const {
    u32 imageBarrierCount = 0;
    ImageBarrier imageBarriers[barriers.size()];
    u32 bufferBarrierCount = 0;
    BufferBarrier bufferBarriers[barriers.size()];

    for (const auto& barrier : barriers) {
        const auto resource = _resources[barrier.index];
        if (std::holds_alternative<BufferInfo>(resource)) {
            const auto buffer = std::get<BufferInfo>(resource).buffer;

            bufferBarriers[bufferBarrierCount++] = BufferBarrier {
                .buffer = buffer,
                .srcStage = barrier.srcStage,
                .dstStage = barrier.dstStage,
                .srcAccess = barrier.srcAccess,
                .dstAccess = barrier.dstAccess,
            };
        } else {
            const auto imageInfo= std::get<ImageInfo>(resource);
            if (imageInfo.swapchainImage && !imageInfo.image)
                continue;

            const auto image = imageInfo.image;

            imageBarriers[imageBarrierCount++] = ImageBarrier {
                .image = image,
                .srcStage = barrier.srcStage,
                .dstStage = barrier.dstStage,
                .srcAccess = barrier.srcAccess,
                .dstAccess = barrier.dstAccess,
                .srcLayout = barrier.srcLayout,
                .dstLayout = barrier.dstLayout,
            };
        }
    }

    for (u32 barrier = 0; barrier < imageBarrierCount; barrier++)
        commands.barrier(imageBarriers[barrier]);
    for (u32 barrier = 0; barrier < bufferBarrierCount; barrier++)
        commands.barrier(bufferBarriers[barrier]);
}

void canta::V2::RenderGraph::buildBarriers() {
    for (auto& pass : _orderedPasses)
        pass.mergeAccesses();

    for (i32 passIndex = _orderedPasses.size() - 1; passIndex > 0; passIndex--) {
        auto& pass = _orderedPasses[passIndex];

        for (auto& access : pass._accesses) {
            i32 resource = access.index;

            const auto currAccess = getCurrAccess(_orderedPasses, passIndex, resource);
            const auto prevAccess = getPrevAccess(_orderedPasses, passIndex, resource);

            auto barrier = RenderPass::Barrier {
                .index = resource,
                .passIndex = passIndex,
                .srcStage = prevAccess.access.stage,
                .dstStage = currAccess.access.stage,
                .srcAccess = prevAccess.access.access,
                .dstAccess = currAccess.access.access,
                .srcLayout = prevAccess.access.layout,
                .dstLayout = currAccess.access.layout,
            };
            pass._barriers.emplace_back(barrier);

            if (prevAccess.passIndex > -1) {
                auto& prevPass = _orderedPasses[prevAccess.passIndex];
                if (prevPass._queueType != pass._queueType || pass._type == RenderPass::Type::PRESENT) {
                    pass._queueWaits.emplace_back(std::make_pair(prevAccess.passIndex, prevPass._queueType));
                }
                if (prevPass._type == RenderPass::Type::PRESENT)
                    pass._queueWaits.emplace_back(std::make_pair(prevAccess.passIndex, QueueType::NONE));
            }
        }
    }
}

void canta::V2::RenderGraph::buildResources() {
    //TODO: allocate and alias in memory pool

    // const auto sizes = std::views::transform(_resources, [](auto& resource) {
    //     if (std::holds_alternative<BufferInfo>(resource)) {
    //         return std::get<BufferInfo>(resource).size;
    //     } else {
    //         auto& info = std::get<ImageInfo>(resource);
    //         return info.width * info.height * info.depth * formatSize(info.format);
    //     }
    // });
    //
    // const auto size = std::accumulate(sizes.begin(), sizes.end(), 0);
    // if (size > _poolSize) {
    //
    //     VmaPoolCreateInfo createInfo = {};
    //     createInfo.blockSize = size;
    //     createInfo.maxBlockCount = 2;
    //
    //
    //
    //     _poolSize = size;
    // }


    for (auto& resource : _resources) {
        if (std::holds_alternative<BufferInfo>(resource)) {
            auto& bufferInfo = std::get<BufferInfo>(resource);
            if (bufferInfo.buffer)
                continue;

            const auto buffer = _device->createBuffer({
                .size = bufferInfo.size,
                .usage = bufferInfo.usage,
                .type = bufferInfo.type,
                .name = bufferInfo.name,
            });

            bufferInfo.buffer = buffer;

        } else {
            auto& imageInfo = std::get<ImageInfo>(resource);
            if (imageInfo.swapchainImage || (imageInfo.image && compareImage(imageInfo, imageInfo.image)))
                continue;

            const auto image = _device->createImage({
                .width = imageInfo.width,
                .height = imageInfo.height,
                .depth = imageInfo.depth,
                .format = imageInfo.format,
                .mipLevels = imageInfo.mips,
                .usage = imageInfo.usage,
                .name = imageInfo.name,
            });

            imageInfo.image = image;
        }
    }
}

auto canta::V2::RenderGraph::buildRenderAttachments() -> std::expected<bool, RenderGraphError> {
    for (i32 i = 0; i < _orderedPasses.size(); i++) {
        auto& pass = _orderedPasses[i];

        pass._renderingColourAttachments.clear();

        for (const auto& attachment : pass._colourAttachments) {
            const auto resource = _resources[attachment.index];
            if (!std::holds_alternative<ImageInfo>(resource))
                return std::unexpected(RenderGraphError::INVALID_RESOURCE);
            const auto image = std::get<ImageInfo>(resource).image;

            const auto prevAccess = getPrevAccess(_orderedPasses, i, attachment.index);
            const auto nextAccess = getNextAccess(_orderedPasses, i, attachment.index);

            canta::Attachment renderingAttachment = {};
            renderingAttachment.image = image;
            renderingAttachment.imageLayout = attachment.layout;
            renderingAttachment.loadOp = prevAccess.passIndex > -1 ? LoadOp::LOAD : LoadOp::CLEAR;
            renderingAttachment.storeOp = nextAccess.passIndex > -1 ? StoreOp::STORE : StoreOp::NONE;
            renderingAttachment.clearColour = attachment.clearColor;

            pass._renderingColourAttachments.push_back(renderingAttachment);
        }

        if (pass._depthAttachment.index > -1) {
            const auto resource = _resources[pass._depthAttachment.index];
            if (!std::holds_alternative<ImageInfo>(resource))
                return std::unexpected(RenderGraphError::INVALID_RESOURCE);
            const auto image = std::get<ImageInfo>(resource).image;

            const auto prevAccess = getPrevAccess(_orderedPasses, i, pass._depthAttachment.index);
            const auto nextAccess = getNextAccess(_orderedPasses, i, pass._depthAttachment.index);

            canta::Attachment renderingAttachment = {};
            renderingAttachment.image = image;
            renderingAttachment.imageLayout = pass._depthAttachment.layout;
            renderingAttachment.loadOp = prevAccess.passIndex > -1 ? LoadOp::LOAD : LoadOp::CLEAR;
            renderingAttachment.storeOp = nextAccess.passIndex > -1 ? StoreOp::STORE : StoreOp::NONE;
            renderingAttachment.clearColour = pass._depthAttachment.clearColor;

            pass._renderingDepthAttachment = renderingAttachment;
        }

    }

    return true;
}
