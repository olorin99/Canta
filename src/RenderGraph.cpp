#include "Canta/RenderGraph.h"

constexpr auto defaultPassStage(const canta::PassType type) -> canta::PipelineStage {
    switch (type) {
        case canta::PassType::GRAPHICS:
            return canta::PipelineStage::ALL_GRAPHICS;
        case canta::PassType::COMPUTE:
            return canta::PipelineStage::COMPUTE_SHADER;
        case canta::PassType::TRANSFER:
            return canta::PipelineStage::TRANSFER;
        case canta::PassType::HOST:
            return canta::PipelineStage::HOST;
    }
    return canta::PipelineStage::ALL_COMMANDS;
}

constexpr auto checkPassStageMatch(const canta::PassType type, const canta::PipelineStage stage) -> bool {
    switch (type) {
        case canta::PassType::GRAPHICS:
            switch (stage) {
                case canta::PipelineStage::TOP:
                        return true;
                }
            break;
        case canta::PassType::COMPUTE:
            switch (stage) {
                case canta::PipelineStage::COMPUTE_SHADER:
                    return true;
            }
            break;
        case canta::PassType::TRANSFER:
            switch (stage) {
                case canta::PipelineStage::COMPUTE_SHADER:
                    return true;
            }
            break;
        case canta::PassType::HOST:
            if (stage != canta::PipelineStage::HOST) return false;
            break;
    }
    return false;
}

auto canta::RenderPass::addColourWrite(const canta::ImageIndex index, const ClearValue &clearColor) -> RenderPass& {
    assert(index.id >= 0);
    if (const auto resource = writes(index, Access::COLOUR_WRITE | Access::COLOUR_READ,
                               PipelineStage::COLOUR_OUTPUT,
                               ImageLayout::COLOUR_ATTACHMENT)) {
        _colourAttachments.push_back({
            .index = static_cast<i32>(index.index),
            .layout = ImageLayout::COLOUR_ATTACHMENT,
            .clearColor = clearColor
        });
        dynamic_cast<ImageResource*>(resource)->usage |= ImageUsage::COLOUR_ATTACHMENT;
    }
    return *this;
}

auto canta::RenderPass::addColourRead(const canta::ImageIndex index) -> RenderPass& {
    assert(index.id >= 0);
    if (const auto resource = reads(index, Access::COLOUR_READ,
                              PipelineStage::COLOUR_OUTPUT,
                              ImageLayout::COLOUR_ATTACHMENT)) {
        dynamic_cast<ImageResource*>(resource)->usage |= ImageUsage::COLOUR_ATTACHMENT;
    }
    return *this;
}

auto canta::RenderPass::addDepthWrite(const canta::ImageIndex index, const ClearValue& clearColor) -> RenderPass& {
    assert(index.id >= 0);
    if (const auto resource = writes(index, Access::DEPTH_STENCIL_WRITE | Access::DEPTH_STENCIL_READ,
                               PipelineStage::EARLY_FRAGMENT_TEST | PipelineStage::LATE_FRAGMENT_TEST | PipelineStage::FRAGMENT_SHADER,
                               ImageLayout::DEPTH_STENCIL_ATTACHMENT)) {
        _depthAttachment = {
            .index = static_cast<i32>(index.index),
            .layout = ImageLayout::DEPTH_STENCIL_ATTACHMENT,
            .clearColor = clearColor
        };
        dynamic_cast<ImageResource*>(resource)->usage |= ImageUsage::DEPTH_STENCIL_ATTACHMENT;
    }
    return *this;
}

auto canta::RenderPass::addDepthRead(const canta::ImageIndex index) -> RenderPass& {
    assert(index.id >= 0);
    if (const auto resource = reads(index, Access::DEPTH_STENCIL_READ,
                              PipelineStage::EARLY_FRAGMENT_TEST | PipelineStage::LATE_FRAGMENT_TEST | PipelineStage::FRAGMENT_SHADER,
                              ImageLayout::DEPTH_STENCIL_ATTACHMENT)) {
        _depthAttachment = {
            .index = static_cast<i32>(index.index),
            .layout = ImageLayout::DEPTH_STENCIL_ATTACHMENT
        };
        dynamic_cast<ImageResource*>(resource)->usage |= ImageUsage::DEPTH_STENCIL_ATTACHMENT;
    }
    return *this;
}

auto canta::RenderPass::addStorageImageWrite(const canta::ImageIndex index, canta::PipelineStage stage) -> RenderPass& {
    assert(index.id >= 0);
    if (stage == PipelineStage::NONE)
        stage = defaultPassStage(_type);
    if (const auto resource = writes(index, stage == PipelineStage::HOST ? Access::HOST_WRITE | Access::HOST_READ : Access::SHADER_WRITE | Access::SHADER_READ,
                               stage, ImageLayout::GENERAL)) {
        dynamic_cast<ImageResource*>(resource)->usage |= ImageUsage::STORAGE;
    }
    return *this;
}

auto canta::RenderPass::addStorageImageRead(const canta::ImageIndex index, canta::PipelineStage stage) -> RenderPass& {
    assert(index.id >= 0);
    if (stage == PipelineStage::NONE)
        stage = defaultPassStage(_type);
    if (const auto resource = reads(index, stage == PipelineStage::HOST ? Access::HOST_READ : Access::SHADER_READ,
                              stage, ImageLayout::GENERAL)) {
        dynamic_cast<ImageResource*>(resource)->usage |= ImageUsage::STORAGE;
    }
    return *this;
}

auto canta::RenderPass::addStorageBufferWrite(const canta::BufferIndex index, canta::PipelineStage stage) -> RenderPass& {
    assert(index.id >= 0);
    if (stage == PipelineStage::NONE)
        stage = defaultPassStage(_type);
    if (const auto resource = writes(index, stage == PipelineStage::HOST ? Access::HOST_WRITE | Access::HOST_READ : Access::SHADER_WRITE | Access::SHADER_READ,
                               stage)) {
        dynamic_cast<BufferResource*>(resource)->usage |= BufferUsage::STORAGE;
    }
    return *this;
}

auto canta::RenderPass::addStorageBufferRead(const canta::BufferIndex index, canta::PipelineStage stage) -> RenderPass& {
    assert(index.id >= 0);
    if (stage == PipelineStage::NONE)
        stage = defaultPassStage(_type);
    if (const auto resource = reads(index, stage == PipelineStage::HOST ? Access::HOST_READ : Access::SHADER_READ,
                              stage)) {
        dynamic_cast<BufferResource*>(resource)->usage |= BufferUsage::STORAGE;
    }
    return *this;
}

auto canta::RenderPass::addSampledRead(const canta::ImageIndex index, canta::PipelineStage stage) -> RenderPass& {
    assert(index.id >= 0);
    if (stage == PipelineStage::NONE)
        stage = defaultPassStage(_type);
    if (const auto resource = reads(index, Access::SHADER_READ,
                              stage, ImageLayout::SHADER_READ_ONLY)) {
        dynamic_cast<ImageResource*>(resource)->usage |= ImageUsage::SAMPLED;
    }
    return *this;
}

auto canta::RenderPass::addBlitWrite(const canta::ImageIndex index) -> RenderPass& {
    assert(index.id >= 0);
    if (const auto resource = writes(index, Access::TRANSFER_WRITE | Access::TRANSFER_READ,
                               PipelineStage::TRANSFER,
                               ImageLayout::TRANSFER_DST)) {
        dynamic_cast<ImageResource*>(resource)->usage |= ImageUsage::TRANSFER_DST;
    }
    return *this;
}

auto canta::RenderPass::addBlitRead(const canta::ImageIndex index) -> RenderPass& {
    assert(index.id >= 0);
    if (const auto resource = reads(index, Access::TRANSFER_READ,
                              PipelineStage::TRANSFER,
                              ImageLayout::TRANSFER_SRC)) {
        dynamic_cast<ImageResource*>(resource)->usage |= ImageUsage::TRANSFER_SRC;
    }
    return *this;
}

auto canta::RenderPass::addTransferWrite(const canta::ImageIndex index) -> RenderPass& {
    assert(index.id >= 0);
    if (const auto resource = writes(index, Access::TRANSFER_WRITE | Access::TRANSFER_READ,
                               PipelineStage::TRANSFER,
                               ImageLayout::TRANSFER_DST)) {
        dynamic_cast<ImageResource*>(resource)->usage |= ImageUsage::TRANSFER_DST;
    }
    return *this;
}

auto canta::RenderPass::addTransferRead(const canta::ImageIndex index) -> RenderPass& {
    assert(index.id >= 0);
    if (const auto resource = reads(index, Access::TRANSFER_READ,
                              PipelineStage::TRANSFER,
                              ImageLayout::TRANSFER_SRC)) {
        dynamic_cast<ImageResource*>(resource)->usage |= ImageUsage::TRANSFER_SRC;
    }
    return *this;
}

