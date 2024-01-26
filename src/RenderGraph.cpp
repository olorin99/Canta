#include "Canta/RenderGraph.h"


void canta::RenderPass::addColourWrite(canta::ImageIndex index, const std::array<f32, 4> &clearColor) {
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
}

void canta::RenderPass::addColourRead(canta::ImageIndex index) {
    if (auto resource = reads(index, Access::COLOUR_READ,
                              PipelineStage::COLOUR_OUTPUT,
                              ImageLayout::COLOUR_ATTACHMENT)) {
        dynamic_cast<ImageResource*>(resource)->usage |= ImageUsage::COLOUR_ATTACHMENT;
    }
}

void canta::RenderPass::addDepthWrite(canta::ImageIndex index, const std::array<f32, 4>& clearColor) {
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
}

void canta::RenderPass::addDepthRead(canta::ImageIndex index) {
    if (auto resource = reads(index, Access::DEPTH_STENCIL_READ,
                              PipelineStage::EARLY_FRAGMENT_TEST | PipelineStage::LATE_FRAGMENT_TEST | PipelineStage::FRAGMENT_SHADER,
                              ImageLayout::DEPTH_STENCIL_ATTACHMENT)) {
        _depthAttachment = {
            .index = static_cast<i32>(index.index),
            .layout = ImageLayout::DEPTH_STENCIL_ATTACHMENT
        };
        dynamic_cast<ImageResource*>(resource)->usage |= ImageUsage::DEPTH_STENCIL_ATTACHMENT;
    }
}

void canta::RenderPass::addStorageImageWrite(canta::ImageIndex index, canta::PipelineStage stage) {
    if (auto resource = writes(index, Access::SHADER_WRITE | Access::SHADER_READ,
                               stage, ImageLayout::GENERAL)) {
        dynamic_cast<ImageResource*>(resource)->usage |= ImageUsage::STORAGE;
    }
}

void canta::RenderPass::addStorageImageRead(canta::ImageIndex index, canta::PipelineStage stage) {
    if (auto resource = reads(index, Access::SHADER_READ,
                              stage, ImageLayout::GENERAL)) {
        dynamic_cast<ImageResource*>(resource)->usage |= ImageUsage::STORAGE;
    }
}

void canta::RenderPass::addStorageBufferWrite(canta::BufferIndex index, canta::PipelineStage stage) {
    if (auto resource = writes(index, Access::SHADER_WRITE | Access::SHADER_READ,
                               stage)) {
        dynamic_cast<BufferResource*>(resource)->usage |= BufferUsage::STORAGE;
    }
}

void canta::RenderPass::addStorageBufferRead(canta::BufferIndex index, canta::PipelineStage stage) {
    if (auto resource = reads(index, Access::SHADER_READ,
                              stage)) {
        dynamic_cast<BufferResource*>(resource)->usage |= BufferUsage::STORAGE;
    }
}

void canta::RenderPass::addSampledRead(canta::ImageIndex index, canta::PipelineStage stage) {
    if (auto resource = reads(index, Access::SHADER_READ,
                              stage, ImageLayout::SHADER_READ_ONLY)) {
        dynamic_cast<ImageResource*>(resource)->usage |= ImageUsage::SAMPLED;
    }
}

void canta::RenderPass::addBlitWrite(canta::ImageIndex index) {
    if (auto resource = writes(index, Access::TRANSFER_WRITE | Access::TRANSFER_READ,
                               PipelineStage::TRANSFER,
                               ImageLayout::TRANSFER_DST)) {
        dynamic_cast<ImageResource*>(resource)->usage |= ImageUsage::TRANSFER_DST;
    }
}

void canta::RenderPass::addBlitRead(canta::ImageIndex index) {
    if (auto resource = reads(index, Access::TRANSFER_READ,
                              PipelineStage::TRANSFER,
                              ImageLayout::TRANSFER_SRC)) {
        dynamic_cast<ImageResource*>(resource)->usage |= ImageUsage::TRANSFER_SRC;
    }
}

void canta::RenderPass::addTransferWrite(canta::ImageIndex index) {
    if (auto resource = writes(index, Access::TRANSFER_WRITE | Access::TRANSFER_READ,
                               PipelineStage::TRANSFER,
                               ImageLayout::TRANSFER_DST)) {
        dynamic_cast<ImageResource*>(resource)->usage |= ImageUsage::TRANSFER_DST;
    }
}

