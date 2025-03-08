#include "Canta/RenderGraph.h"


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

auto canta::RenderPass::addStorageImageWrite(const canta::ImageIndex index, const canta::PipelineStage stage) -> RenderPass& {
    assert(index.id >= 0);
    if (const auto resource = writes(index, stage == PipelineStage::HOST ? Access::HOST_WRITE | Access::HOST_READ : Access::SHADER_WRITE | Access::SHADER_READ,
                               stage, ImageLayout::GENERAL)) {
        dynamic_cast<ImageResource*>(resource)->usage |= ImageUsage::STORAGE;
    }
    return *this;
}

auto canta::RenderPass::addStorageImageRead(const canta::ImageIndex index, const canta::PipelineStage stage) -> RenderPass& {
    assert(index.id >= 0);
    if (const auto resource = reads(index, stage == PipelineStage::HOST ? Access::HOST_READ : Access::SHADER_READ,
                              stage, ImageLayout::GENERAL)) {
        dynamic_cast<ImageResource*>(resource)->usage |= ImageUsage::STORAGE;
    }
    return *this;
}

auto canta::RenderPass::addStorageBufferWrite(const canta::BufferIndex index, const canta::PipelineStage stage) -> RenderPass& {
    assert(index.id >= 0);
    if (const auto resource = writes(index, stage == PipelineStage::HOST ? Access::HOST_WRITE | Access::HOST_READ : Access::SHADER_WRITE | Access::SHADER_READ,
                               stage)) {
        dynamic_cast<BufferResource*>(resource)->usage |= BufferUsage::STORAGE;
    }
    return *this;
}

auto canta::RenderPass::addStorageBufferRead(const canta::BufferIndex index, const canta::PipelineStage stage) -> RenderPass& {
    assert(index.id >= 0);
    if (const auto resource = reads(index, stage == PipelineStage::HOST ? Access::HOST_READ : Access::SHADER_READ,
                              stage)) {
        dynamic_cast<BufferResource*>(resource)->usage |= BufferUsage::STORAGE;
    }
    return *this;
}

auto canta::RenderPass::addSampledRead(const canta::ImageIndex index, const canta::PipelineStage stage) -> RenderPass& {
    assert(index.id >= 0);
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
    }
    return resource;
}

auto canta::RenderPass::aliasImageOutputs() const -> std::vector<ImageIndex> {
    std::vector<ImageIndex> aliases = {};
    for (const auto& output : _outputs) {
        const auto& resource = _graph->_resources[output.index];
        if (resource->type == ResourceType::IMAGE)
            aliases.push_back(_graph->addAlias(ImageIndex{ .id = output.id, .index = output.index }));
    }
    return aliases;
}

