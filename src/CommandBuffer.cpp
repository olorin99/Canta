#include "Canta/CommandBuffer.h"
#include <Canta/Device.h>
#include <Canta/Pipeline.h>
#include <Canta/Buffer.h>
#include <Canta/Image.h>
#include <format>

canta::CommandBuffer::CommandBuffer(canta::CommandBuffer &&rhs) noexcept {
    std::swap(_device, rhs._device);
    std::swap(_buffer, rhs._buffer);
    std::swap(_queueType, rhs._queueType);
}

auto canta::CommandBuffer::operator=(canta::CommandBuffer &&rhs) noexcept -> CommandBuffer & {
    std::swap(_device, rhs._device);
    std::swap(_buffer, rhs._buffer);
    std::swap(_queueType, rhs._queueType);
    return *this;
}

auto canta::CommandBuffer::submit(std::span<Semaphore::Pair> waitSemaphores, std::span<Semaphore::Pair> signalSemaphores, VkFence fence) -> std::expected<bool, Error> {
    VkSemaphoreSubmitInfo waits[waitSemaphores.size()];
    VkSemaphoreSubmitInfo signals[signalSemaphores.size()];

    for (u32 i = 0; i < waitSemaphores.size(); i++) {
        auto& wait = waitSemaphores[i];
        waits[i].sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
        waits[i].semaphore = wait.semaphore->semaphore();
        waits[i].value = wait.value;
        waits[i].stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
        waits[i].deviceIndex = 0;
        waits[i].pNext = nullptr;
    }
    for (u32 i = 0; i < signalSemaphores.size(); i++) {
        auto& signal = signalSemaphores[i];
        signals[i].sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
        signals[i].semaphore = signal.semaphore->semaphore();
        signals[i].value = signal.value;
        signals[i].stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
        signals[i].deviceIndex = 0;
        signals[i].pNext = nullptr;
    }

    VkCommandBufferSubmitInfo commandBufferSubmitInfo = {};
    commandBufferSubmitInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
    commandBufferSubmitInfo.commandBuffer = _buffer;
    commandBufferSubmitInfo.deviceMask = 0;

    VkSubmitInfo2 submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
    submitInfo.waitSemaphoreInfoCount = waitSemaphores.size();
    submitInfo.pWaitSemaphoreInfos = waits;
    submitInfo.signalSemaphoreInfoCount = signalSemaphores.size();
    submitInfo.pSignalSemaphoreInfos = signals;
    submitInfo.commandBufferInfoCount = 1;
    submitInfo.pCommandBufferInfos = &commandBufferSubmitInfo;

    auto& queue = _device->queue(_queueType);

    auto result = vkQueueSubmit2(queue.queue(), 1, &submitInfo, fence);
    if (result != VK_SUCCESS)
        return std::unexpected(static_cast<Error>(result));
    return true;
}

auto canta::CommandBuffer::begin() -> bool {
    _stats = {};
    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    return vkBeginCommandBuffer(_buffer, &beginInfo) == VK_SUCCESS;
}

auto canta::CommandBuffer::end() -> bool {
    return vkEndCommandBuffer(_buffer);
}

void canta::CommandBuffer::beginRendering(RenderingInfo info) {
    VkRenderingInfo renderingInfo = {};
    renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
    renderingInfo.renderArea = {
            { info.offset.x(), info.offset.y() },
            { info.size.x(), info.size.y() }
    };
    renderingInfo.layerCount = 1;
    renderingInfo.viewMask = {};

    std::vector<VkRenderingAttachmentInfo> colourAttachments(info.colourAttachments.size());
    for (u32 i = 0; i < info.colourAttachments.size(); i++) {
        VkClearValue clearValue = loadVkClearValue(info.colourAttachments[i].image->format(), info.colourAttachments[i].clearColour);
        colourAttachments[i] = {
                .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
                .imageView = info.colourAttachments[i].image->defaultView()->view(),
                .imageLayout = static_cast<VkImageLayout>(info.colourAttachments[i].imageLayout),
                .resolveMode = VK_RESOLVE_MODE_NONE,
                .resolveImageView = VK_NULL_HANDLE,
                .resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                .loadOp = static_cast<VkAttachmentLoadOp>(info.colourAttachments[i].loadOp),
                .storeOp = static_cast<VkAttachmentStoreOp>(info.colourAttachments[i].storeOp),
                .clearValue = clearValue
        };
    }
    renderingInfo.colorAttachmentCount = colourAttachments.size();
    renderingInfo.pColorAttachments = colourAttachments.data();

    VkRenderingAttachmentInfo depthAttachment = {
            .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
            .imageLayout = static_cast<VkImageLayout>(info.depthAttachment.imageLayout),
            .resolveMode = VK_RESOLVE_MODE_NONE,
            .resolveImageView = VK_NULL_HANDLE,
            .resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .loadOp = static_cast<VkAttachmentLoadOp>(info.depthAttachment.loadOp),
            .storeOp = static_cast<VkAttachmentStoreOp>(info.depthAttachment.storeOp),
            .clearValue = {}
    };
    if (info.depthAttachment.image) {
        depthAttachment.clearValue = loadVkClearValue(info.depthAttachment.image->format(), info.depthAttachment.clearColour);
        depthAttachment.imageView = info.depthAttachment.image->defaultView()->view();
        renderingInfo.pDepthAttachment = &depthAttachment;
    }

    vkCmdBeginRendering(_buffer, &renderingInfo);
}