auto canta::RenderPass::addTransferWrite(const canta::BufferIndex index) -> RenderPass& {
    assert(index.id >= 0);
    if (const auto resource = writes(index, Access::TRANSFER_WRITE | Access::TRANSFER_READ,
                               PipelineStage::TRANSFER)) {
        dynamic_cast<BufferResource*>(resource)->usage |= BufferUsage::TRANSFER_DST;
    }
    return *this;
}

auto canta::RenderPass::addTransferRead(const canta::BufferIndex index) -> RenderPass& {
    assert(index.id >= 0);
    if (const auto resource = reads(index, Access::TRANSFER_READ,
                              PipelineStage::TRANSFER)) {
        dynamic_cast<BufferResource*>(resource)->usage |= BufferUsage::TRANSFER_SRC;
    }
    return *this;
}

auto canta::RenderPass::addIndirectRead(const canta::BufferIndex index) -> RenderPass& {
    assert(index.id >= 0);
    if (const auto resource = reads(index, Access::INDIRECT,
                              PipelineStage::DRAW_INDIRECT)) {
        dynamic_cast<BufferResource*>(resource)->usage |= BufferUsage::INDIRECT;
    }
    return *this;
}

auto canta::RenderPass::addDummyWrite(const ImageIndex index) -> RenderPass & {
    assert(index.id >= 0);
    writes(index, Access::NONE, PipelineStage::NONE, ImageLayout::GENERAL);
    return *this;
}

auto canta::RenderPass::addDummyRead(const ImageIndex index) -> RenderPass & {
    assert(index.id >= 0);
    reads(index, Access::NONE, PipelineStage::NONE, ImageLayout::GENERAL);
    return *this;
}

auto canta::RenderPass::addDummyWrite(const BufferIndex index) -> RenderPass & {
    assert(index.id >= 0);
    writes(index, Access::NONE, PipelineStage::NONE);
    return *this;
}

auto canta::RenderPass::addDummyRead(const BufferIndex index) -> RenderPass & {
    assert(index.id >= 0);
    reads(index, Access::NONE, PipelineStage::NONE);
    return *this;
}

bool isDummy(const canta::ResourceAccess &access) {
    return access.access == canta::Access::NONE && access.stage == canta::PipelineStage::NONE;
}


auto canta::RenderPass::writes(const canta::ImageIndex index, const canta::Access access, const canta::PipelineStage stage, const canta::ImageLayout layout) -> Resource * {
    assert(index.id >= 0);
    const auto resource = _graph->_resources[index.index].get();
    if (resource) {
        _outputs.push_back({
            .id = index.id,
            .index = index.index,
            .access = access,
            .stage = stage,
            .layout = layout
        });
    }
    return resource;
}

auto canta::RenderPass::reads(const canta::ImageIndex index, const canta::Access access, const canta::PipelineStage stage, const canta::ImageLayout layout) -> Resource * {
    assert(index.id >= 0);
    const auto resource = _graph->_resources[index.index].get();
    if (resource) {
        _inputs.push_back({
            .id = index.id,
            .index = index.index,
            .access = access,
            .stage = stage,
            .layout = layout
        });
    }
    return resource;
}

auto canta::RenderPass::writes(const canta::BufferIndex index, const canta::Access access, const canta::PipelineStage stage) -> Resource * {
    assert(index.id >= 0);
    const auto resource = _graph->_resources[index.index].get();
    if (resource) {
        _outputs.push_back({
            .id = index.id,
            .index = index.index,
            .access = access,
            .stage = stage
        });
        if (stage == PipelineStage::HOST && resource->type == ResourceType::BUFFER) {
            dynamic_cast<BufferResource*>(resource)->memoryType = MemoryType::STAGING;
        }
    }
    return resource;
}

auto canta::RenderPass::reads(const canta::BufferIndex index, const canta::Access access, const canta::PipelineStage stage) -> Resource * {
    assert(index.id >= 0);
    const auto resource = _graph->_resources[index.index].get();
    if (resource) {
        _inputs.push_back({
            .id = index.id,
            .index = index.index,
            .access = access,
            .stage = stage
        });
        if (stage == PipelineStage::HOST && resource->type == ResourceType::BUFFER) {
            dynamic_cast<BufferResource*>(resource)->memoryType = MemoryType::READBACK;
        }
    }
    return resource;
}

void canta::RenderPass::unpack(std::array<u8, 192>& dst, i32& i, const ImageIndex& image) {
    // in debug builds check that image pushconstants have corresponding dependencies declared in pass
    assert([&]() -> bool {
        return std::ranges::any_of(_inputs, [&](const auto& input) { return input.index == image.index; }) ||
            std::ranges::any_of(_outputs, [&](const auto& output) { return output.index == image.index; });
    }());

    _deferredPushConstants.push_back({
        .type = 0,
        .id = image.id,
        .index = image.index,
        .offset = i
    });
    i += sizeof(i32);
    _pushConstantSize += sizeof(i32);
    assert(_pushConstantSize <= 128);
}

void canta::RenderPass::unpack(std::array<u8, 192>& dst, i32& i, const BufferIndex& buffer) {
    // in debug builds check that image pushconstants have corresponding dependencies declared in pass
    assert([&]() -> bool {
        return std::ranges::any_of(_inputs, [&](const auto& input) { return input.index == buffer.index; }) ||
            std::ranges::any_of(_outputs, [&](const auto& output) { return output.index == buffer.index; });
    }());

    _deferredPushConstants.push_back({
        .type = 1,
        .id = buffer.id,
        .index = buffer.index,
        .offset = i
    });
    i += sizeof(u64);
    _pushConstantSize += sizeof(u64);
    assert(_pushConstantSize <= 128);
}

void unpackImage(std::array<u8, 192>& dst, i32 i, const canta::ImageHandle& image) {
    const auto id = image->defaultView().index();
    auto* data = reinterpret_cast<const u8*>(&id);
    for (auto j = 0; j < sizeof(id); j++) {
        dst[i + j] = data[j];
    }
    i += sizeof(id);
    assert(i <= 128);
}

void unpackBuffer(std::array<u8, 192>& dst, i32 i, const canta::BufferHandle& buffer) {
    const auto id = buffer->address();
    auto* data = reinterpret_cast<const u8*>(&id);
    for (auto j = 0; j < sizeof(id); j++) {
        dst[i + j] = data[j];
    }
    i += sizeof(id);
    assert(i <= 128);
}

void canta::RenderPass::unpack(std::array<u8, 192>& dst, i32& i, const ImageHandle& image) {
    const auto id = image->defaultView().index();
    unpack(dst, i, id);
}

void canta::RenderPass::unpack(std::array<u8, 192>& dst, i32& i, const BufferHandle& buffer) {
    const auto id = buffer->address();
    unpack(dst, i, id);
}

auto canta::RenderPass::dispatchWorkgroups(u32 x, u32 y, u32 z) -> RenderPass & {
    auto pushConstants = _pushConstants;
    auto pushConstantsSize = _pushConstantSize;
    auto deferredPushConstants = _deferredPushConstants;
    return setExecuteFunction([x, y, z, pushConstants, pushConstantsSize, deferredPushConstants] (CommandBuffer& cmd, RenderGraph& graph) {
        auto push = pushConstants;
        for (auto& deferredPushConstant : deferredPushConstants) {
            if (deferredPushConstant.type == 0) {
                unpackImage(push, deferredPushConstant.offset, graph.getImage({.id = deferredPushConstant.id, .index = deferredPushConstant.index}));
            } else {
                unpackBuffer(push, deferredPushConstant.offset, graph.getBuffer({.id = deferredPushConstant.id, .index = deferredPushConstant.index}));
            }
        }

        cmd.pushConstants(canta::ShaderStage::COMPUTE, { push.data(), pushConstantsSize }, 0);
        cmd.dispatchWorkgroups(x, y, z);
    });
}

auto canta::RenderPass::dispatchThreads(u32 x, u32 y, u32 z) -> RenderPass & {
    auto pushConstants = _pushConstants;
    auto pushConstantsSize = _pushConstantSize;
    auto deferredPushConstants = _deferredPushConstants;
    return setExecuteFunction([x, y, z, pushConstants, pushConstantsSize, deferredPushConstants] (CommandBuffer& cmd, RenderGraph& graph) {
        auto push = pushConstants;
        for (auto& deferredPushConstant : deferredPushConstants) {
            if (deferredPushConstant.type == 0) {
                unpackImage(push, deferredPushConstant.offset, graph.getImage({.id = deferredPushConstant.id, .index = deferredPushConstant.index}));
            } else {
                unpackBuffer(push, deferredPushConstant.offset, graph.getBuffer({.id = deferredPushConstant.id, .index = deferredPushConstant.index}));
            }
        }

        cmd.pushConstants(canta::ShaderStage::COMPUTE, { push.data(), pushConstantsSize }, 0);
        cmd.dispatchThreads(x, y, z);
    });
}

