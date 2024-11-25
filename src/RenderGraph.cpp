#include "Canta/RenderGraph.h"


auto canta::RenderPass::addColourWrite(canta::ImageIndex index, const ClearValue &clearColor) -> RenderPass& {
    assert(index.id >= 0);
    if (auto resource = writes(index, Access::COLOUR_WRITE | Access::COLOUR_READ,
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

auto canta::RenderPass::addColourRead(canta::ImageIndex index) -> RenderPass& {
    assert(index.id >= 0);
    if (auto resource = reads(index, Access::COLOUR_READ,
                              PipelineStage::COLOUR_OUTPUT,
                              ImageLayout::COLOUR_ATTACHMENT)) {
        dynamic_cast<ImageResource*>(resource)->usage |= ImageUsage::COLOUR_ATTACHMENT;
    }
    return *this;
}

auto canta::RenderPass::addDepthWrite(canta::ImageIndex index, const ClearValue& clearColor) -> RenderPass& {
    assert(index.id >= 0);
    if (auto resource = writes(index, Access::DEPTH_STENCIL_WRITE | Access::DEPTH_STENCIL_READ,
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

auto canta::RenderPass::addDepthRead(canta::ImageIndex index) -> RenderPass& {
    assert(index.id >= 0);
    if (auto resource = reads(index, Access::DEPTH_STENCIL_READ,
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

auto canta::RenderPass::addStorageImageWrite(canta::ImageIndex index, canta::PipelineStage stage) -> RenderPass& {
    assert(index.id >= 0);
    if (auto resource = writes(index, Access::SHADER_WRITE | Access::SHADER_READ,
                               stage, ImageLayout::GENERAL)) {
        dynamic_cast<ImageResource*>(resource)->usage |= ImageUsage::STORAGE;
    }
    return *this;
}

auto canta::RenderPass::addStorageImageRead(canta::ImageIndex index, canta::PipelineStage stage) -> RenderPass& {
    assert(index.id >= 0);
    if (auto resource = reads(index, Access::SHADER_READ,
                              stage, ImageLayout::GENERAL)) {
        dynamic_cast<ImageResource*>(resource)->usage |= ImageUsage::STORAGE;
    }
    return *this;
}

auto canta::RenderPass::addStorageBufferWrite(canta::BufferIndex index, canta::PipelineStage stage) -> RenderPass& {
    assert(index.id >= 0);
    if (auto resource = writes(index, Access::SHADER_WRITE | Access::SHADER_READ,
                               stage)) {
        dynamic_cast<BufferResource*>(resource)->usage |= BufferUsage::STORAGE;
    }
    return *this;
}

auto canta::RenderPass::addStorageBufferRead(canta::BufferIndex index, canta::PipelineStage stage) -> RenderPass& {
    assert(index.id >= 0);
    if (auto resource = reads(index, Access::SHADER_READ,
                              stage)) {
        dynamic_cast<BufferResource*>(resource)->usage |= BufferUsage::STORAGE;
    }
    return *this;
}

auto canta::RenderPass::addSampledRead(canta::ImageIndex index, canta::PipelineStage stage) -> RenderPass& {
    assert(index.id >= 0);
    if (auto resource = reads(index, Access::SHADER_READ,
                              stage, ImageLayout::SHADER_READ_ONLY)) {
        dynamic_cast<ImageResource*>(resource)->usage |= ImageUsage::SAMPLED;
    }
    return *this;
}

auto canta::RenderPass::addBlitWrite(canta::ImageIndex index) -> RenderPass& {
    assert(index.id >= 0);
    if (auto resource = writes(index, Access::TRANSFER_WRITE | Access::TRANSFER_READ,
                               PipelineStage::TRANSFER,
                               ImageLayout::TRANSFER_DST)) {
        dynamic_cast<ImageResource*>(resource)->usage |= ImageUsage::TRANSFER_DST;
    }
    return *this;
}

auto canta::RenderPass::addBlitRead(canta::ImageIndex index) -> RenderPass& {
    assert(index.id >= 0);
    if (auto resource = reads(index, Access::TRANSFER_READ,
                              PipelineStage::TRANSFER,
                              ImageLayout::TRANSFER_SRC)) {
        dynamic_cast<ImageResource*>(resource)->usage |= ImageUsage::TRANSFER_SRC;
    }
    return *this;
}

auto canta::RenderPass::addTransferWrite(canta::ImageIndex index) -> RenderPass& {
    assert(index.id >= 0);
    if (auto resource = writes(index, Access::TRANSFER_WRITE | Access::TRANSFER_READ,
                               PipelineStage::TRANSFER,
                               ImageLayout::TRANSFER_DST)) {
        dynamic_cast<ImageResource*>(resource)->usage |= ImageUsage::TRANSFER_DST;
    }
    return *this;
}

auto canta::RenderPass::addTransferRead(canta::ImageIndex index) -> RenderPass& {
    assert(index.id >= 0);
    if (auto resource = reads(index, Access::TRANSFER_READ,
                              PipelineStage::TRANSFER,
                              ImageLayout::TRANSFER_SRC)) {
        dynamic_cast<ImageResource*>(resource)->usage |= ImageUsage::TRANSFER_SRC;
    }
    return *this;
}

auto canta::RenderPass::addTransferWrite(canta::BufferIndex index) -> RenderPass& {
    assert(index.id >= 0);
    if (auto resource = writes(index, Access::TRANSFER_WRITE | Access::TRANSFER_READ,
                               PipelineStage::TRANSFER)) {
        dynamic_cast<BufferResource*>(resource)->usage |= BufferUsage::TRANSFER_DST;
    }
    return *this;
}

auto canta::RenderPass::addTransferRead(canta::BufferIndex index) -> RenderPass& {
    assert(index.id >= 0);
    if (auto resource = reads(index, Access::TRANSFER_READ,
                              PipelineStage::TRANSFER)) {
        dynamic_cast<BufferResource*>(resource)->usage |= BufferUsage::TRANSFER_SRC;
    }
    return *this;
}

auto canta::RenderPass::addIndirectRead(canta::BufferIndex index) -> RenderPass& {
    assert(index.id >= 0);
    if (auto resource = reads(index, Access::INDIRECT,
                              PipelineStage::DRAW_INDIRECT)) {
        dynamic_cast<BufferResource*>(resource)->usage |= BufferUsage::INDIRECT;
    }
    return *this;
}


auto canta::RenderPass::writes(canta::ImageIndex index, canta::Access access, canta::PipelineStage stage, canta::ImageLayout layout) -> Resource * {
    assert(index.id >= 0);
    auto resource = _graph->_resources[index.index].get();
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

auto canta::RenderPass::reads(canta::ImageIndex index, canta::Access access, canta::PipelineStage stage, canta::ImageLayout layout) -> Resource * {
    assert(index.id >= 0);
    auto resource = _graph->_resources[index.index].get();
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

auto canta::RenderPass::writes(canta::BufferIndex index, canta::Access access, canta::PipelineStage stage) -> Resource * {
    assert(index.id >= 0);
    auto resource = _graph->_resources[index.index].get();
    if (resource) {
        _outputs.push_back({
            .id = index.id,
            .index = index.index,
            .access = access,
            .stage = stage
        });
    }
    return resource;
}

auto canta::RenderPass::reads(canta::BufferIndex index, canta::Access access, canta::PipelineStage stage) -> Resource * {
    assert(index.id >= 0);
    auto resource = _graph->_resources[index.index].get();
    if (resource) {
        _inputs.push_back({
            .id = index.id,
            .index = index.index,
            .access = access,
            .stage = stage
        });
    }
    return resource;
}

auto canta::RenderPass::aliasImageOutputs() const -> std::vector<ImageIndex> {
    std::vector<ImageIndex> aliases = {};
    for (auto& output : _outputs) {
        auto& resource = _graph->_resources[output.index];
        if (resource->type == ResourceType::IMAGE)
            aliases.push_back(_graph->addAlias(ImageIndex{ .id = output.id, .index = output.index }));
    }
    return aliases;
}

auto canta::RenderPass::aliasBufferOutputs() const -> std::vector<BufferIndex> {
    std::vector<BufferIndex> aliases = {};
    for (auto& output : _outputs) {
        auto& resource = _graph->_resources[output.index];
        if (resource->type == ResourceType::BUFFER)
            aliases.push_back(_graph->addAlias(BufferIndex{ .id = output.id, .index = output.index }));
    }
    return aliases;
}

auto canta::RenderGraph::create(canta::RenderGraph::CreateInfo info) -> RenderGraph {
    RenderGraph graph = {};

    graph._device = info.device;
    graph._name = info.name;
    for (auto& pool : graph._commandPools) {
        pool = info.device->createCommandPool({
            .queueType = QueueType::GRAPHICS
        }).value();
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

    return graph;
}

auto canta::RenderGraph::addPass(std::string_view name, PassType type, RenderGroup group) -> RenderPass & {
    u32 index = _passes.size();
    _passes.emplace_back();
    _passes.back()._graph = this;
    _passes.back()._name = name;
    _passes.back()._type = type;
    _passes.back().setGroup(group);
    if (_timingEnabled && _timingMode != TimingMode::SINGLE) {
        if (_timers[_device->flyingIndex()].size() <= index) {
            _timers[_device->flyingIndex()].emplace_back(std::make_pair(name, _device->createTimer()));
        } else
            _timers[_device->flyingIndex()][index].first = name;
    }
    if (_pipelineStatisticsEnabled && _individualPipelineStatistics) {
        if (_pipelineStats[_device->flyingIndex()].size() <= index) {
            _pipelineStats[_device->flyingIndex()].emplace_back(std::make_pair(name, _device->createPipelineStatistics()));
        } else
            _pipelineStats[_device->flyingIndex()][index].first = name;
    }
    return _passes.back();
}

auto canta::RenderGraph::addPass(canta::RenderPass &&pass) -> RenderPass & {
    u32 index = _passes.size();
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

auto canta::RenderGraph::addClearPass(std::string_view name, canta::ImageIndex index, const ClearValue& value, RenderGroup group) -> RenderPass & {
    auto& clearPass = addPass(name, PassType::TRANSFER, group);
    clearPass.addTransferWrite(index);
    clearPass.setExecuteFunction([index, value] (CommandBuffer& cmd, RenderGraph& graph) {
        auto image = graph.getImage(index);
        cmd.clearImage(image, ImageLayout::TRANSFER_DST, value);
    });
    return clearPass;
}

auto canta::RenderGraph::addBlitPass(std::string_view name, canta::ImageIndex src, canta::ImageIndex dst, Filter filter, RenderGroup group) -> RenderPass & {
    auto& blitPass = addPass(name, PassType::TRANSFER, group);
    blitPass.addTransferRead(src);
    blitPass.addTransferWrite(dst);
    blitPass.setExecuteFunction([src, dst, filter] (CommandBuffer& cmd, RenderGraph& graph) {
        auto srcImage = graph.getImage(src);
        auto dstImage = graph.getImage(dst);
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

auto canta::RenderGraph::getGroup(std::string_view name, const std::array<f32, 4>& colour) -> RenderGroup {
    auto it = _renderGroups.find(name);
    if (it != _renderGroups.end()) {
        return {
            .id = it->second.first
        };
    }

    auto it1 = _renderGroups.insert(std::make_pair(name, std::make_pair(_groupId++, colour)));
    return {
        .id = it1.first->second.first
    };
}

auto canta::RenderGraph::getGroupName(canta::RenderGroup group) -> std::string {
    for (auto [key, value] : _renderGroups) {
        if (value.first == group.id)
            return key.data();
    }
    return {};
}

auto canta::RenderGraph::getGroupColour(canta::RenderGroup group) -> std::array<f32, 4> {
    for (auto [key, value] : _renderGroups) {
        if (value.first == group.id)
            return value.second;
    }
    return { 0, 1, 0, 1 };
}

auto canta::RenderGraph::addImage(canta::ImageDescription description) -> ImageIndex {
    auto it = _nameToIndex.find(description.name.data());
    if (it != _nameToIndex.end()) {
        u32 index = it->second;
        auto imageResource = dynamic_cast<ImageResource*>(_resources[index].get());
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
            u32 imageIndex = imageResource->imageIndex;
            _images[imageIndex] = description.handle;
        }
        return {
            .id = _resourceId++,
            .index = index
        };
    }

    u32 index = _resources.size();
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

auto canta::RenderGraph::addBuffer(canta::BufferDescription description) -> BufferIndex {
    auto it = _nameToIndex.find(description.name.data());
    if (it != _nameToIndex.end()) {
        u32 index = it->second;
        auto bufferResource = dynamic_cast<BufferResource*>(_resources[index].get());
        if (description.handle) {
            bufferResource->size = description.handle->size();
            bufferResource->usage = description.handle->usage();
        } else {
            bufferResource->size = description.size;
            bufferResource->usage = description.usage;
        }
        if (description.handle) {
            u32 bufferIndex = bufferResource->bufferIndex;
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

auto canta::RenderGraph::addAlias(canta::ImageIndex index) -> ImageIndex {
    return {
        .id = _resourceId++,
        .index = index.index
    };
}

auto canta::RenderGraph::addAlias(canta::BufferIndex index) -> BufferIndex {
    return {
        .id = _resourceId++,
        .index = index.index
    };
}

auto canta::RenderGraph::getImage(canta::ImageIndex index) -> ImageHandle {
    auto imageIndex = dynamic_cast<ImageResource*>(_resources[index.index].get())->imageIndex;
    return _images[imageIndex];
}

auto canta::RenderGraph::getBuffer(canta::BufferIndex index) -> BufferHandle {
    auto bufferIndex = dynamic_cast<BufferResource*>(_resources[index.index].get())->bufferIndex;
    return _buffers[bufferIndex];
}

void canta::RenderGraph::setBackbuffer(canta::ImageIndex index, ImageLayout finalLayout) {
    _backbufferId = index.id;
    _backbufferIndex = index.index;
    _backbufferFinalLayout = finalLayout;
}

void canta::RenderGraph::setBackbuffer(canta::BufferIndex index) {
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
        auto& pass = _passes[i];
        for (auto& output : pass._outputs)
            outputs[output.id].push_back(i);
    }

    std::vector<bool> visited(_passes.size(), false);
    std::vector<bool> onStack(_passes.size(), false);

    std::function<bool(u32)> dfs = [&](u32 index) -> bool {
        visited[index] = true;
        onStack[index] = true;
        auto& pass = _passes[index];
        for (auto& input : pass._inputs) {
            for (auto& output : outputs[input.id]) {
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

    for (auto& pass : outputs[_backbufferId]) {
        if (!dfs(pass)) {
            _device->logger().error("circular dependencies found in rendergraph");
            return std::unexpected(RenderGraphError::CYCLICAL_GRAPH);
        }
    }

    buildBarriers();
    buildResources();
    buildRenderAttachments();

    return true;
}

auto canta::RenderGraph::execute(std::span<Semaphore::Pair> waits, std::span<Semaphore::Pair> signals, std::span<ImageBarrier> imagesToAcquire) -> std::expected<bool, RenderGraphError> {
    _timerCount = 0;
    RenderGroup currentGroup = {};
    bool groupChanged = false;
    _commandPools[_device->flyingIndex()].reset();
    auto& cmd = _commandPools[_device->flyingIndex()].getBuffer();
    cmd.begin();
    if (_timingEnabled && _timingMode == TimingMode::SINGLE) {
        _timers[_device->flyingIndex()].front().first = _name;
        _timers[_device->flyingIndex()].front().second.begin(cmd);
    }
    if (_pipelineStatisticsEnabled && !_individualPipelineStatistics) {
        _pipelineStats[_device->flyingIndex()].front().first = _name;
        _pipelineStats[_device->flyingIndex()].front().second.begin(cmd);
    }

    for (auto& image : imagesToAcquire) {
        cmd.barrier(image);
    }

    for (u32 i = 0; i < _orderedPasses.size(); i++) {
        auto& pass = _orderedPasses[i];

        // if render group changes
        if (currentGroup.id != pass->getGroup().id) {
            if (currentGroup.id >= 0)
                cmd.popDebugLabel();
            groupChanged = true;
            if (pass->getGroup().id >= 0)
                cmd.pushDebugLabel(getGroupName(pass->getGroup()), getGroupColour(pass->getGroup()));
        } else
            groupChanged = false;

        cmd.pushDebugLabel(pass->_name, pass->_debugColour);

        u32 imageBarrierCount = 0;
        ImageBarrier imageBarriers[pass->_barriers.size()];
        u32 bufferBarrierCount = 0;
        BufferBarrier bufferBarriers[pass->_barriers.size()];

        for (auto& barrier : pass->_barriers) {
            auto* resource = _resources[barrier.index].get();
            if (resource->type == ResourceType::IMAGE) {
                auto image = getImage({ .index = barrier.index });
                imageBarriers[imageBarrierCount++] = ImageBarrier{
                    .image = image,
                    .srcStage = barrier.srcStage,
                    .dstStage = barrier.dstStage,
                    .srcAccess = barrier.srcAccess,
                    .dstAccess = barrier.dstAccess,
                    .srcLayout = barrier.srcLayout,
                    .dstLayout = barrier.dstLayout
                };
            } else {
                auto buffer = getBuffer({ .index = barrier.index });
                bufferBarriers[bufferBarrierCount++] = BufferBarrier{
                    .buffer = buffer,
                    .srcStage = barrier.srcStage,
                    .dstStage = barrier.dstStage,
                    .srcAccess = barrier.srcAccess,
                    .dstAccess = barrier.dstAccess,
                };
            }
        }
        for (u32 barrier = 0; barrier < imageBarrierCount; barrier++)
            cmd.barrier(imageBarriers[barrier]);
        for (u32 barrier = 0; barrier < bufferBarrierCount; barrier++)
            cmd.barrier(bufferBarriers[barrier]);

        if (_timingEnabled && _timingMode != TimingMode::SINGLE) {
            if (_timingMode == TimingMode::PER_GROUP && groupChanged) {
                if (currentGroup.id >= 0) {
                    _timers[_device->flyingIndex()][_timerCount].second.end(cmd);
                    _timerCount++;
                }
                if (pass->getGroup().id >= 0) {
                    _timers[_device->flyingIndex()][_timerCount].first = getGroupName(pass->getGroup());
                    _timers[_device->flyingIndex()][_timerCount].second.begin(cmd);
                }
            }
            if (_timingMode == TimingMode::PER_PASS || pass->getGroup().id < 0) {
                _timers[_device->flyingIndex()][_timerCount].first = pass->_name;
                _timers[_device->flyingIndex()][_timerCount].second.begin(cmd);
            }
        }
        if (_pipelineStatisticsEnabled && _individualPipelineStatistics)
            _pipelineStats[_device->flyingIndex()][i].second.begin(cmd);

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
            cmd.beginRendering(info);
        }
        pass->_execute(cmd, *this);
        if (pass->_type == PassType::GRAPHICS) {
            cmd.endRendering();
        }
        if (_pipelineStatisticsEnabled && _individualPipelineStatistics)
            _pipelineStats[_device->flyingIndex()][i].second.end(cmd);
        if (_timingEnabled && _timingMode != TimingMode::SINGLE) {
            if (_timingMode == TimingMode::PER_PASS || pass->getGroup().id < 0) {
                _timers[_device->flyingIndex()][_timerCount++].second.end(cmd);
            }
        }
        cmd.popDebugLabel();
        currentGroup = pass->getGroup();
    }

    if (_backbufferFinalLayout != ImageLayout::UNDEFINED) {
        cmd.barrier({
            .image = getImage({ .index = static_cast<u32>(_backbufferIndex)}),
            .srcStage = _backbufferBarrier.srcStage,
            .dstStage = _backbufferBarrier.dstStage,
            .srcAccess = _backbufferBarrier.srcAccess,
            .dstAccess = _backbufferBarrier.dstAccess,
            .srcLayout = _backbufferBarrier.srcLayout,
            .dstLayout = _backbufferBarrier.dstLayout
        });
    }

    if (_pipelineStatisticsEnabled && !_individualPipelineStatistics)
        _pipelineStats[_device->flyingIndex()].front().second.end(cmd);
    if (_timingEnabled && _timingMode == TimingMode::SINGLE)
        _timers[_device->flyingIndex()].front().second.end(cmd);
    cmd.end();
    cmd.submit(waits, signals);

    return true;
}

void canta::RenderGraph::buildBarriers() {
    auto findNextAccess = [&](i32 startIndex, u32 resource) -> std::tuple<i32, i32, ResourceAccess> {
        for (i32 passIndex = startIndex + 1; passIndex < _orderedPasses.size(); passIndex++) {
            auto& pass = _orderedPasses[passIndex];
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
    auto findPrevAccess = [&](i32 startIndex, u32 resource) -> std::tuple<i32, i32, ResourceAccess> {
        for (i32 passIndex = startIndex; passIndex > -1; passIndex--) {
            auto& pass = _orderedPasses[passIndex];
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
    auto readsResource = [&](RenderPass& pass, u32 resource) -> bool {
        for (auto& access : pass._inputs) {
            if (access.index == resource)
                return true;
        }
        return false;
    };
    auto writesResource = [&](RenderPass& pass, u32 resource) -> bool {
        for (auto& access : pass._outputs) {
            if (access.index == resource)
                return true;
        }
        return false;
    };

    // for each pass for each input/output find next pass that access that resource and add barrier to next pass
    for (i32 passIndex = 0; passIndex < _orderedPasses.size(); passIndex++) {
        auto& pass = _orderedPasses[passIndex];

        for (i32 outputIndex = 0; outputIndex < pass->_outputs.size(); outputIndex++) {
            auto& currentAccess = pass->_outputs[outputIndex];
            auto [accessPassIndex, accessIndex, nextAccess] = findNextAccess(passIndex, currentAccess.index);
            if (accessPassIndex < 0)
                continue;
            auto& accessPass = _orderedPasses[accessPassIndex];
            accessPass->_barriers.push_back({
                nextAccess.index,
                currentAccess.stage,
                nextAccess.stage,
                currentAccess.access,
                nextAccess.access,
                currentAccess.layout,
                nextAccess.layout
            });
        }
        for (i32 inputIndex = 0; inputIndex < pass->_inputs.size(); inputIndex++) {
            auto& currentAccess = pass->_inputs[inputIndex];
            if (writesResource(*pass, currentAccess.index))
                continue;

            auto [accessPassIndex, accessIndex, nextAccess] = findNextAccess(passIndex, currentAccess.index);
            if (accessPassIndex < 0)
                continue;
            auto& accessPass = _orderedPasses[accessPassIndex];
            accessPass->_barriers.push_back({
                nextAccess.index,
                currentAccess.stage,
                nextAccess.stage,
                currentAccess.access,
                nextAccess.access,
                currentAccess.layout,
                nextAccess.layout
            });
        }
    }

    // find first access of resources
    for (u32 i = 0; i < _resources.size(); i++) {
        auto [accessPassIndex, accessIndex, nextAccess] = findNextAccess(-1, i);
        if (accessPassIndex < 0)
            continue;
        auto& accessPass = _orderedPasses[accessPassIndex];
        auto& resource = _resources[i];
        ImageLayout initialLayout = ImageLayout::UNDEFINED;
        if (resource->type == ResourceType::IMAGE) {
            initialLayout = dynamic_cast<ImageResource*>(resource.get())->initialLayout;
        }
        accessPass->_barriers.push_back({
            nextAccess.index,
            PipelineStage::TOP,
            nextAccess.stage,
            Access::MEMORY_READ | Access::MEMORY_WRITE,
            nextAccess.access,
            initialLayout,
            nextAccess.layout
        });
    }

    if (_backbufferFinalLayout != ImageLayout::UNDEFINED) {
        auto [accessPassIndex, accessIndex, prevAccess] = findPrevAccess(_orderedPasses.size() - 1, _backbufferIndex);
        _backbufferBarrier = {
            .index = prevAccess.index,
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
    for (auto& resource : _resources) {
        if (resource->type == ResourceType::IMAGE) {
            auto imageResource = dynamic_cast<ImageResource*>(resource.get());
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

            auto image = _images[imageResource->imageIndex];
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
            auto bufferResource = dynamic_cast<BufferResource*>(resource.get());

            auto buffer = _buffers[bufferResource->bufferIndex];
            if (!buffer && !buffer || buffer->size() < bufferResource->size ||
                (buffer->usage() & bufferResource->usage) != bufferResource->usage) {
                _buffers[bufferResource->bufferIndex] = _device->createBuffer({
                    .size = bufferResource->size,
                    .usage = bufferResource->usage,
                    .name = bufferResource->name
                });
            }
        }
    }
}

void canta::RenderGraph::buildRenderAttachments() {
    auto findNextAccess = [&](i32 startIndex, u32 resource) -> std::tuple<i32, i32, ResourceAccess> {
        for (i32 passIndex = startIndex + 1; passIndex < _orderedPasses.size(); passIndex++) {
            auto& pass = _orderedPasses[passIndex];
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

    auto findCurrAccess = [&](RenderPass& pass, u32 resource) -> std::tuple<bool, ResourceAccess> {
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

    auto findPrevAccess = [&](i32 startIndex, u32 resource) -> std::tuple<i32, i32, ResourceAccess> {
        for (i32 passIndex = startIndex; passIndex > -1; passIndex--) {
            auto& pass = _orderedPasses[passIndex];
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

    for (u32 i = 0; i < _orderedPasses.size(); i++) {
        auto& pass = _orderedPasses[i];

        pass->_renderingColourAttachments.clear();

        for (auto& attachment : pass->_colourAttachments) {
            auto image = dynamic_cast<ImageResource*>(_resources[attachment.index].get());
            auto [prevAccessPassIndex, prevAccessIndex, prevAccess] = findPrevAccess(i - 1, image->index);
            auto [read, access] = findCurrAccess(*pass, image->index);
            auto [nextAccessPassIndex, nextAccessIndex, nextAccess] = findNextAccess(i, image->index);

            canta::Attachment renderingAttachment = {};
            renderingAttachment.image = _images[image->imageIndex];
            renderingAttachment.imageLayout = attachment.layout;
            renderingAttachment.loadOp = prevAccessPassIndex > -1 ? LoadOp::LOAD : LoadOp::CLEAR;
            renderingAttachment.storeOp = nextAccessPassIndex > -1 ? StoreOp::STORE : StoreOp::NONE;
            renderingAttachment.clearColour = attachment.clearColor;
            pass->_renderingColourAttachments.push_back(renderingAttachment);
        }
        if (pass->_depthAttachment.index > -1) {

            auto image = dynamic_cast<ImageResource*>(_resources[pass->_depthAttachment.index].get());
            auto [prevAccessPassIndex, prevAccessIndex, prevAccess] = findPrevAccess(i - 1, image->index);
            auto [read, access] = findCurrAccess(*pass, image->index);
            auto [nextAccessPassIndex, nextAccessIndex, nextAccess] = findNextAccess(i, image->index);

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
        .commandBuffers = _commandPools[_device->flyingIndex()].bufferCount()
    };
}