void canta::CommandBuffer::endRendering() {
    vkCmdEndRendering(_buffer);
}

void canta::CommandBuffer::setViewport(const ende::math::Vec<2, f32> &size, const ende::math::Vec<2, f32> &offset, bool scissor) {
    VkViewport viewport = {
            .x = offset.x(),
            .y = offset.y(),
            .width = size.x(),
            .height = size.y(),
            .minDepth = 0.f,
            .maxDepth = 1.f
    };
    vkCmdSetViewport(_buffer, 0, 1, &viewport);
    if (scissor)
        setScissor({ static_cast<u32>(size.x()), static_cast<u32>(size.y()) }, { static_cast<i32>(offset.x()), static_cast<i32>(offset.y()) });
}

void canta::CommandBuffer::setScissor(const ende::math::Vec<2, u32> &size, const ende::math::Vec<2, i32> &offset) {
    VkRect2D scissor = {
            .offset = { offset.x(), offset.y() },
            .extent = { size.x(), size.y() }
    };
    vkCmdSetScissor(_buffer, 0, 1, &scissor);
}

auto canta::CommandBuffer::bindPipeline(PipelineHandle pipeline) -> bool {
    if (!pipeline)
        return false;
    vkCmdBindPipeline(_buffer, pipeline->mode() == PipelineMode::GRAPHICS ? VK_PIPELINE_BIND_POINT_GRAPHICS : VK_PIPELINE_BIND_POINT_COMPUTE, pipeline->pipeline());
    _currentPipeline = pipeline;
    auto set = _device->bindlessSet();
    vkCmdBindDescriptorSets(_buffer, pipeline->mode() == PipelineMode::GRAPHICS ? VK_PIPELINE_BIND_POINT_GRAPHICS : VK_PIPELINE_BIND_POINT_COMPUTE, _currentPipeline->layout(), 0, 1, &set, 0, nullptr);
    return true;
}

void canta::CommandBuffer::bindVertexBuffer(canta::BufferHandle handle) {
    assert(handle);
    assert((handle->usage() & BufferUsage::VERTEX) == BufferUsage::VERTEX);
    VkDeviceSize offset = 0;
    auto buffer = handle->buffer();
    vkCmdBindVertexBuffers(_buffer, 0, 1, &buffer, &offset);
}

void canta::CommandBuffer::bindVertexBuffers(std::span<BufferHandle> handles, u32 first, u32 offset) {
    VkDeviceSize off = offset;
    VkBuffer buffers[handles.size()];
    for (u32 i = 0; i < handles.size(); i++) {
        assert(handles[i]);
        assert((handles[i]->usage() & BufferUsage::VERTEX) == BufferUsage::VERTEX);
        buffers[i] = handles[i]->buffer();
    }
    vkCmdBindVertexBuffers(_buffer, first, handles.size(), buffers, &off);
}

void canta::CommandBuffer::bindIndexBuffer(canta::BufferHandle handle, u32 offset, u32 indexType) {
    assert(handle);
    assert((handle->usage() & BufferUsage::INDEX) == BufferUsage::INDEX);
    vkCmdBindIndexBuffer(_buffer, handle->buffer(), offset, static_cast<VkIndexType>(indexType));
}

void canta::CommandBuffer::pushConstants(canta::ShaderStage stage, std::span<const u8> data, u32 offset) {
    assert(offset + data.size() <= 128);
    vkCmdPushConstants(_buffer, _currentPipeline->layout(), static_cast<VkShaderStageFlagBits>(stage), offset, data.size(), data.data());
}