auto canta::RenderPass::aliasImageOutput(i32 index) -> std::expected<ImageIndex, i32> {
    assert(index < _outputs.size());
    auto& output = _outputs[index];
    const auto& resource = _graph->_resources[output.index];
    if (resource->type != ResourceType::IMAGE)
        return std::unexpected(-1);
    auto alias = _graph->addAlias(ImageIndex{ .id = output.id, .index = output.index });
    output.id = alias.id;
    output.index = alias.index;
    return alias;
}

auto canta::RenderPass::aliasBufferOutput(i32 index) -> std::expected<BufferIndex, i32> {
    assert(index < _outputs.size());
    auto& output = _outputs[index];
    const auto& resource = _graph->_resources[output.index];
    if (resource->type != ResourceType::BUFFER)
        return std::unexpected(-1);
    auto alias = _graph->addAlias(BufferIndex{ .id = output.id, .index = output.index });
    output.id = alias.id;
    output.index = alias.index;
    return alias;
}

auto canta::RenderPass::aliasImageOutput(ImageIndex index) -> std::expected<ImageIndex, i32> {
    i32 outputIndex = -1;
    for (i32  i = 0; const auto& output : _outputs) {
        if (output.id == index.id) {
            outputIndex = i;
            break;
        }
        i++;
    }
    if (outputIndex == -1) return std::unexpected(-2);

    return aliasImageOutput(outputIndex);
}

auto canta::RenderPass::aliasBufferOutput(BufferIndex index) -> std::expected<BufferIndex, i32> {
    i32 outputIndex = -1;
    for (i32  i = 0; const auto& output : _outputs) {
        if (output.id == index.id) {
            outputIndex = i;
            break;
        }
        i++;
    }
    if (outputIndex == -1) return std::unexpected(-2);

    return aliasBufferOutput(outputIndex);
}


auto canta::RenderPass::aliasImageOutputs() -> std::vector<ImageIndex> {
    std::vector<ImageIndex> aliases = {};
    for (i32 i = 0; i < _outputs.size(); ++i) {
        if (auto alias = aliasImageOutput(i))
            aliases.push_back(*alias);
    }
    return aliases;
}

auto canta::RenderPass::aliasBufferOutputs() -> std::vector<BufferIndex> {
    std::vector<BufferIndex> aliases = {};
    for (i32 i = 0; i < _outputs.size(); ++i) {
        if (auto alias = aliasBufferOutput(i))
            aliases.push_back(*alias);
    }
    return aliases;
}

auto canta::RenderPass::isInput(ImageIndex index) const -> bool {
    for (auto& input : _inputs) {
        if (input.id == index.id) return true;
    }
    return false;
}

auto canta::RenderPass::isInput(BufferIndex index) const -> bool {
    for (auto& input : _inputs) {
        if (input.id == index.id) return true;
    }
    return false;
}

auto canta::RenderPass::isOutput(ImageIndex index) const -> bool {
    for (auto& output : _outputs) {
        if (output.id == index.id) return true;
    }
    return false;
}

auto canta::RenderPass::isOutput(BufferIndex index) const -> bool {
    for (auto& output : _outputs) {
        if (output.id == index.id) return true;
    }
    return false;
}

auto canta::RenderGraph::create(const canta::RenderGraph::CreateInfo &info) -> RenderGraph {
    RenderGraph graph = {};

    graph._device = info.device;
    graph._name = info.name;
    graph._multiQueue = info.multiQueue;
    graph._allowHostPasses = info.allowHostPasses;
    for (auto& poolGroup : graph._commandPools) {
        poolGroup[0] = info.device->createCommandPool({
            .queueType = QueueType::GRAPHICS
        }).value();
        poolGroup[1] = info.device->createCommandPool({
            .queueType = QueueType::COMPUTE
        }).value();
//        for (auto& pool : poolGroup) {
//            pool = info.device->createCommandPool({
//                .queueType = QueueType::GRAPHICS
//            }).value();
//        }
    }
    graph._timingEnabled = info.enableTiming;
    graph._timingMode = info.timingMode;
    graph._pipelineStatisticsEnabled = info.enablePipelineStatistics;
    graph._individualPipelineStatistics = info.individualPipelineStatistics;

    if (info.enableTiming && info.timingMode == TimingMode::SINGLE) {
        for (auto& frameTimers : graph._timers) {
            frameTimers.emplace_back(std::make_pair(info.name, info.device->createTimer()));
        }
    }
    if (info.enablePipelineStatistics && !info.individualPipelineStatistics) {
        for (auto& framePipelineStats : graph._pipelineStats) {
            framePipelineStats.emplace_back(std::make_pair(info.name, info.device->createPipelineStatistics()));
        }
    }

    graph._cpuTimeline = info.device->createSemaphore({
        .initialValue = 0,
        .name = "cpu_timeline"
    }).value();
    return graph;
}

auto canta::RenderGraph::addPass(PassInfo info) -> RenderPass & {
    const u32 index = _passes.size();
    _passes.emplace_back();
    _passes.back()._graph = this;
    _passes.back()._name = info.name;
    _passes.back()._type = info.type;
    switch (info.type) {
        case PassType::GRAPHICS:
            info.queue = QueueType::GRAPHICS;
            break;
        case PassType::HOST:
            info.queue = QueueType::NONE;
            break;
        default:
            break;
    }
    _passes.back().setQueue(info.queue);
    _passes.back().setGroup(info.group);
    _passes.back().setManualPipeline(info.manualPipeline);
    if (_timingEnabled && _timingMode != TimingMode::SINGLE) {
        if (_timers[_device->flyingIndex()].size() <= index) {
            _timers[_device->flyingIndex()].emplace_back(std::make_pair(info.name, _device->createTimer()));
        } else
            _timers[_device->flyingIndex()][index].first = info.name;
    }
    if (_pipelineStatisticsEnabled && _individualPipelineStatistics) {
        if (_pipelineStats[_device->flyingIndex()].size() <= index) {
            _pipelineStats[_device->flyingIndex()].emplace_back(std::make_pair(info.name, _device->createPipelineStatistics()));
        } else
            _pipelineStats[_device->flyingIndex()][index].first = info.name;
    }
    return _passes.back();
}

auto canta::RenderGraph::addPass(canta::RenderPass &&pass) -> RenderPass & {
    const u32 index = _passes.size();
    _passes.push_back(std::move(pass));
    _passes.back()._graph = this;
    if (_timingEnabled && _timingMode != TimingMode::SINGLE) {
        if (_timers[_device->flyingIndex()].size() <= index) {
            _timers[_device->flyingIndex()].emplace_back(std::make_pair(_passes.back().name(), _device->createTimer()));
        } else
            _timers[_device->flyingIndex()][index].first = _passes.back().name();
    }
    if (_pipelineStatisticsEnabled && _individualPipelineStatistics) {
        if (_pipelineStats[_device->flyingIndex()].size() <= index) {
            _pipelineStats[_device->flyingIndex()].emplace_back(std::make_pair(_passes.back().name(), _device->createPipelineStatistics()));
        } else
            _pipelineStats[_device->flyingIndex()][index].first = _passes.back().name();
    }
    return _passes.back();
}

auto canta::RenderGraph::addClearPass(const std::string_view name, const canta::ImageIndex index, const ClearValue& value, const RenderGroup group) -> RenderPass & {
    auto& clearPass = addPass({.name = name, .type = PassType::TRANSFER, .group = group, .manualPipeline = true});
    clearPass.addTransferWrite(index);
    clearPass.setExecuteFunction([index, value] (CommandBuffer& cmd, RenderGraph& graph) {
        const auto image = graph.getImage(index);
        cmd.clearImage(image, ImageLayout::TRANSFER_DST, value);
    });
    return clearPass;
}

auto canta::RenderGraph::addBlitPass(const std::string_view name, const canta::ImageIndex src, const canta::ImageIndex dst, const Filter filter, const RenderGroup group) -> RenderPass & {
    auto& blitPass = addPass({.name = name, .type = PassType::TRANSFER, .group = group, .manualPipeline = true});
    blitPass.addTransferRead(src);
    blitPass.addTransferWrite(dst);
    blitPass.setExecuteFunction([src, dst, filter] (CommandBuffer& cmd, RenderGraph& graph) {
        const auto srcImage = graph.getImage(src);
        const auto dstImage = graph.getImage(dst);
        cmd.blit({
            .src = srcImage,
            .dst = dstImage,
            .srcLayout = ImageLayout::TRANSFER_SRC,
            .dstLayout = ImageLayout::TRANSFER_DST,
            .filter = filter
        });
    });
    return blitPass;
}