void canta::RenderPass::addTransferRead(canta::ImageIndex index) {
    if (auto resource = reads(index, Access::TRANSFER_READ,
                              PipelineStage::TRANSFER,
                              ImageLayout::TRANSFER_SRC)) {
        dynamic_cast<ImageResource*>(resource)->usage |= ImageUsage::TRANSFER_SRC;
    }
}

void canta::RenderPass::addTransferWrite(canta::BufferIndex index) {
    if (auto resource = writes(index, Access::TRANSFER_WRITE | Access::TRANSFER_READ,
                               PipelineStage::TRANSFER)) {
        dynamic_cast<BufferResource*>(resource)->usage |= BufferUsage::TRANSFER_DST;
    }
}

void canta::RenderPass::addTransferRead(canta::BufferIndex index) {
    if (auto resource = reads(index, Access::TRANSFER_READ,
                              PipelineStage::TRANSFER)) {
        dynamic_cast<BufferResource*>(resource)->usage |= BufferUsage::TRANSFER_SRC;
    }
}

void canta::RenderPass::addIndirectRead(canta::BufferIndex index) {
    if (auto resource = reads(index, Access::INDIRECT,
                              PipelineStage::DRAW_INDIRECT)) {
        dynamic_cast<BufferResource*>(resource)->usage |= BufferUsage::INDIRECT;
    }
}