void canta::CommandBuffer::draw(u32 count, u32 instanceCount, u32 firstVertex, u32 firstIndex, u32 firstInstance, bool indexed) {
    assert(_currentPipeline);
    assert(_currentPipeline->mode() == PipelineMode::GRAPHICS);
    if (indexed) {
        vkCmdDrawIndexed(_buffer, count, instanceCount, firstVertex, firstIndex, firstInstance);
        writeMarker(PipelineStage::VERTEX_SHADER, util::storeMarker(util::DrawIndexed{ .stage = PipelineStage::VERTEX_SHADER, .count = count, .instanceCount = instanceCount, .firstVertex = firstVertex, .firstIndex = firstIndex, .firstInstance = firstInstance}));
        writeMarker(PipelineStage::FRAGMENT_SHADER, util::storeMarker(util::DrawIndexed{ .stage = PipelineStage::FRAGMENT_SHADER, .count = count, .instanceCount = instanceCount, .firstVertex = firstVertex, .firstIndex = firstIndex, .firstInstance = firstInstance}));
//        writeMarker(PipelineStage::VERTEX_SHADER, std::format("drawIndexed({}, {}, {}, {}) - VERTEX", count, instanceCount, firstVertex, firstInstance));
//        writeMarker(PipelineStage::FRAGMENT_SHADER, std::format("drawIndexed({}, {}, {}, {}) - FRAGMENT", count, instanceCount, firstVertex, firstInstance));
    } else {
        vkCmdDraw(_buffer, count, instanceCount, firstVertex, firstInstance);
        writeMarker(PipelineStage::VERTEX_SHADER, util::storeMarker(util::Draw{ .stage = PipelineStage::VERTEX_SHADER, .count = count, .instanceCount = instanceCount, .firstVertex = firstVertex, .firstInstance = firstInstance}));
        writeMarker(PipelineStage::FRAGMENT_SHADER, util::storeMarker(util::Draw{ .stage = PipelineStage::FRAGMENT_SHADER, .count = count, .instanceCount = instanceCount, .firstVertex = firstVertex, .firstInstance = firstInstance}));
//        writeMarker(PipelineStage::VERTEX_SHADER, std::format("draw({}, {}, {}, {}) - VERTEX", count, instanceCount, firstVertex, firstInstance));
//        writeMarker(PipelineStage::FRAGMENT_SHADER, std::format("draw({}, {}, {}, {}) - FRAGMENT", count, instanceCount, firstVertex, firstInstance));
    }
    _stats.drawCalls++;
}

void canta::CommandBuffer::drawIndirect(canta::BufferHandle commands, u32 offset, u32 drawCount, bool indexed, u32 stride) {
    assert(_currentPipeline);
    assert(_currentPipeline->mode() == PipelineMode::GRAPHICS);
    if (indexed) {
        if (stride == 0)
            stride = sizeof(VkDrawIndexedIndirectCommand);
        vkCmdDrawIndexedIndirect(_buffer, commands->buffer(), offset, drawCount, stride);
        writeMarker(PipelineStage::VERTEX_SHADER, util::storeMarker(util::DrawIndexedIndirect{ .stage = PipelineStage::VERTEX_SHADER, .bufferIndex = commands.index(), .offset = offset, .drawCount = drawCount, .stride = stride}));
        writeMarker(PipelineStage::FRAGMENT_SHADER, util::storeMarker(util::DrawIndexedIndirect{ .stage = PipelineStage::FRAGMENT_SHADER, .bufferIndex = commands.index(), .offset = offset, .drawCount = drawCount, .stride = stride}));
//        writeMarker(PipelineStage::VERTEX_SHADER, std::format("drawIndexedIndirect({}, {}, {}, {}) - VERTEX", commands.index(), offset, drawCount, stride));
//        writeMarker(PipelineStage::FRAGMENT_SHADER, std::format("drawIndexedIndirect({}, {}, {}, {}) - FRAGMENT", commands.index(), offset, drawCount, stride));
    } else {
        if (stride == 0)
            stride = sizeof(VkDrawIndirectCommand);
        vkCmdDrawIndirect(_buffer, commands->buffer(), offset, drawCount, stride);
        writeMarker(PipelineStage::VERTEX_SHADER, util::storeMarker(util::DrawIndirect{ .stage = PipelineStage::VERTEX_SHADER, .bufferIndex = commands.index(), .offset = offset, .drawCount = drawCount, .stride = stride}));
        writeMarker(PipelineStage::FRAGMENT_SHADER, util::storeMarker(util::DrawIndirect{ .stage = PipelineStage::FRAGMENT_SHADER, .bufferIndex = commands.index(), .offset = offset, .drawCount = drawCount, .stride = stride}));
//        writeMarker(PipelineStage::VERTEX_SHADER, std::format("drawIndirect({}, {}, {}, {}) - VERTEX", commands.index(), offset, drawCount, stride));
//        writeMarker(PipelineStage::FRAGMENT_SHADER, std::format("drawIndirect({}, {}, {}, {}) - FRAGMENT", commands.index(), offset, drawCount, stride));
    }
    _stats.drawCalls++;
}