auto canta::RenderGraph::addClearPass(const std::string_view name, const BufferIndex index, u32 value, u32 offset, u32 size, const RenderGroup group) -> RenderPass & {
    auto& clearPass = addPass({ .name = name, .type = PassType::TRANSFER, .group = group, .manualPipeline = true});
    clearPass.addTransferWrite(index);
    clearPass.setExecuteFunction([index, value, offset, size] (CommandBuffer& cmd, RenderGraph& graph) {
        const auto buffer = graph.getBuffer(index);
        cmd.clearBuffer(buffer, value, offset, size);
    });
    return clearPass;
}

auto canta::RenderGraph::addCopyPass(const std::string_view name, const BufferIndex src, const BufferIndex dst, u32 srcOffset, u32 dstOffset, u32 size, const RenderGroup group) -> RenderPass & {
    auto& copyPass = addPass({.name = name, .type = PassType::TRANSFER, .group = group, .manualPipeline = true});
    copyPass.addTransferRead(src);
    copyPass.addTransferWrite(dst);
    copyPass.setExecuteFunction([src, dst, srcOffset, dstOffset, size] (CommandBuffer& cmd, RenderGraph& graph) {
        const auto srcBuffer = graph.getBuffer(src);
        const auto dstBuffer = graph.getBuffer(dst);
        cmd.copyBuffer({
            .src = srcBuffer,
            .dst = dstBuffer,
            .srcOffset = srcOffset,
            .dstOffset = dstOffset,
            .size = size,
        });
    });
    return copyPass;
}

auto canta::RenderGraph::addCopyPass(const std::string_view name, const BufferIndex src, const ImageIndex dst, const RenderGroup group) -> RenderPass & {
    auto& copyPass = addPass({.name = name, .type = PassType::TRANSFER, .group = group, .manualPipeline = true});
    copyPass.addTransferRead(src);
    copyPass.addTransferWrite(dst);
    copyPass.setExecuteFunction([src, dst] (CommandBuffer& cmd, RenderGraph& graph) {
        const auto srcBuffer = graph.getBuffer(src);
        const auto dstImage = graph.getImage(dst);
        cmd.copyBufferToImage({
            .buffer = srcBuffer,
            .image = dstImage,
            .dstLayout = ImageLayout::TRANSFER_DST,
        });
    });
    return copyPass;
}

auto canta::RenderGraph::addCopyPass(const std::string_view name, const ImageIndex src, const BufferIndex dst, const RenderGroup group) -> RenderPass & {
    auto& copyPass = addPass({.name = name, .type = PassType::TRANSFER, .group = group, .manualPipeline = true});
    copyPass.addTransferRead(src);
    copyPass.addTransferWrite(dst);
    copyPass.setExecuteFunction([src, dst] (CommandBuffer& cmd, RenderGraph& graph) {
        const auto srcImage = graph.getImage(src);
        const auto dstBuffer = graph.getBuffer(dst);
        cmd.copyImageToBuffer({
            .buffer = dstBuffer,
            .image = srcImage,
            .dstLayout = ImageLayout::TRANSFER_SRC,
        });
    });
    return copyPass;
}




auto canta::RenderGraph::getGroup(const std::string_view name, const std::array<f32, 4>& colour) -> RenderGroup {
    const auto it = _renderGroups.find(name);
    if (it != _renderGroups.end()) {
        return {
            .id = it->second.first
        };
    }

    const auto it1 = _renderGroups.insert(std::make_pair(name, std::make_pair(_groupId++, colour)));
    return {
        .id = it1.first->second.first
    };
}

auto canta::RenderGraph::getGroupName(const canta::RenderGroup group) -> std::string {
    for (const auto [key, value] : _renderGroups) {
        if (value.first == group.id)
            return key.data();
    }
    return {};
}

auto canta::RenderGraph::getGroupColour(const canta::RenderGroup group) -> std::array<f32, 4> {
    for (const auto [key, value] : _renderGroups) {
        if (value.first == group.id)
            return value.second;
    }
    return { 0, 1, 0, 1 };
}

auto canta::RenderGraph::addImage(const canta::ImageDescription &description) -> ImageIndex {
    const auto it = _nameToIndex.find(description.name.data());
    if (it != _nameToIndex.end()) {
        const u32 index = it->second;
        const auto imageResource = dynamic_cast<ImageResource*>(_resources[index].get());
        imageResource->initialLayout = description.initialLayout;
        if (description.handle) {
            imageResource->width = description.handle->width();
            imageResource->height = description.handle->height();
            imageResource->depth = description.handle->depth();
            imageResource->mipLevels = description.handle->mips();
            imageResource->format = description.handle->format();
            imageResource->usage = description.handle->usage();
        } else {
            imageResource->width = description.width;
            imageResource->height = description.height;
            imageResource->depth = description.depth;
            imageResource->mipLevels = description.mipLevels;
            imageResource->format = description.format;
            imageResource->usage = description.usage;
        }
        if (description.handle) {
            const u32 imageIndex = imageResource->imageIndex;
            _images[imageIndex] = description.handle;
        }
        return {
            .id = _resourceId++,
            .index = index
        };
    }

    const u32 index = _resources.size();
    ImageResource resource = {};
    resource.imageIndex = _images.size();
    _images.push_back(description.handle);
    resource.matchesBackbuffer = description.matchesBackbuffer;
    if (description.handle) {
        resource.width = description.handle->width();
        resource.height = description.handle->height();
        resource.depth = description.handle->depth();
        resource.mipLevels = description.handle->mips();
        resource.format = description.handle->format();
        resource.usage = description.handle->usage();
    } else {
        resource.width = description.width;
        resource.height = description.height;
        resource.depth = description.depth;
        resource.mipLevels = description.mipLevels;
        resource.format = description.format;
        resource.usage = description.usage;
    }
    resource.initialLayout = description.initialLayout;
    resource.index = index;
    resource.type = ResourceType::IMAGE;
    resource.name = description.name;
    _resources.push_back(std::make_unique<ImageResource>(resource));
    _nameToIndex.insert(std::make_pair(description.name.data(), index));
    return {
        .id = _resourceId++,
        .index = index
    };
}

auto canta::RenderGraph::addBuffer(const canta::BufferDescription &description) -> BufferIndex {
    const auto it = _nameToIndex.find(description.name.data());
    if (it != _nameToIndex.end()) {
        const u32 index = it->second;
        const auto bufferResource = dynamic_cast<BufferResource*>(_resources[index].get());
        if (description.handle) {
            bufferResource->size = description.handle->size();
            bufferResource->usage = description.handle->usage();
        } else {
            bufferResource->size = description.size;
            bufferResource->usage = description.usage;
        }
        if (description.handle) {
            const u32 bufferIndex = bufferResource->bufferIndex;
            _buffers[bufferIndex] = description.handle;
        }
        return {
            .id = _resourceId++,
            .index = index
        };
    }

    u32 index = _resources.size();
    BufferResource resource = {};
    resource.bufferIndex = _buffers.size();
    _buffers.push_back(description.handle);
    if (description.handle) {
        resource.size = description.handle->size();
        resource.usage = description.handle->usage();
    } else {
        resource.size = description.size;
        resource.usage = description.usage;
    }
    resource.index = index;
    resource.type = ResourceType::BUFFER;
    resource.name = description.name;
    _resources.push_back(std::make_unique<BufferResource>(resource));
    _nameToIndex.insert(std::make_pair(description.name.data(), index));
    return {
        .id = _resourceId++,
        .index = index
    };
}

auto canta::RenderGraph::addAlias(const canta::ImageIndex index) -> ImageIndex {
    return {
        .id = _resourceId++,
        .index = index.index
    };
}

auto canta::RenderGraph::addAlias(const canta::BufferIndex index) -> BufferIndex {
    return {
        .id = _resourceId++,
        .index = index.index
    };
}

auto canta::RenderGraph::getImage(const canta::ImageIndex index) -> ImageHandle {
    const auto imageIndex = dynamic_cast<ImageResource*>(_resources[index.index].get())->imageIndex;
    return _images[imageIndex];
}

auto canta::RenderGraph::getBuffer(const canta::BufferIndex index) -> BufferHandle {
    const auto bufferIndex = dynamic_cast<BufferResource*>(_resources[index.index].get())->bufferIndex;
    return _buffers[bufferIndex];
}

