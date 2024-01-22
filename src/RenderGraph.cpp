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

void canta::RenderPass::addDepthWrite(canta::ImageIndex index) {
    if (auto resource = writes(index, Access::DEPTH_STENCIL_WRITE | Access::DEPTH_STENCIL_READ,
                               PipelineStage::EARLY_FRAGMENT_TEST | PipelineStage::LATE_FRAGMENT_TEST | PipelineStage::FRAGMENT_SHADER,
                               ImageLayout::DEPTH_STENCIL_ATTACHMENT)) {
        _depthAttachment = {
            .index = static_cast<i32>(index.index),
            .layout = ImageLayout::DEPTH_STENCIL_ATTACHMENT
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


auto canta::RenderPass::writes(canta::ImageIndex index, canta::Access access, canta::PipelineStage stage, canta::ImageLayout layout) -> Resource * {
    auto resource = _graph->_resources[index.index].get();
    if (resource) {
        _outputs.push_back({
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

    return graph;
}

auto canta::RenderGraph::addPass(std::string_view name, RenderPass::Type type) -> RenderPass & {
    _passes.push_back({});
    _passes.back()._graph = this;
    _passes.back()._name = name;
    _passes.back()._type = type;
    return _passes.back();
}

auto canta::RenderGraph::addImage(canta::ImageDescription description, canta::ImageHandle handle) -> ImageIndex {
    auto it = _nameToIndex.find(description.name.data());
    if (it != _nameToIndex.end()) {
        u32 index = it->second;
        if (handle) {
            u32 imageIndex = dynamic_cast<ImageResource*>(_resources[index].get())->imageIndex;
            _images[imageIndex] = handle;
        }
        return {
            .id = _resourceId++,
            .index = index
        };
    }

    u32 index = _resources.size();
    ImageResource resource = {};
    resource.imageIndex = _images.size();
    _images.push_back(handle);
    resource.width = description.width;
    resource.height = description.height;
    resource.depth = description.depth;
    resource.mipLevels = description.mipLevels;
    resource.format = description.format;
    resource.usage = description.usage;
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

auto canta::RenderGraph::addBuffer(canta::BufferDescription description, canta::BufferHandle handle) -> BufferIndex {
    auto it = _nameToIndex.find(description.name.data());
    if (it != _nameToIndex.end()) {
        u32 index = it->second;
        if (handle) {
            u32 bufferIndex = dynamic_cast<BufferResource*>(_resources[index].get())->bufferIndex;
            _buffers[bufferIndex] = handle;
        }
        return {
            .id = _resourceId++,
            .index = index
        };
    }

    u32 index = _resources.size();
    BufferResource resource = {};
    resource.bufferIndex = _buffers.size();
    _buffers.push_back(handle);
    resource.size = description.size;
    resource.usage = description.usage;
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
    _backbuffer = index.index;
}

void canta::RenderGraph::setBackbuffer(canta::BufferIndex index) {
    _backbuffer = index.index;
}

void canta::RenderGraph::reset() {
    _passes.clear();
    _resourceId = 0;
}

auto canta::RenderGraph::compile() -> std::expected<bool, RenderGraphError> {
    _orderedPasses.clear();

    std::vector<std::vector<u32>> outputs(_resources.size());
    for (u32 i = 0; i < _passes.size(); i++) {
        auto& pass = _passes[i];
        for (auto& output : pass._outputs)
            outputs[output.index].push_back(i);
    }

    std::vector<bool> visited(_passes.size(), false);
    std::vector<bool> onStack(_passes.size(), false);

    std::function<bool(u32)> dfs = [&](u32 index) -> bool {
        visited[index] = true;
        onStack[index] = true;
        auto& pass = _passes[index];
        for (auto& input : pass._inputs) {
            for (auto& output : outputs[input.index]) {
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

    for (auto& pass : outputs[_backbuffer]) {
        if (!dfs(pass))
            return std::unexpected(RenderGraphError::CYCLICAL_GRAPH);
    }

    buildBarriers();
    buildResources();

    return true;
}

auto canta::RenderGraph::execute(canta::CommandBuffer& cmd) -> std::expected<bool, RenderGraphError> {
    for (u32 i = 0; i < _orderedPasses.size(); i++) {
        auto& pass = _orderedPasses[i];

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

        if (pass->_type == RenderPass::Type::GRAPHICS) {
            RenderingInfo info = {};
            info.size = { static_cast<u32>(pass->_width), static_cast<u32>(pass->_height) };

            std::vector<canta::Attachment> attachments = {};
            for (auto& attachment : pass->_colourAttachments) {
                auto attachmentImage = getImage({ .index = static_cast<u32>(attachment.index) });
                attachments.push_back({
                    .imageView = attachmentImage->defaultView().view(),
                    .imageLayout = attachment.layout,
                    .clearColour = attachment.clearColor
                });
            }
            info.colourAttachments = attachments;
            if (pass->_depthAttachment.index >= 0) {
                auto depthAttachmentImage = getImage({ .index = static_cast<u32>(pass->_depthAttachment.index) });
                info.depthAttachment = {
                    .imageView = depthAttachmentImage->defaultView().view(),
                    .imageLayout = pass->_depthAttachment.layout
                };
            }
            cmd.beginRendering(info);
        }
        pass->_execute(cmd, *this);
        if (pass->_type == RenderPass::Type::GRAPHICS) {
            cmd.endRendering();
        }
    }

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