void canta::CommandBuffer::drawIndirectCount(canta::BufferHandle commands, u32 offset, canta::BufferHandle countBuffer, u32 countOffset, bool indexed, u32 stride) {
    assert(_currentPipeline);
    assert(_currentPipeline->mode() == PipelineMode::GRAPHICS);
    if (indexed) {
        if (stride == 0)
            stride = sizeof(VkDrawIndexedIndirectCommand);
        u32 maxDrawCount = (commands->size() - offset) / stride;
        vkCmdDrawIndexedIndirectCount(_buffer, commands->buffer(), offset, countBuffer->buffer(), countOffset, maxDrawCount, stride);
        writeMarker(PipelineStage::VERTEX_SHADER, util::storeMarker(util::DrawIndexedIndirectCount{ .stage = PipelineStage::VERTEX_SHADER, .bufferIndex = commands.index(), .offset = offset, .countBufferIndex = countBuffer.index(), .countOffset = countOffset, .maxDrawCount = maxDrawCount, .stride = stride}));
        writeMarker(PipelineStage::FRAGMENT_SHADER, util::storeMarker(util::DrawIndexedIndirectCount{ .stage = PipelineStage::FRAGMENT_SHADER, .bufferIndex = commands.index(), .offset = offset, .countBufferIndex = countBuffer.index(), .countOffset = countOffset, .maxDrawCount = maxDrawCount, .stride = stride}));
//        writeMarker(PipelineStage::VERTEX_SHADER, std::format("drawIndexedIndirectCount({}, {}, {}, {}, {}, {}) - VERTEX", commands.index(), offset, countBuffer.index(), countOffset, maxDrawCount, stride));
//        writeMarker(PipelineStage::FRAGMENT_SHADER, std::format("drawIndexedIndirectCount({}, {}, {}, {}, {}, {}) - FRAGMENT", commands.index(), offset, countBuffer.index(), countOffset, maxDrawCount, stride));
    } else {
        if (stride == 0)
            stride = sizeof(VkDrawIndirectCommand);
        u32 maxDrawCount = (commands->size() - offset) / stride;
        vkCmdDrawIndirectCount(_buffer, commands->buffer(), offset, countBuffer->buffer(), countOffset, maxDrawCount, stride);
        writeMarker(PipelineStage::VERTEX_SHADER, util::storeMarker(util::DrawIndirectCount{ .stage = PipelineStage::VERTEX_SHADER, .bufferIndex = commands.index(), .offset = offset, .countBufferIndex = countBuffer.index(), .countOffset = countOffset, .maxDrawCount = maxDrawCount, .stride = stride}));
        writeMarker(PipelineStage::FRAGMENT_SHADER, util::storeMarker(util::DrawIndirectCount{ .stage = PipelineStage::FRAGMENT_SHADER, .bufferIndex = commands.index(), .offset = offset, .countBufferIndex = countBuffer.index(), .countOffset = countOffset, .maxDrawCount = maxDrawCount, .stride = stride}));
//        writeMarker(PipelineStage::VERTEX_SHADER, std::format("drawIndirectCount({}, {}, {}, {}, {}, {}) - VERTEX", commands.index(), offset, countBuffer.index(), countOffset, maxDrawCount, stride));
//        writeMarker(PipelineStage::FRAGMENT_SHADER, std::format("drawIndirectCount({}, {}, {}, {}, {}, {}) - FRAGMENT", commands.index(), offset, countBuffer.index(), countOffset, maxDrawCount, stride));
    }
    _stats.drawCalls++;
}

void canta::CommandBuffer::drawMeshTasksWorkgroups(u32 x, u32 y, u32 z) {
    assert(_currentPipeline);
    assert(_currentPipeline->mode() == PipelineMode::GRAPHICS);
    assert(_currentPipeline->interface().stagePresent(ShaderStage::MESH));
    vkCmdDrawMeshTasksEXT(_buffer, x, y, z);
    writeMarker(PipelineStage::MESH_SHADER, util::storeMarker(util::MeshTasks{ .stage = PipelineStage::MESH_SHADER, .x = x, .y = y, .z = z }));
    writeMarker(PipelineStage::TASK_SHADER, util::storeMarker(util::MeshTasks{ .stage = PipelineStage::TASK_SHADER, .x = x, .y = y, .z = z }));
    writeMarker(PipelineStage::FRAGMENT_SHADER, util::storeMarker(util::MeshTasks{ .stage = PipelineStage::FRAGMENT_SHADER, .x = x, .y = y, .z = z }));
    _stats.drawCalls++;
}

void canta::CommandBuffer::drawMeshTasksThreads(u32 x, u32 y, u32 z) {
    ende::math::Vec<3, u32> localSize = { 1, 1, 1 };
    if (_currentPipeline->interface().stagePresent(ShaderStage::TASK))
        localSize = _currentPipeline->interface().localSize(ShaderStage::TASK);
    else
        localSize = _currentPipeline->interface().localSize(ShaderStage::MESH);
    drawMeshTasksWorkgroups(std::ceil(static_cast<f32>(x) / static_cast<f32>(localSize.x())), std::ceil(static_cast<f32>(y) / static_cast<f32>(localSize.y())), std::ceil(static_cast<f32>(z) / static_cast<f32>(localSize.z())));
}