void canta::RenderGraph::setBackbuffer(const canta::ImageIndex index, const ImageLayout finalLayout) {
    _backbufferId = index.id;
    _backbufferIndex = index.index;
    _backbufferFinalLayout = finalLayout;
}

void canta::RenderGraph::setBackbuffer(const canta::BufferIndex index) {
    _backbufferId = index.id;
    _backbufferIndex = index.index;
}

void canta::RenderGraph::reset() {
    _passes.clear();
    _resourceId = 0;
}

auto canta::RenderGraph::compile() -> std::expected<bool, RenderGraphError> {
    _orderedPasses.clear();

    std::vector<std::vector<u32>> outputs(_resourceId);
    for (u32 i = 0; i < _passes.size(); i++) {
        const auto& pass = _passes[i];
        for (const auto& output : pass._outputs)
            outputs[output.id].push_back(i);
    }

    std::vector<bool> visited(_passes.size(), false);
    std::vector<bool> onStack(_passes.size(), false);

    std::function<bool(u32)> dfs = [&](const u32 index) -> bool {
        visited[index] = true;
        onStack[index] = true;
        const auto& pass = _passes[index];
        for (const auto& input : pass._inputs) {
            for (const auto& output : outputs[input.id]) {
                if (visited[output] && onStack[output])
                    return false;
                if (!visited[output]) {
                    if (!dfs(output))
                        return false;
                }
            }
        }
        _orderedPasses.push_back(&_passes[index]);
        onStack[index] = false;
        return true;
    };

    for (const auto& pass : outputs[_backbufferId]) {
        if (!dfs(pass)) {
            _device->logger().error("circular dependencies found in rendergraph");
            return std::unexpected(RenderGraphError::CYCLICAL_GRAPH);
        }
    }

    if (_multiQueue) {
        std::vector<std::vector<u32>> inputs(_resourceId);
        for (u32 i = 0; i < _orderedPasses.size(); i++) {
            const auto& pass = _orderedPasses[i];
            for (const auto& input : pass->_inputs)
                inputs[input.id].push_back(i);
        }

        i32 dependencyLevelCount = 1;
        std::vector<i32> distances(_orderedPasses.size(), 0);
        for (i32 i = 0; const auto& pass : _orderedPasses) {
            for (const auto& output : pass->_outputs) {
                for (const auto& vertex : inputs[output.id]) {
                    if (distances[vertex] < distances[i] + 1) {
                        distances[vertex] = distances[i] + 1;
                        dependencyLevelCount = std::max(distances[i] + 2, dependencyLevelCount);
                    }
                }
            }
            i++;
        }

        for (i32 i = 0; const auto& pass : _orderedPasses) {
            const auto distance = distances[i];
            auto onLevelCount = 1;
            for (i32 j = 0; const auto& d : distances) {
                if (j == i) {
                    j++;
                    continue;
                };
                if (d == distance)
                    onLevelCount++;
                j++;
            }

            if (pass->_type == PassType::COMPUTE && onLevelCount == 2) {
                pass->setQueue(QueueType::COMPUTE);
            }
            if (pass->_type == PassType::HOST) {
                if (!_allowHostPasses) return std::unexpected(RenderGraphError::INVALID_PASS);
                pass->setQueue(QueueType::NONE);
            }

            i++;
        }
    }

    buildBarriers();
    buildResources();
    buildRenderAttachments();

    return true;
}

inline auto getQueueIndex(const canta::QueueType queue) -> u32 {
    switch (queue) {
        case canta::QueueType::NONE:
            return 2;
            break;
        case canta::QueueType::GRAPHICS:
            return 0;
        case canta::QueueType::COMPUTE:
            return 1;
        case canta::QueueType::TRANSFER:
            break;
        case canta::QueueType::SPARSE_BINDING:
            break;
        case canta::QueueType::PRESENT:
            break;
    }
    return 0;
}