auto canta::RenderPass::writes(canta::ImageIndex index, canta::Access access, canta::PipelineStage stage, canta::ImageLayout layout) -> Resource * {
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

auto
canta::RenderPass::reads(canta::BufferIndex index, canta::Access access, canta::PipelineStage stage) -> Resource * {
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
    graph._individualTiming = info.individualTiming;
    graph._pipelineStatisticsEnabled = info.enablePipelineStatistics;
    graph._individualPipelineStatistics = info.individualPipelineStatistics;

    if (info.enableTiming && !info.individualTiming) {
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

auto canta::RenderGraph::addPass(std::string_view name, RenderPass::Type type) -> RenderPass & {
    u32 index = _passes.size();
    _passes.emplace_back();
    _passes.back()._graph = this;
    _passes.back()._name = name;
    _passes.back()._type = type;
    if (_timingEnabled && _individualTiming) {
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

auto canta::RenderGraph::addClearPass(std::string_view name, canta::ImageIndex index) -> RenderPass & {
    auto& clearPass = addPass(name);
    clearPass.addTransferWrite(index);
    clearPass.setExecuteFunction([index] (CommandBuffer& cmd, RenderGraph& graph) {
        auto image = graph.getImage(index);
        cmd.clearImage(image, ImageLayout::TRANSFER_DST);
    });
    return clearPass;
}

auto canta::RenderGraph::addBlitPass(std::string_view name, canta::ImageIndex src, canta::ImageIndex dst) -> RenderPass & {
    auto& blitPass = addPass(name);
    blitPass.addTransferRead(src);
    blitPass.addTransferWrite(dst);
    blitPass.setExecuteFunction([src, dst] (CommandBuffer& cmd, RenderGraph& graph) {
        auto srcImage = graph.getImage(src);
        auto dstImage = graph.getImage(dst);
        cmd.blit({
            .src = srcImage,
            .dst = dstImage,
            .srcLayout = ImageLayout::TRANSFER_SRC,
            .dstLayout = ImageLayout::TRANSFER_DST
        });
    });
    return blitPass;
}

auto canta::RenderGraph::addImage(canta::ImageDescription description) -> ImageIndex {
    auto it = _nameToIndex.find(description.name.data());
    if (it != _nameToIndex.end()) {
        u32 index = it->second;
        if (description.handle) {
            u32 imageIndex = dynamic_cast<ImageResource*>(_resources[index].get())->imageIndex;
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
        if (description.handle) {
            u32 bufferIndex = dynamic_cast<BufferResource*>(_resources[index].get())->bufferIndex;
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

void canta::RenderGraph::setBackbuffer(canta::ImageIndex index) {
    _backbufferId = index.id;
    _backbufferIndex = index.index;
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
        if (!dfs(pass))
            return std::unexpected(RenderGraphError::CYCLICAL_GRAPH);
    }

    buildBarriers();
    buildResources();
    buildRenderAttachments();

    return true;
}

auto canta::RenderGraph::execute(std::span<Semaphore::Pair> waits, std::span<Semaphore::Pair> signals, bool backbufferIsSwapchain) -> std::expected<bool, RenderGraphError> {
    _commandPools[_device->flyingIndex()].reset();
    auto& cmd = _commandPools[_device->flyingIndex()].getBuffer();
    cmd.begin();
    if (_timingEnabled && !_individualTiming) {
        _timers[_device->flyingIndex()].front().first = _name;
        _timers[_device->flyingIndex()].front().second.begin(cmd);
    }
    if (_pipelineStatisticsEnabled && !_individualPipelineStatistics) {
        _pipelineStats[_device->flyingIndex()].front().first = _name;
        _pipelineStats[_device->flyingIndex()].front().second.begin(cmd);
    }

    for (u32 i = 0; i < _orderedPasses.size(); i++) {
        auto& pass = _orderedPasses[i];
        cmd.pushDebugLabel(pass->_name);

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

        if (_timingEnabled && _individualTiming)
            _timers[_device->flyingIndex()][i].second.begin(cmd);
        if (_pipelineStatisticsEnabled && _individualPipelineStatistics)
            _pipelineStats[_device->flyingIndex()][i].second.begin(cmd);

        if (pass->_type == RenderPass::Type::GRAPHICS) {
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
        if (pass->_type == RenderPass::Type::GRAPHICS) {
            cmd.endRendering();
        }
        if (_pipelineStatisticsEnabled && _individualPipelineStatistics)
            _pipelineStats[_device->flyingIndex()][i].second.end(cmd);
        if (_timingEnabled && _individualTiming)
            _timers[_device->flyingIndex()][i].second.end(cmd);
        cmd.popDebugLabel();
    }

    if (backbufferIsSwapchain) {
        cmd.barrier({
            .image = getImage({ .index = static_cast<u32>(_backbufferIndex) }),
            .srcStage = canta::PipelineStage::COLOUR_OUTPUT,
            .dstStage = canta::PipelineStage::BOTTOM,
            .srcAccess = canta::Access::COLOUR_WRITE | canta::Access::COLOUR_READ,
            .dstAccess = canta::Access::MEMORY_WRITE | canta::Access::MEMORY_READ,
            .srcLayout = canta::ImageLayout::COLOUR_ATTACHMENT,
            .dstLayout = canta::ImageLayout::PRESENT
        });
    }

    if (_pipelineStatisticsEnabled && !_individualPipelineStatistics)
        _pipelineStats[_device->flyingIndex()].front().second.end(cmd);
    if (_timingEnabled && !_individualTiming)
        _timers[_device->flyingIndex()].front().second.end(cmd);
    cmd.end();
    cmd.submit(waits, signals);

    return true;
}

void canta::RenderGraph::buildBarriers() {
    auto findNextAccess = [&](i32 startIndex, u32 resource) -> std::tuple<i32, i32, RenderPass::ResourceAccess> {
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
        accessPass->_barriers.push_back({
            nextAccess.index,
            PipelineStage::TOP,
            nextAccess.stage,
            Access::MEMORY_READ | Access::MEMORY_WRITE,
            nextAccess.access,
            ImageLayout::UNDEFINED,
            nextAccess.layout
        });
    }
}

void canta::RenderGraph::buildResources() {
    for (auto& resource : _resources) {
        if (resource->type == ResourceType::IMAGE) {
            auto imageResource = dynamic_cast<ImageResource*>(resource.get());
            if (imageResource->matchesBackbuffer) {
                auto backbufferImage = getImage({ .index = static_cast<u32>(_backbufferIndex) });
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
    auto findNextAccess = [&](i32 startIndex, u32 resource) -> std::tuple<i32, i32, RenderPass::ResourceAccess> {
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

    auto findCurrAccess = [&](RenderPass& pass, u32 resource) -> std::tuple<bool, RenderPass::ResourceAccess> {
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

    auto findPrevAccess = [&](i32 startIndex, u32 resource) -> std::tuple<i32, i32, RenderPass::ResourceAccess> {
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