void canta::CommandBuffer::drawMeshTasksIndirect(canta::BufferHandle commands, u32 offset, u32 drawCount, u32 stride) {
    assert(_currentPipeline);
    assert(_currentPipeline->mode() == PipelineMode::GRAPHICS);
    assert(_currentPipeline->interface().stagePresent(ShaderStage::MESH));
    vkCmdDrawMeshTasksIndirectEXT(_buffer, commands->buffer(), offset, drawCount, stride);
    writeMarker(PipelineStage::MESH_SHADER, util::storeMarker(util::MeshTasksIndirect{ .stage = PipelineStage::MESH_SHADER, .bufferIndex = commands.index(), .offset = offset, .drawCount = drawCount }));
    writeMarker(PipelineStage::TASK_SHADER, util::storeMarker(util::MeshTasksIndirect{ .stage = PipelineStage::TASK_SHADER, .bufferIndex = commands.index(), .offset = offset, .drawCount = drawCount }));
    writeMarker(PipelineStage::FRAGMENT_SHADER, util::storeMarker(util::MeshTasksIndirect{ .stage = PipelineStage::FRAGMENT_SHADER, .bufferIndex = commands.index(), .offset = offset, .drawCount = drawCount }));
    _stats.drawCalls++;
}

void canta::CommandBuffer::drawMeshTasksIndirectCount(canta::BufferHandle commands, u32 offset, canta::BufferHandle countBuffer, u32 countOffset, u32 stride) {
    assert(_currentPipeline);
    assert(_currentPipeline->mode() == PipelineMode::GRAPHICS);
    assert(_currentPipeline->interface().stagePresent(ShaderStage::MESH));
    u32 maxDrawCount = (commands->size() - offset) / stride;
    vkCmdDrawMeshTasksIndirectCountEXT(_buffer, commands->buffer(), offset, countBuffer->buffer(), countOffset, maxDrawCount, stride);
    writeMarker(PipelineStage::MESH_SHADER, util::storeMarker(util::MeshTasksIndirectCount{ .stage = PipelineStage::MESH_SHADER, .bufferIndex = commands.index(), .offset = offset, .countBufferIndex = countBuffer.index(), .countOffset = countOffset, .maxDrawCount = maxDrawCount, .stride = stride }));
    writeMarker(PipelineStage::TASK_SHADER, util::storeMarker(util::MeshTasksIndirectCount{ .stage = PipelineStage::TASK_SHADER, .bufferIndex = commands.index(), .offset = offset, .countBufferIndex = countBuffer.index(), .countOffset = countOffset, .maxDrawCount = maxDrawCount, .stride = stride }));
    writeMarker(PipelineStage::FRAGMENT_SHADER, util::storeMarker(util::MeshTasksIndirectCount{ .stage = PipelineStage::FRAGMENT_SHADER, .bufferIndex = commands.index(), .offset = offset, .countBufferIndex = countBuffer.index(), .countOffset = countOffset, .maxDrawCount = maxDrawCount, .stride = stride }));
    _stats.drawCalls++;
}

void canta::CommandBuffer::dispatchWorkgroups(u32 x, u32 y, u32 z) {
    assert(_currentPipeline);
    assert(_currentPipeline->mode() == PipelineMode::COMPUTE);
    assert(_currentPipeline->interface().stagePresent(ShaderStage::COMPUTE));
    vkCmdDispatch(_buffer, x, y, z);
    writeMarker(PipelineStage::COMPUTE_SHADER, util::storeMarker(util::Dispatch{ .stage = PipelineStage::COMPUTE_SHADER, .x = x, .y = y, .z = z }));
    _stats.dispatchCalls++;
}

void canta::CommandBuffer::dispatchThreads(u32 x, u32 y, u32 z) {
    auto localSize = _currentPipeline->interface().localSize(ShaderStage::COMPUTE);
    dispatchWorkgroups(std::ceil(static_cast<f32>(x) / static_cast<f32>(localSize.x())), std::ceil(static_cast<f32>(y) / static_cast<f32>(localSize.y())), std::ceil(static_cast<f32>(z) / static_cast<f32>(localSize.z())));
}

void canta::CommandBuffer::dispatchIndirect(canta::BufferHandle commands, u32 offset) {
    assert(_currentPipeline);
    assert(_currentPipeline->mode() == PipelineMode::COMPUTE);
    assert(_currentPipeline->interface().stagePresent(ShaderStage::COMPUTE));
    vkCmdDispatchIndirect(_buffer, commands->buffer(), offset);
    writeMarker(PipelineStage::COMPUTE_SHADER, util::storeMarker(util::DispatchIndirect{ .stage = PipelineStage::COMPUTE_SHADER, .bufferIndex = commands.index(), .offset = offset}));
    _stats.dispatchCalls++;
}