auto canta::RenderGraph::execute(std::span<SemaphorePair> waits, std::span<SemaphorePair> signals, std::span<ImageBarrier> imagesToAcquire, bool synchronous) -> std::expected<bool, RenderGraphError> {
    _timerCount = 0;
    RenderGroup currentGroup = {};
    for (auto& pool : _commandPools[_device->flyingIndex()])
        pool.reset();
    CommandBuffer* currentCommandBuffer = nullptr;
    QueueType currentQueue = QueueType::NONE;
    struct CommandIndex {
        u32 queueIndex;
        u32 bufferIndex;
        u32 waitIndex;
        u32 hostIndex;
    };
    std::vector<CommandIndex> commandBufferIndices;
    std::vector<RenderPass*> hostPasses;
    std::vector<std::vector<QueueType>> commandBufferWaits = {};

    // currentCommandBuffer->begin();
//    if (_timingEnabled && _timingMode == TimingMode::SINGLE) {
//        _timers[_device->flyingIndex()].front().first = _name;
//        _timers[_device->flyingIndex()].front().second.begin(*currentCommandBuffer);
//    }
//    if (_pipelineStatisticsEnabled && !_individualPipelineStatistics) {
//        _pipelineStats[_device->flyingIndex()].front().first = _name;
//        _pipelineStats[_device->flyingIndex()].front().second.begin(*currentCommandBuffer);
//    }

    // for (auto& image : imagesToAcquire) {
    //     currentCommandBuffer->barrier(image);
    // }

    for (u32 i = 0; i < _orderedPasses.size(); i++) {
        const auto& pass = _orderedPasses[i];
        if (pass->_type == PassType::HOST) {
            if (!_allowHostPasses) return std::unexpected(RenderGraphError::INVALID_PASS);
            commandBufferIndices.push_back(CommandIndex{ .queueIndex = getQueueIndex(QueueType::NONE), .bufferIndex = 0, .waitIndex = static_cast<u32>(commandBufferWaits.size()), .hostIndex = static_cast<u32>(hostPasses.size())});
            commandBufferWaits.push_back({});
            for (const auto& wait : pass->_waits) {
                commandBufferWaits.back().push_back(wait.second);
            }
            hostPasses.push_back(pass);
            continue;
        }
        if (pass->_waits.empty() && !currentCommandBuffer) {
            currentCommandBuffer = &_commandPools[_device->flyingIndex()][getQueueIndex(pass->getQueue())].getBuffer();
            commandBufferIndices.push_back(CommandIndex{ .queueIndex = getQueueIndex(pass->getQueue()), .bufferIndex = _commandPools[_device->flyingIndex()][getQueueIndex(pass->getQueue())].index() - 1, .waitIndex = static_cast<u32>(commandBufferWaits.size()), .hostIndex = 0});
            commandBufferWaits.push_back({});
            currentCommandBuffer->begin();
            currentQueue = pass->getQueue();
        }
        if (!pass->_waits.empty()) {
            if (_commandPools[_device->flyingIndex()][getQueueIndex(pass->getQueue())].index() != 0 && _commandPools[_device->flyingIndex()][getQueueIndex(pass->getQueue())].buffers().back().isActive()) {
                _commandPools[_device->flyingIndex()][getQueueIndex(pass->getQueue())].buffers().back().end();
            }

            currentCommandBuffer = &_commandPools[_device->flyingIndex()][getQueueIndex(pass->getQueue())].getBuffer();
            commandBufferIndices.push_back(CommandIndex{ .queueIndex = getQueueIndex(pass->getQueue()), .bufferIndex = _commandPools[_device->flyingIndex()][getQueueIndex(pass->getQueue())].index() - 1, .waitIndex = static_cast<u32>(commandBufferWaits.size()), .hostIndex = 0});
            commandBufferWaits.push_back({});
            bool waitCurrentQueue = false;
            for (const auto& wait : pass->_waits) {
                if (wait.second == pass->getQueue()) waitCurrentQueue = true;
                commandBufferWaits.back().push_back(wait.second);
            }

            if (!waitCurrentQueue)
                commandBufferWaits.back().push_back(pass->getQueue());

            currentCommandBuffer->begin();
            currentQueue = pass->getQueue();
        } else if (pass->getQueue() != currentQueue) {
            if (_commandPools[_device->flyingIndex()][getQueueIndex(pass->getQueue())].index() == 0 || !_commandPools[_device->flyingIndex()][getQueueIndex(pass->getQueue())].buffers().back().isActive()) {
                currentCommandBuffer = &_commandPools[_device->flyingIndex()][getQueueIndex(pass->getQueue())].getBuffer();
                commandBufferIndices.push_back(CommandIndex{ .queueIndex = getQueueIndex(pass->getQueue()), .bufferIndex = _commandPools[_device->flyingIndex()][getQueueIndex(pass->getQueue())].index() - 1, .waitIndex = static_cast<u32>(commandBufferWaits.size()), .hostIndex = 0});
                commandBufferWaits.push_back({});
                currentCommandBuffer->begin();
                currentQueue = pass->getQueue();
            } else {
                currentCommandBuffer = &_commandPools[_device->flyingIndex()][getQueueIndex(pass->getQueue())].buffers().back();
                currentQueue = pass->getQueue();
            }
        }

        if (!currentCommandBuffer)
            return std::unexpected(RenderGraphError::INVALID_SUBMISSION);

        submitBarriers(*currentCommandBuffer, pass->_barriers);
        startQueries(*currentCommandBuffer, i, *pass, currentGroup);
        currentCommandBuffer->pushDebugLabel(pass->_name, pass->_debugColour);

        if (pass->_type == PassType::GRAPHICS) {
            RenderingInfo info = {};
            i32 width = pass->_width;
            i32 height = pass->_height;

            if (width < 0 || height < 0) {
                if (!pass->_renderingColourAttachments.empty()) {
                    width = pass->_renderingColourAttachments.front().image->width();
                    height = pass->_renderingColourAttachments.front().image->height();
                } else if (pass->_depthAttachment.index > -1) {
                    width = pass->_renderingDepthAttachment.image->width();
                    height = pass->_renderingDepthAttachment.image->height();
                }
            }

            info.colourAttachments = pass->_renderingColourAttachments;
            if (pass->_depthAttachment.index >= 0) {
                info.depthAttachment = pass->_renderingDepthAttachment;
            }

            info.size = { static_cast<u32>(width), static_cast<u32>(height) };
            currentCommandBuffer->beginRendering(info);
        }
        if (!pass->_manualPipeline &&
            !currentCommandBuffer->bindPipeline(pass->_pipeline))
            return std::unexpected(RenderGraphError::INVALID_PIPELINE);

        pass->_execute(*currentCommandBuffer, *this);

        if (pass->_type == PassType::GRAPHICS) {
            currentCommandBuffer->endRendering();
        }
        currentCommandBuffer->popDebugLabel();
        endQueries(*currentCommandBuffer, i, *pass);
        submitBarriers(*currentCommandBuffer, pass->_releaseBarriers);

        currentGroup = pass->getGroup();

        if (pass->_signal) {
            currentCommandBuffer->end();
            currentCommandBuffer = nullptr;
        }
    }
    if (!currentCommandBuffer)
        return std::unexpected(RenderGraphError::INVALID_SUBMISSION);

    if (_backbufferFinalLayout != ImageLayout::UNDEFINED) {
        currentCommandBuffer->barrier({
            .image = getImage({ .index = static_cast<u32>(_backbufferIndex)}),
            .srcStage = _backbufferBarrier.srcStage,
            .dstStage = _backbufferBarrier.dstStage,
            .srcAccess = _backbufferBarrier.srcAccess,
            .dstAccess = _backbufferBarrier.dstAccess,
            .srcLayout = _backbufferBarrier.srcLayout,
            .dstLayout = _backbufferBarrier.dstLayout
        });
    }

//    if (_pipelineStatisticsEnabled && !_individualPipelineStatistics)
//        _pipelineStats[_device->flyingIndex()].front().second.end(*currentCommandBuffer);
//    if (_timingEnabled && _timingMode == TimingMode::SINGLE)
//        _timers[_device->flyingIndex()].front().second.end(*currentCommandBuffer);
    currentCommandBuffer->end();

    bool result = true;

    auto graphicsQueue = _device->queue(QueueType::GRAPHICS);
    auto computeQueue = _device->queue(QueueType::COMPUTE);

    u32 firstDevice = 0;
    for (i32 d = 0; d < commandBufferIndices.size(); d++) {
        if (commandBufferIndices[d].queueIndex != getQueueIndex(QueueType::NONE)) {
            firstDevice = d;
            break;
        }
    }
    u32 lastDevice = 0;
    for (i32 d = commandBufferIndices.size() - 1; d >= 0; --d) {
        if (commandBufferIndices[d].queueIndex != getQueueIndex(QueueType::NONE)) {
            lastDevice = d;
            break;
        }
    }

    for (u32 i = 0; const auto& indices : commandBufferIndices) {
        if (indices.queueIndex > 1) { // if host

            for (auto& wait : commandBufferWaits[indices.waitIndex]) {
                _device->queue(wait)->timeline()->wait(_device->queue(wait)->timeline()->value());
            }
            hostPasses[indices.hostIndex]->_execute(*currentCommandBuffer, *this);

            _cpuTimeline->signal(_cpuTimeline->value() + 1);
            // _cpuTimeline->increment();

        } else { // if device
            auto& commandList = _commandPools[_device->flyingIndex()][indices.queueIndex].buffers()[indices.bufferIndex];

            std::vector<SemaphorePair> internalWaits;// = commandBufferWaits[indices.second.first];
            for (auto& wait : commandBufferWaits[indices.waitIndex]) {
                if (wait == QueueType::NONE) {
                    internalWaits.push_back(SemaphorePair(_cpuTimeline));
                } else {
                    internalWaits.push_back(SemaphorePair(_device->queue(wait)->timeline()));
                }
            }
            if (i == 0 || i == firstDevice)
                internalWaits.insert(internalWaits.end(), waits.begin(), waits.end());
            std::vector<SemaphorePair> internalSignals = {
                SemaphorePair(indices.queueIndex == 0 ? graphicsQueue->timeline() : computeQueue->timeline(), (indices.queueIndex == 0 ? graphicsQueue->timeline()->value() : computeQueue->timeline()->value()) + 1)
            };
            (indices.queueIndex == 0 ? graphicsQueue->timeline() : computeQueue->timeline())->increment();
            if (i == commandBufferIndices.size() - 1 || i == lastDevice)
                internalSignals.insert(internalSignals.end(), signals.begin(), signals.end());

            if (synchronous && i == commandBufferIndices.size() - 1) {
                internalSignals.push_back(SemaphorePair(_cpuTimeline, _cpuTimeline->value() + 1));
                _cpuTimeline->increment();
            }

            result |= (indices.queueIndex == 0 ? graphicsQueue : computeQueue)->submit({ &commandList, 1 }, internalWaits, internalSignals).value();
            if (!result)
                return std::unexpected(RenderGraphError::INVALID_SUBMISSION);
        }

        i++;
    }

    if (synchronous)
        _cpuTimeline->wait(_cpuTimeline->value());

    return result;
}

void canta::RenderGraph::submitBarriers(CommandBuffer& commandBuffer, const std::vector<RenderPass::Barrier> &barriers) {
    u32 imageBarrierCount = 0;
    ImageBarrier imageBarriers[barriers.size()];
    u32 bufferBarrierCount = 0;
    BufferBarrier bufferBarriers[barriers.size()];

    for (const auto& barrier : barriers) {
        const auto* resource = _resources[barrier.index].get();
        if (resource->type == ResourceType::IMAGE) {
            const auto image = getImage({ .index = barrier.index });
            imageBarriers[imageBarrierCount++] = ImageBarrier{
                    .image = image,
                    .srcStage = barrier.srcStage,
                    .dstStage = barrier.dstStage,
                    .srcAccess = barrier.srcAccess,
                    .dstAccess = barrier.dstAccess,
                    .srcLayout = barrier.srcLayout,
                    .dstLayout = barrier.dstLayout,
                    .srcQueue = _device->queue(barrier.srcQueue)->familyIndex(),
                    .dstQueue = _device->queue(barrier.dstQueue)->familyIndex(),
            };
        } else {
            const auto buffer = getBuffer({ .index = barrier.index });
            bufferBarriers[bufferBarrierCount++] = BufferBarrier{
                    .buffer = buffer,
                    .srcStage = barrier.srcStage,
                    .dstStage = barrier.dstStage,
                    .srcAccess = barrier.srcAccess,
                    .dstAccess = barrier.dstAccess,
                    .srcQueue = _device->queue(barrier.srcQueue)->familyIndex(),
                    .dstQueue = _device->queue(barrier.dstQueue)->familyIndex(),
            };
        }
    }
    for (u32 barrier = 0; barrier < imageBarrierCount; barrier++)
        commandBuffer.barrier(imageBarriers[barrier]);
    for (u32 barrier = 0; barrier < bufferBarrierCount; barrier++)
        commandBuffer.barrier(bufferBarriers[barrier]);
}