auto canta::RenderPass::aliasBufferOutputs() const -> std::vector<BufferIndex> {
    std::vector<BufferIndex> aliases = {};
    for (const auto& output : _outputs) {
        const auto& resource = _graph->_resources[output.index];
        if (resource->type == ResourceType::BUFFER)
            aliases.push_back(_graph->addAlias(BufferIndex{ .id = output.id, .index = output.index }));
    }
    return aliases;
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

auto canta::RenderGraph::execute(std::span<SemaphorePair> waits, std::span<SemaphorePair> signals, std::span<ImageBarrier> imagesToAcquire) -> std::expected<bool, RenderGraphError> {
    _timerCount = 0;
    RenderGroup currentGroup = {};
    for (auto& pool : _commandPools[_device->flyingIndex()])
        pool.reset();
    CommandBuffer* currentCommandBuffer = nullptr;
    // CommandBuffer* currentCommandBuffer = &_commandPools[_device->flyingIndex()][0].getBuffer();
    std::vector<std::pair<u32, std::pair<u32, u32>>> commandBufferIndices; // queueindex, bufferindex
    std::vector<RenderPass*> hostPasses;
    std::vector<std::vector<SemaphorePair>> commandBufferWaits = {};


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
            commandBufferIndices.push_back({getQueueIndex(QueueType::NONE), {commandBufferWaits.size(), hostPasses.size()}});
            commandBufferWaits.push_back({});
            for (const auto& wait : pass->_waits) {
                SemaphoreHandle semaphore = {};
                switch (wait.second) {
                    case QueueType::NONE:
                        semaphore = _cpuTimeline;
                    break;
                    default:
                        semaphore = _device->queue(wait.second).timeline();
                }

                commandBufferWaits.back().push_back(SemaphorePair(semaphore));
            }
            hostPasses.push_back(pass);
            continue;
        }
        if (pass->_waits.empty() && !currentCommandBuffer) {
            currentCommandBuffer = &_commandPools[_device->flyingIndex()][getQueueIndex(pass->getQueue())].getBuffer();
            commandBufferIndices.push_back({getQueueIndex(pass->getQueue()), {_commandPools[_device->flyingIndex()][getQueueIndex(pass->getQueue())].index() - 1, 0}});
            commandBufferWaits.push_back({});
            currentCommandBuffer->begin();
        }
        if (!pass->_waits.empty()) {
            if (currentCommandBuffer && currentCommandBuffer->isActive())
                currentCommandBuffer->end();

            currentCommandBuffer = &_commandPools[_device->flyingIndex()][getQueueIndex(pass->getQueue())].getBuffer();
            commandBufferIndices.push_back({getQueueIndex(pass->getQueue()), {_commandPools[_device->flyingIndex()][getQueueIndex(pass->getQueue())].index() - 1, 0}});
            commandBufferWaits.push_back({});
            for (const auto& wait : pass->_waits) {
                SemaphoreHandle semaphore = {};
                switch (wait.second) {
                    case QueueType::NONE:
                        semaphore = _cpuTimeline;
                        break;
                    default:
                        semaphore = _device->queue(wait.second).timeline();
                }

                commandBufferWaits.back().push_back(SemaphorePair(semaphore));
            }

            currentCommandBuffer->begin();
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
            currentCommandBuffer = &_commandPools[_device->flyingIndex()][getQueueIndex(pass->getQueue())].getBuffer();
            commandBufferIndices.push_back({getQueueIndex(pass->getQueue()), {_commandPools[_device->flyingIndex()][getQueueIndex(pass->getQueue())].index() - 1, 0}});
            commandBufferWaits.push_back({});
            currentCommandBuffer->begin();
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

    auto& graphicsQueue = _device->queue(QueueType::GRAPHICS);
    auto& computeQueue = _device->queue(QueueType::COMPUTE);

    u32 firstDevice = 0;
    for (i32 d = 0; d < commandBufferIndices.size(); d++) {
        if (commandBufferIndices[d].first != getQueueIndex(QueueType::NONE)) {
            firstDevice = d;
            break;
        }
    }
    u32 lastDevice = 0;
    for (i32 d = commandBufferIndices.size() - 1; d >= 0; --d) {
        if (commandBufferIndices[d].first != getQueueIndex(QueueType::NONE)) {
            lastDevice = d;
            break;
        }
    }

    for (u32 i = 0; const auto& indices : commandBufferIndices) {
        if (indices.first > 1) { // if host

            for (auto& wait : commandBufferWaits[indices.second.first]) {
                wait.semaphore->wait(wait.value);
            }
            hostPasses[indices.second.second]->_execute(*currentCommandBuffer, *this);

            _cpuTimeline->signal(_cpuTimeline->value() + 1);
            _cpuTimeline->increment();

        } else { // if device
            auto& commandList = _commandPools[_device->flyingIndex()][indices.first].buffers()[indices.second.first];

            std::vector<SemaphorePair> internalWaits = commandBufferWaits[indices.second.first];
            if (i == 0 || i == firstDevice)
                internalWaits.insert(internalWaits.end(), waits.begin(), waits.end());
            std::vector<SemaphorePair> internalSignals = {
                SemaphorePair(indices.first == 0 ? graphicsQueue.timeline() : computeQueue.timeline(), (indices.first == 0 ? graphicsQueue.timeline()->value() : computeQueue.timeline()->value()) + 1)
            };
            (indices.first == 0 ? graphicsQueue.timeline() : computeQueue.timeline())->increment();
            if (i == commandBufferIndices.size() - 1 || i == lastDevice)
                internalSignals.insert(internalSignals.end(), signals.begin(), signals.end());

            result |= (indices.first == 0 ? graphicsQueue : computeQueue).submit({ &commandList, 1 }, internalWaits, internalSignals).value();
            if (!result)
                return std::unexpected(RenderGraphError::INVALID_SUBMISSION);
        }

        i++;
    }

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
                    .srcQueue = _device->queue(barrier.srcQueue).familyIndex(),
                    .dstQueue = _device->queue(barrier.dstQueue).familyIndex(),
            };
        } else {
            const auto buffer = getBuffer({ .index = barrier.index });
            bufferBarriers[bufferBarrierCount++] = BufferBarrier{
                    .buffer = buffer,
                    .srcStage = barrier.srcStage,
                    .dstStage = barrier.dstStage,
                    .srcAccess = barrier.srcAccess,
                    .dstAccess = barrier.dstAccess,
                    .srcQueue = _device->queue(barrier.srcQueue).familyIndex(),
                    .dstQueue = _device->queue(barrier.dstQueue).familyIndex(),
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
    auto findNextAccess = [&](const i32 startIndex, const u32 resource) -> std::tuple<i32, i32, ResourceAccess> {
        for (i32 passIndex = startIndex + 1; passIndex < _orderedPasses.size(); passIndex++) {
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
    auto findPrevAccess = [&](const i32 startIndex, const u32 resource) -> std::tuple<i32, i32, ResourceAccess> {
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
            const auto& currentAccess = pass->_outputs[outputIndex];
            const auto [accessPassIndex, accessIndex, nextAccess] = findNextAccess(passIndex, currentAccess.index);
            if (accessPassIndex < 0)
                continue;
            const auto& accessPass = _orderedPasses[accessPassIndex];
            auto currentLayout = currentAccess.layout;

            if (pass->getType() == PassType::HOST) { // if pass is host no image transitions are made so use previous layout
                if (_resources[currentAccess.index]->type == ResourceType::IMAGE) {
                    if (auto [prevPassIndex, prevIndex, prevAccess] = findPrevAccess(passIndex - 1, currentAccess.index); prevPassIndex < 0) {
                        currentLayout = dynamic_cast<ImageResource*>(_resources[currentAccess.index].get())->initialLayout;
                    } else {
                        currentLayout = prevAccess.layout;
                    }
                }
            }

            RenderPass::Barrier barrier = {
                nextAccess.index,
                currentAccess.stage,
                nextAccess.stage,
                currentAccess.access,
                nextAccess.access,
                currentLayout,
                nextAccess.layout,
                pass->getQueue(),
                accessPass->getQueue()
            };
            accessPass->_barriers.push_back(barrier);
            if (pass->getQueue() != accessPass->getQueue()) {
                pass->_releaseBarriers.push_back(barrier);
                accessPass->_waits.push_back({passIndex, pass->getQueue()});
                pass->_signal = true;
            }
        }
        for (i32 inputIndex = 0; inputIndex < pass->_inputs.size(); inputIndex++) {
            const auto& currentAccess = pass->_inputs[inputIndex];
            if (writesResource(*pass, currentAccess.index))
                continue;

            const auto [accessPassIndex, accessIndex, nextAccess] = findNextAccess(passIndex, currentAccess.index);
            if (accessPassIndex < 0)
                continue;
            const auto& accessPass = _orderedPasses[accessPassIndex];
            auto currentLayout = currentAccess.layout;

            if (pass->getType() == PassType::HOST) { // if pass is host no image transitions are made so use previous layout
                if (_resources[currentAccess.index]->type == ResourceType::IMAGE) {
                    if (auto [prevPassIndex, prevIndex, prevAccess] = findPrevAccess(passIndex - 1, currentAccess.index); prevPassIndex < 0) {
                        currentLayout = dynamic_cast<ImageResource*>(_resources[currentAccess.index].get())->initialLayout;
                    } else {
                        currentLayout = prevAccess.layout;
                    }
                }
            }

            RenderPass::Barrier barrier = {
                nextAccess.index,
                currentAccess.stage,
                nextAccess.stage,
                currentAccess.access,
                nextAccess.access,
                currentLayout,
                nextAccess.layout,
                pass->getQueue(),
                accessPass->getQueue()
            };
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
        if (accessPassIndex < 0)
            continue;
        const auto& accessPass = _orderedPasses[accessPassIndex];
        const auto& resource = _resources[i];
        auto initialLayout = ImageLayout::UNDEFINED;
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
            accessPass->getType() == PassType::HOST ? initialLayout : nextAccess.layout
        });
    }

    if (_backbufferFinalLayout != ImageLayout::UNDEFINED) {
        const auto [accessPassIndex, accessIndex, prevAccess] = findPrevAccess(_orderedPasses.size() - 1, _backbufferIndex);
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
                    .name = bufferResource->name
                });
            }
        }
    }
}

void canta::RenderGraph::buildRenderAttachments() {
    auto findNextAccess = [&](const i32 startIndex, const u32 resource) -> std::tuple<i32, i32, ResourceAccess> {
        for (i32 passIndex = startIndex + 1; passIndex < _orderedPasses.size(); passIndex++) {
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

    auto findCurrAccess = [&](const RenderPass& pass, const u32 resource) -> std::tuple<bool, ResourceAccess> {
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

    auto findPrevAccess = [&](const i32 startIndex, const u32 resource) -> std::tuple<i32, i32, ResourceAccess> {
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