void canta::CommandBuffer::blit(canta::CommandBuffer::BlitInfo info) {
    VkImageSubresourceLayers srcSubresource = {};
    srcSubresource.aspectMask = aspectMask(info.src->format());
    srcSubresource.mipLevel = info.srcMip;
    srcSubresource.baseArrayLayer = info.srcLayer;
    srcSubresource.layerCount = info.srcLayerCount == 0 ? info.src->layers() : info.srcLayerCount;

    VkImageSubresourceLayers dstSubresource = {};
    dstSubresource.aspectMask = aspectMask(info.dst->format());
    dstSubresource.mipLevel = info.dstMip;
    dstSubresource.baseArrayLayer = info.dstLayer;
    dstSubresource.layerCount = info.dstLayerCount == 0 ? info.dst->layers() : info.dstLayerCount;

    VkImageBlit blit = {};
    blit.srcSubresource = srcSubresource;
    blit.srcOffsets[0] = { info.srcOffset.x(), info.srcOffset.y(), info.srcOffset.z() };
    if (info.srcSize.x() > 0 || info.srcSize.y() > 0 || info.srcSize.z() > 0)
        blit.srcOffsets[1] = { info.srcSize.x(), info.srcSize.y(), info.srcSize.z() };
    else
    blit.srcOffsets[1] = { static_cast<i32>(info.src->width()), static_cast<i32>(info.src->height()), static_cast<i32>(info.src->depth()) };

    blit.dstSubresource = dstSubresource;
    blit.dstOffsets[0] = { info.dstOffset.x(), info.dstOffset.y(), info.dstOffset.z() };
    if (info.dstSize.x() > 0 || info.dstSize.y() > 0 || info.dstSize.z() > 0)
        blit.dstOffsets[1] = { info.dstSize.x(), info.dstSize.y(), info.dstSize.z() };
    else
        blit.dstOffsets[1] = { static_cast<i32>(info.dst->width()), static_cast<i32>(info.dst->height()), static_cast<i32>(info.dst->depth()) };

    vkCmdBlitImage(_buffer, info.src->image(), static_cast<VkImageLayout>(info.srcLayout), info.dst->image(), static_cast<VkImageLayout>(info.dstLayout), 1, &blit, static_cast<VkFilter>(info.filter));
}

void canta::CommandBuffer::clearImage(ImageHandle handle, ImageLayout layout, const ClearValue &clearColour) {
    VkClearColorValue clearValue = loadVkClearValue(handle->format(), clearColour).color;
    VkImageSubresourceRange range = {};
    range.aspectMask = aspectMask(handle->format());
    range.baseMipLevel = 0;
    range.levelCount = handle->mips();
    range.baseArrayLayer = 0;
    range.layerCount = handle->layers();

    vkCmdClearColorImage(_buffer, handle->image(), static_cast<VkImageLayout>(layout), &clearValue, 1, &range);
}

void canta::CommandBuffer::clearBuffer(canta::BufferHandle handle, u32 clearValue, u32 offset, u32 size) {
    vkCmdFillBuffer(_buffer, handle->buffer(), offset, size == 0 ? handle->size() - offset : size, clearValue);
}

void canta::CommandBuffer::copyBufferToImage(BufferImageCopyInfo info) {
    VkBufferImageCopy imageCopy{};
    imageCopy.bufferOffset = info.srcOffset;
    imageCopy.bufferRowLength = 0;
    imageCopy.bufferImageHeight = 0;

    imageCopy.imageSubresource.aspectMask = aspectMask(info.image->format());
    imageCopy.imageSubresource.mipLevel = info.dstMipLevel;
    imageCopy.imageSubresource.baseArrayLayer = info.dstLayer;
    imageCopy.imageSubresource.layerCount = info.dstLayerCount;
    imageCopy.imageExtent.width = info.dstDimensions.x() == 0 ? info.image->width() : info.dstDimensions.x();
    imageCopy.imageExtent.height = info.dstDimensions.y() == 0 ? info.image->height() : info.dstDimensions.y();
    imageCopy.imageExtent.depth = info.dstDimensions.z() == 0 ? info.image->depth() : info.dstDimensions.z();
    imageCopy.imageOffset.x = info.dstOffsets.x();
    imageCopy.imageOffset.y = info.dstOffsets.y();
    imageCopy.imageOffset.z = info.dstOffsets.z();

    vkCmdCopyBufferToImage(_buffer, info.buffer->buffer(), info.image->image(), static_cast<VkImageLayout>(info.dstLayout), 1, &imageCopy);
}