void canta::RenderGraph::startQueries(canta::CommandBuffer &commandBuffer, u32 passIndex, canta::RenderPass &pass, RenderGroup& currentGroup) {
    // if render group changes
    bool groupChanged = false;
    if (currentGroup.id != pass.getGroup().id) {
        if (currentGroup.id >= 0)
            commandBuffer.popDebugLabel();
        groupChanged = true;
        if (pass.getGroup().id >= 0)
            commandBuffer.pushDebugLabel(getGroupName(pass.getGroup()), getGroupColour(pass.getGroup()));
    } else
        groupChanged = false;

    if (_timingEnabled && _timingMode != TimingMode::SINGLE) {
        if (_timingMode == TimingMode::PER_GROUP && groupChanged) {
            if (currentGroup.id >= 0) {
                _timers[_device->flyingIndex()][_timerCount].second.end(commandBuffer);
                _timerCount++;
            }
            if (pass.getGroup().id >= 0) {
                _timers[_device->flyingIndex()][_timerCount].first = getGroupName(pass.getGroup());
                _timers[_device->flyingIndex()][_timerCount].second.begin(commandBuffer);
            }
        }
        if (_timingMode == TimingMode::PER_PASS || pass.getGroup().id < 0) {
            _timers[_device->flyingIndex()][_timerCount].first = pass._name;
            _timers[_device->flyingIndex()][_timerCount].second.begin(commandBuffer);
        }
    }
    if (_pipelineStatisticsEnabled && _individualPipelineStatistics)
        _pipelineStats[_device->flyingIndex()][passIndex].second.begin(commandBuffer);
}

void canta::RenderGraph::endQueries(canta::CommandBuffer &commandBuffer, u32 passIndex, canta::RenderPass &pass) {
    if (_pipelineStatisticsEnabled && _individualPipelineStatistics)
        _pipelineStats[_device->flyingIndex()][passIndex].second.end(commandBuffer);
    if (_timingEnabled && _timingMode != TimingMode::SINGLE) {
        if (_timingMode == TimingMode::PER_PASS || pass.getGroup().id < 0) {
            _timers[_device->flyingIndex()][_timerCount++].second.end(commandBuffer);
        }
    }
}

void canta::RenderGraph::buildBarriers() {
    auto readsResource = [&](const RenderPass& pass, const u32 resource) -> bool {
        for (const auto& access : pass._inputs) {
            if (access.index == resource)
                return true;
        }
        return false;
    };
    auto writesResource = [&](const RenderPass& pass, const u32 resource) -> bool {
        for (const auto& access : pass._outputs) {
            if (access.index == resource)
                return true;
        }
        return false;
    };

    // for each pass for each input/output find next pass that access that resource and add barrier to next pass
    for (i32 passIndex = 0; passIndex < _orderedPasses.size(); passIndex++) {
        const auto& pass = _orderedPasses[passIndex];

        for (i32 outputIndex = 0; outputIndex < pass->_outputs.size(); outputIndex++) {
            // const auto& currentAccess = pass->_outputs[outputIndex];
            const auto& [currentState, currentAccess] = findCurrAccess(*pass, pass->_outputs[outputIndex].index);
            if (isDummy(currentAccess)) continue;
            // const auto& [currentState, currentAccess] = findCurrAccess(*pass, outputIndex);
            const auto [accessPassIndex, accessIndex, nextAccess] = findNextAccess(passIndex, currentAccess.index);
            if (accessPassIndex < 0 || nextAccess.access == Access::NONE)
                continue;
            const auto& accessPass = _orderedPasses[accessPassIndex];
            RenderPass::Barrier barrier = {};
            if (pass->getType() == PassType::HOST) { // if pass is host no image transitions are made so use previous layout
                if (pass->_barriers.empty()) {
                    if (_resources[currentAccess.index]->type == ResourceType::IMAGE) {
                        if (auto [prevPassIndex, prevIndex, prevAccess] = findPrevAccess(passIndex - 1, currentAccess.index); prevPassIndex < 0) {
                            barrier = {
                                nextAccess.index,
                                passIndex,
                                currentAccess.stage,
                                nextAccess.stage,
                                currentAccess.access,
                                nextAccess.access,
                                dynamic_cast<ImageResource*>(_resources[currentAccess.index].get())->initialLayout,
                                nextAccess.layout,
                                pass->getQueue(),
                                accessPass->getQueue()
                            };
                        } else {
                            barrier = {
                                nextAccess.index,
                                passIndex,
                                currentAccess.stage,
                                nextAccess.stage,
                                currentAccess.access,
                                nextAccess.access,
                                ImageLayout::GENERAL,
                                nextAccess.layout,
                                pass->getQueue(),
                                accessPass->getQueue()
                            };
                        }
                    }
                } else {
                    for (auto& b : pass->_barriers) {
                        if (b.index == currentAccess.index && b.srcLayout != b.dstLayout) {
                            barrier = {
                                nextAccess.index,
                                passIndex,
                                b.srcStage,
                                nextAccess.stage,
                                b.srcAccess,
                                nextAccess.access,
                                ImageLayout::GENERAL,
                                nextAccess.layout,
                                b.srcQueue,
                                accessPass->getQueue()
                            };
                            break;
                        }
                    }
                }
            } else {
                barrier = {
                    nextAccess.index,
                    passIndex,
                    currentAccess.stage,
                    nextAccess.stage,
                    currentAccess.access,
                    nextAccess.access,
                    currentAccess.layout,
                    nextAccess.layout,
                    pass->getQueue(),
                    accessPass->getQueue()
                };
            }
            accessPass->_barriers.push_back(barrier);
            if (pass->getQueue() != accessPass->getQueue()) {
                pass->_releaseBarriers.push_back(barrier);
                accessPass->_waits.push_back({passIndex, pass->getQueue()});
                pass->_signal = true;
            }
        }
        for (i32 inputIndex = 0; inputIndex < pass->_inputs.size(); inputIndex++) {
            // const auto& currentAccess = pass->_inputs[inputIndex];
            const auto& [currentState, currentAccess] = findCurrAccess(*pass, pass->_inputs[inputIndex].index);
            if (isDummy(currentAccess)) continue;
            if (writesResource(*pass, currentAccess.index) || isDummy(currentAccess))
                continue;

            const auto [accessPassIndex, accessIndex, nextAccess] = findNextAccess(passIndex, currentAccess.index);
            if (accessPassIndex < 0 || nextAccess.access == Access::NONE)
                continue;
            const auto& accessPass = _orderedPasses[accessPassIndex];
            RenderPass::Barrier barrier = {};
            if (pass->getType() == PassType::HOST) { // if pass is host no image transitions are made so use previous layout
                if (pass->_barriers.empty()) {
                    if (_resources[currentAccess.index]->type == ResourceType::IMAGE) {
                        if (auto [prevPassIndex, prevIndex, prevAccess] = findPrevAccess(passIndex - 1, currentAccess.index); prevPassIndex < 0) {
                            barrier = {
                                nextAccess.index,
                                passIndex,
                                currentAccess.stage,
                                nextAccess.stage,
                                currentAccess.access,
                                nextAccess.access,
                                dynamic_cast<ImageResource*>(_resources[currentAccess.index].get())->initialLayout,
                                nextAccess.layout,
                                pass->getQueue(),
                                accessPass->getQueue()
                            };
                        } else {
                            barrier = {
                                nextAccess.index,
                                passIndex,
                                currentAccess.stage,
                                nextAccess.stage,
                                currentAccess.access,
                                nextAccess.access,

                                ImageLayout::GENERAL,
                                nextAccess.layout,
                                pass->getQueue(),
                                accessPass->getQueue()
                            };
                        }
                    }
                } else {
                    for (auto& b : pass->_barriers) {
                        if (b.index == currentAccess.index && b.srcLayout != b.dstLayout) {
                            barrier = {
                                nextAccess.index,
                                passIndex,
                                b.srcStage,
                                nextAccess.stage,
                                b.srcAccess,
                                nextAccess.access,
                                ImageLayout::GENERAL,
                                nextAccess.layout,
                                b.srcQueue,
                                accessPass->getQueue()
                            };
                            break;
                        }
                    }
                }
            } else {
                barrier = {
                    nextAccess.index,
                    passIndex,
                    currentAccess.stage,
                    nextAccess.stage,
                    currentAccess.access,
                    nextAccess.access,
                    currentAccess.layout,
                    nextAccess.layout,
                    pass->getQueue(),
                    accessPass->getQueue()
                };
            }
            accessPass->_barriers.push_back(barrier);
            if (pass->getQueue() != accessPass->getQueue()) {
                pass->_releaseBarriers.push_back(barrier);
                accessPass->_waits.push_back({passIndex, pass->getQueue()});
                pass->_signal = true;
            }
        }
    }

    // find first access of resources
    for (u32 i = 0; i < _resources.size(); i++) {
        const auto [accessPassIndex, accessIndex, nextAccess] = findNextAccess(-1, i);
        if (accessPassIndex < 0 || nextAccess.access == Access::NONE)
            continue;
        const auto& accessPass = _orderedPasses[accessPassIndex];
        const auto& resource = _resources[i];
        auto initialLayout = ImageLayout::UNDEFINED;
        if (resource->type == ResourceType::IMAGE) {
            initialLayout = dynamic_cast<ImageResource*>(resource.get())->initialLayout;
        }
        accessPass->_barriers.push_back(RenderPass::Barrier{
            nextAccess.index,
            -1,
            PipelineStage::TOP,
            nextAccess.stage,
            Access::MEMORY_READ | Access::MEMORY_WRITE,
            nextAccess.access,
            initialLayout,
            accessPass->getType() == PassType::HOST ? initialLayout : nextAccess.layout
        });
    }

    if (_backbufferFinalLayout != ImageLayout::UNDEFINED) {
        const auto [accessPassIndex, accessIndex, prevAccess] = findPrevAccess(_orderedPasses.size() - 1, _backbufferIndex);
        _backbufferBarrier = {
            .index = prevAccess.index,
            .passIndex = -1,
            .srcStage = prevAccess.stage,
            .dstStage = PipelineStage::BOTTOM,
            .srcAccess = prevAccess.access,
            .dstAccess = Access::MEMORY_WRITE | Access::MEMORY_READ,
            .srcLayout = prevAccess.layout,
            .dstLayout = _backbufferFinalLayout
        };
    }
}