void canta::CommandBuffer::copyImageToBuffer(canta::CommandBuffer::BufferImageCopyInfo info) {
    VkBufferImageCopy imageCopy = {};
    imageCopy.bufferOffset = info.srcOffset;
    imageCopy.bufferRowLength = 0;
    imageCopy.bufferImageHeight = 0;

    imageCopy.imageSubresource.aspectMask = aspectMask(info.image->format());
    imageCopy.imageSubresource.mipLevel = info.dstMipLevel;
    imageCopy.imageSubresource.baseArrayLayer = info.dstLayer;
    imageCopy.imageSubresource.layerCount = info.dstLayerCount;
    imageCopy.imageExtent.width = info.dstDimensions.x() == 0 ? info.image->width() : info.dstDimensions.x();
    imageCopy.imageExtent.height = info.dstDimensions.y() == 0 ? info.image->height() : info.dstDimensions.y();
    imageCopy.imageExtent.depth = info.dstDimensions.z() == 0 ? info.image->depth() : info.dstDimensions.z();
    imageCopy.imageOffset.x = info.dstOffsets.x();
    imageCopy.imageOffset.y = info.dstOffsets.y();
    imageCopy.imageOffset.z = info.dstOffsets.z();

    vkCmdCopyImageToBuffer(_buffer, info.image->image(), static_cast<VkImageLayout>(info.dstLayout), info.buffer->buffer(), 1, &imageCopy);
}

void canta::CommandBuffer::copyBuffer(canta::CommandBuffer::BufferCopyInfo info) {
    VkBufferCopy copy = {};
    copy.srcOffset = info.srcOffset;
    copy.dstOffset = info.dstOffset;
    copy.size = info.size;
    vkCmdCopyBuffer(_buffer, info.src->buffer(), info.dst->buffer(), 1, &copy);
}

void canta::CommandBuffer::generateMips(canta::ImageHandle image, ImageLayout initalLayout, ImageLayout finalLayout) {
    barrier({
        .image = image,
        .srcStage = PipelineStage::TOP,
        .dstStage = PipelineStage::TRANSFER,
        .srcAccess = Access::NONE,
        .dstAccess = Access::TRANSFER_READ,
        .srcLayout = initalLayout,
        .dstLayout = ImageLayout::TRANSFER_SRC,
    });

    for (u32 layer = 0; layer < image->layers(); layer++) {
        i32 mipWidth = image->width();
        i32 mipHeight = image->height();
        i32 mipDepth = image->depth();

        for (u32 mip = 1; mip < image->mips(); mip++) {
            barrier({
                .image = image,
                .srcStage = PipelineStage::TRANSFER,
                .dstStage = PipelineStage::TRANSFER,
                .srcAccess = Access::TRANSFER_READ,
                .dstAccess = Access::TRANSFER_WRITE,
                .srcLayout = ImageLayout::TRANSFER_SRC,
                .dstLayout = ImageLayout::TRANSFER_DST,
                .layer = layer,
                .layerCount = 1,
                .mip = mip,
                .mipCount = 1
            });

            BlitInfo blitInfo = {
                .src = image,
                .srcMip = mip - 1,
                .srcLayer = layer,
                .srcLayerCount = 1,
                .srcSize = { mipWidth, mipHeight, mipDepth },
                .dst = image,
                .dstMip = mip,
                .dstLayer = layer,
                .dstLayerCount = 1
            };

            mipWidth = std::max(1, mipWidth / 2);
            mipHeight = std::max(1, mipHeight / 2);
            mipDepth = std::max(1, mipDepth / 2);

            blitInfo.dstSize = { mipWidth, mipHeight, mipDepth };

            blit(blitInfo);

            barrier({
                .image = image,
                .srcStage = PipelineStage::TRANSFER,
                .dstStage = PipelineStage::TRANSFER,
                .srcAccess = Access::TRANSFER_WRITE,
                .dstAccess = Access::TRANSFER_READ,
                .srcLayout = ImageLayout::TRANSFER_DST,
                .dstLayout = ImageLayout::TRANSFER_SRC,
                .layer = layer,
                .layerCount = 1,
                .mip = mip,
                .mipCount = 1
            });
        }
    }

    barrier({
        .image = image,
        .srcStage = PipelineStage::TRANSFER,
        .dstStage = PipelineStage::BOTTOM,
        .srcAccess = Access::TRANSFER_READ | Access::TRANSFER_WRITE,
        .dstAccess = Access::NONE,
        .srcLayout = ImageLayout::TRANSFER_DST,
        .dstLayout = finalLayout,
    });
}