void canta::RenderGraph::buildResources() {
    for (const auto& resource : _resources) {
        if (resource->type == ResourceType::IMAGE) {
            const auto imageResource = dynamic_cast<ImageResource*>(resource.get());
            if (imageResource->matchesBackbuffer) {
                auto backbufferImage = getImage({ .index = static_cast<u32>(_backbufferIndex) });
                if (!backbufferImage) {
                    _device->logger().error("invalid backbuffer in rendergraph");
                    return;
                }
                imageResource->width = backbufferImage->width();
                imageResource->height = backbufferImage->height();
                imageResource->depth = 1;
            }

            const auto image = _images[imageResource->imageIndex];
            if (!image && (!image || (image->usage() & imageResource->usage) != imageResource->usage ||
                image->width() != imageResource->width ||
                image->height() != imageResource->height ||
                image->depth() != imageResource->depth ||
                image->mips() != imageResource->mipLevels ||
                image->format() != imageResource->format)) {
                _images[imageResource->imageIndex] = _device->createImage({
                    .width = imageResource->width,
                    .height = imageResource->height,
                    .depth = imageResource->depth,
                    .format = imageResource->format,
                    .mipLevels = imageResource->mipLevels,
                    .allocateMipViews = true,
                    .usage = imageResource->usage,
                    .name = imageResource->name
                });
            }
        } else {
            const auto bufferResource = dynamic_cast<BufferResource*>(resource.get());

            const auto buffer = _buffers[bufferResource->bufferIndex];
            if (!buffer && !buffer || buffer->size() < bufferResource->size ||
                (buffer->usage() & bufferResource->usage) != bufferResource->usage) {
                _buffers[bufferResource->bufferIndex] = _device->createBuffer({
                    .size = bufferResource->size,
                    .usage = bufferResource->usage,
                    .type = bufferResource->memoryType,
                    .name = bufferResource->name
                });
            }
        }
    }
}

void canta::RenderGraph::buildRenderAttachments() {
    for (u32 i = 0; i < _orderedPasses.size(); i++) {
        auto& pass = _orderedPasses[i];

        pass->_renderingColourAttachments.clear();

        for (const auto& attachment : pass->_colourAttachments) {
            const auto image = dynamic_cast<ImageResource*>(_resources[attachment.index].get());
            const auto [prevAccessPassIndex, prevAccessIndex, prevAccess] = findPrevAccess(i - 1, image->index);
            const auto [read, access] = findCurrAccess(*pass, image->index);
            const auto [nextAccessPassIndex, nextAccessIndex, nextAccess] = findNextAccess(i, image->index);

            canta::Attachment renderingAttachment = {};
            renderingAttachment.image = _images[image->imageIndex];
            renderingAttachment.imageLayout = attachment.layout;
            renderingAttachment.loadOp = prevAccessPassIndex > -1 ? LoadOp::LOAD : LoadOp::CLEAR;
            renderingAttachment.storeOp = nextAccessPassIndex > -1 ? StoreOp::STORE : StoreOp::NONE;
            renderingAttachment.clearColour = attachment.clearColor;
            pass->_renderingColourAttachments.push_back(renderingAttachment);
        }
        if (pass->_depthAttachment.index > -1) {

            const auto image = dynamic_cast<ImageResource*>(_resources[pass->_depthAttachment.index].get());
            const auto [prevAccessPassIndex, prevAccessIndex, prevAccess] = findPrevAccess(i - 1, image->index);
            const auto [read, access] = findCurrAccess(*pass, image->index);
            const auto [nextAccessPassIndex, nextAccessIndex, nextAccess] = findNextAccess(i, image->index);

            canta::Attachment renderingAttachment = {};
            renderingAttachment.image = _images[image->imageIndex];
            renderingAttachment.imageLayout = pass->_depthAttachment.layout;
            renderingAttachment.loadOp = prevAccessPassIndex > -1 ? LoadOp::LOAD : LoadOp::CLEAR;
            renderingAttachment.storeOp = nextAccessPassIndex > -1 ? StoreOp::STORE : StoreOp::NONE;
            renderingAttachment.clearColour = pass->_depthAttachment.clearColor;
            pass->_renderingDepthAttachment = renderingAttachment;
        }
    }
}

auto canta::RenderGraph::statistics() const -> Statistics {
    return {
        .passes = static_cast<u32>(_orderedPasses.size()),
        .resources = static_cast<u32>(_resources.size()),
        .images = static_cast<u32>(_images.size()),
        .buffers = static_cast<u32>(_buffers.size()),
        .commandBuffers = _commandPools[_device->flyingIndex()][0].index() + _commandPools[_device->flyingIndex()][1].index(),
    };
}

auto canta::RenderGraph::getPass(std::string_view name) const -> std::optional<RenderPass*> {
    for (auto& pass : _orderedPasses) {
        if (pass->name() == name) return pass;
    }
    return std::nullopt;
}


auto canta::RenderGraph::findNextAccess(const i32 startIndex, const u32 resource, bool prioritiseInputs) const -> std::tuple<i32, i32, ResourceAccess> {
    for (i32 passIndex = startIndex + 1; passIndex < _orderedPasses.size(); passIndex++) {
        const auto& pass = _orderedPasses[passIndex];
        if (prioritiseInputs) {
            for (i32 inputIndex = 0; inputIndex < pass->_inputs.size(); inputIndex++) {
                if (pass->_inputs[inputIndex].index == resource) {
                    if (!isDummy(pass->_inputs[inputIndex]))
                        return { passIndex, inputIndex, pass->_inputs[inputIndex] };
                    break;
                }
            }
        }
        for (i32 outputIndex = 0; outputIndex < pass->_outputs.size(); outputIndex++) {
            if (pass->_outputs[outputIndex].index == resource) {
                if (!isDummy(pass->_outputs[outputIndex]))
                    return { passIndex, outputIndex, pass->_outputs[outputIndex] };
                break;
            }
        }
        if (!prioritiseInputs) {
            for (i32 inputIndex = 0; inputIndex < pass->_inputs.size(); inputIndex++) {
                if (pass->_inputs[inputIndex].index == resource) {
                    if (!isDummy(pass->_inputs[inputIndex]))
                        return { passIndex, inputIndex, pass->_inputs[inputIndex] };
                    break;
                }
            }
        }
    }
    return { -1, -1, {} };
};

auto canta::RenderGraph::findCurrAccess(const RenderPass& pass, const u32 resource) const -> std::tuple<bool, ResourceAccess> {
    for (auto& input : pass._inputs) {
        if (input.index == resource)
            return { true, input };
    }
    for (auto& output : pass._outputs) {
        if (output.index == resource)
            return { false, output };
    }
    return { false, {} };
};

auto canta::RenderGraph::findPrevAccess(const i32 startIndex, const u32 resource) const -> std::tuple<i32, i32, ResourceAccess> {
    for (i32 passIndex = startIndex; passIndex > -1; passIndex--) {
        const auto& pass = _orderedPasses[passIndex];
        for (i32 outputIndex = 0; outputIndex < pass->_outputs.size(); outputIndex++) {
            if (pass->_outputs[outputIndex].index == resource)
                return { passIndex, outputIndex, pass->_outputs[outputIndex] };
        }
        for (i32 inputIndex = 0; inputIndex < pass->_inputs.size(); inputIndex++) {
            if (pass->_inputs[inputIndex].index == resource)
                return { passIndex, inputIndex, pass->_inputs[inputIndex] };
        }
    }
    return { -1, -1, {} };
};