void canta::CommandBuffer::barrier(ImageBarrier barrier) {
    VkImageMemoryBarrier2 imageBarrier = {};
    imageBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
    imageBarrier.image = barrier.image->image();
    imageBarrier.srcStageMask = static_cast<VkPipelineStageFlagBits>(barrier.srcStage);
    imageBarrier.dstStageMask = static_cast<VkPipelineStageFlagBits>(barrier.dstStage);
    imageBarrier.srcAccessMask = static_cast<VkAccessFlagBits>(barrier.srcAccess);
    imageBarrier.dstAccessMask = static_cast<VkAccessFlagBits>(barrier.dstAccess);
    imageBarrier.oldLayout = static_cast<VkImageLayout>(barrier.srcLayout);
    imageBarrier.newLayout = static_cast<VkImageLayout>(barrier.dstLayout);
    imageBarrier.srcQueueFamilyIndex = barrier.srcQueue;
    imageBarrier.dstQueueFamilyIndex = barrier.dstQueue;

    imageBarrier.subresourceRange.layerCount = barrier.layerCount == 0 ? VK_REMAINING_ARRAY_LAYERS : barrier.layerCount;
    imageBarrier.subresourceRange.baseArrayLayer = barrier.layer;
    imageBarrier.subresourceRange.levelCount = barrier.mipCount == 0 ? VK_REMAINING_MIP_LEVELS : barrier.mipCount;
    imageBarrier.subresourceRange.baseMipLevel = barrier.mip;
    imageBarrier.subresourceRange.aspectMask = aspectMask(barrier.image->format());

    VkDependencyInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
    info.imageMemoryBarrierCount = 1;
    info.pImageMemoryBarriers = &imageBarrier;
    vkCmdPipelineBarrier2(_buffer, &info);
    _stats.barriers++;
}

void canta::CommandBuffer::barrier(canta::BufferBarrier barrier) {
    VkBufferMemoryBarrier2 bufferBarrier = {};
    bufferBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2;
    bufferBarrier.buffer = barrier.buffer->buffer();
    bufferBarrier.srcStageMask = static_cast<VkPipelineStageFlagBits>(barrier.srcStage);
    bufferBarrier.dstStageMask = static_cast<VkPipelineStageFlagBits>(barrier.dstStage);
    bufferBarrier.srcAccessMask = static_cast<VkAccessFlagBits>(barrier.srcAccess);
    bufferBarrier.dstAccessMask = static_cast<VkAccessFlagBits>(barrier.dstAccess);
    bufferBarrier.srcQueueFamilyIndex = barrier.srcQueue;
    bufferBarrier.dstQueueFamilyIndex = barrier.dstQueue;
    bufferBarrier.offset = barrier.offset;
    bufferBarrier.size = barrier.size == 0 ? VK_WHOLE_SIZE : barrier.size;

    VkDependencyInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
    info.bufferMemoryBarrierCount = 1;
    info.pBufferMemoryBarriers = &bufferBarrier;
    vkCmdPipelineBarrier2(_buffer, &info);
    _stats.barriers++;
}

void canta::CommandBuffer::barrier(canta::MemoryBarrier barrier) {
    VkMemoryBarrier2 memoryBarrier = {};
    memoryBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER_2;
    memoryBarrier.srcStageMask = static_cast<VkPipelineStageFlagBits>(barrier.srcStage);
    memoryBarrier.dstStageMask = static_cast<VkPipelineStageFlagBits>(barrier.dstStage);
    memoryBarrier.srcAccessMask = static_cast<VkAccessFlagBits>(barrier.srcAccess);
    memoryBarrier.dstAccessMask = static_cast<VkAccessFlagBits>(barrier.dstAccess);

    VkDependencyInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
    info.memoryBarrierCount = 1;
    info.pMemoryBarriers = &memoryBarrier;
    vkCmdPipelineBarrier2(_buffer, &info);
    _stats.barriers++;
}

void canta::CommandBuffer::pushDebugLabel(std::string_view label, std::array<f32, 4> colour) {
#ifndef NDEBUG
    VkDebugUtilsLabelEXT labelInfo = {};
    labelInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
    labelInfo.pLabelName = label.data();
    for (u32 i = 0; i < 4; i++)
        labelInfo.color[i] = colour[i];
    vkCmdBeginDebugUtilsLabelEXT(_buffer, &labelInfo);
#endif
}

void canta::CommandBuffer::popDebugLabel() {
#ifndef NDEBUG
    vkCmdEndDebugUtilsLabelEXT(_buffer);
#endif
}

void canta::CommandBuffer::writeMarker(canta::PipelineStage stage, std::array<u8, util::debugMarkerSize> marker) {
#ifndef NDEBUG
    if (_device->_markerBuffers[_device->flyingIndex()]) {
        vkCmdWriteBufferMarkerAMD(_buffer, static_cast<VkPipelineStageFlagBits>(stage), _device->_markerBuffers[_device->flyingIndex()]->buffer(), _device->_markerOffset, _device->_marker);
//        _device->_markerCommands[_device->flyingIndex()].push_back(command.data());
        _device->_markerCommands[_device->flyingIndex()].push_back(marker);
        _device->_markerOffset += sizeof(u32);
        _device->_marker++;
    }
#endif
}