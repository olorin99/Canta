#include "Canta/CommandBuffer.h"
#include <Canta/Device.h>
#include <Canta/Pipeline.h>
#include <Canta/Buffer.h>
#include <Canta/Image.h>

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

    auto queue = _device->queue(_queueType);

    auto result = vkQueueSubmit2(queue, 1, &submitInfo, fence);
    if (result != VK_SUCCESS)
        return std::unexpected(static_cast<Error>(result));
    return true;
}

auto canta::CommandBuffer::begin() -> bool {
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
        colourAttachments[i] = {
                .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
                .imageView = info.colourAttachments[i].imageView,
                .imageLayout = static_cast<VkImageLayout>(info.colourAttachments[i].imageLayout),
                .resolveMode = VK_RESOLVE_MODE_NONE,
                .resolveImageView = VK_NULL_HANDLE,
                .resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                .clearValue = { 0, 0, 0, 1 }
        };
    }
    renderingInfo.colorAttachmentCount = colourAttachments.size();
    renderingInfo.pColorAttachments = colourAttachments.data();
    VkRenderingAttachmentInfo depthAttachment = {
            .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
            .imageView = info.depthAttachment,
            .imageLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .resolveMode = VK_RESOLVE_MODE_NONE,
            .resolveImageView = VK_NULL_HANDLE,
            .resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .clearValue = { 0, 0, 0, 1 }
    };
    if (info.depthAttachment != VK_NULL_HANDLE) {
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

void canta::CommandBuffer::bindPipeline(PipelineHandle pipeline) {
    if (!pipeline)
        return;
    vkCmdBindPipeline(_buffer, pipeline->mode() == PipelineMode::GRAPHICS ? VK_PIPELINE_BIND_POINT_GRAPHICS : VK_PIPELINE_BIND_POINT_COMPUTE, pipeline->pipeline());
    _currentPipeline = pipeline;
    auto set = _device->bindlessSet();
    vkCmdBindDescriptorSets(_buffer, pipeline->mode() == PipelineMode::GRAPHICS ? VK_PIPELINE_BIND_POINT_GRAPHICS : VK_PIPELINE_BIND_POINT_COMPUTE, _currentPipeline->layout(), 0, 1, &set, 0, nullptr);
}

void canta::CommandBuffer::pushConstants(canta::ShaderStage stage, std::span<const u8> data, u32 offset) {
    assert(offset + data.size() <= 128);
    vkCmdPushConstants(_buffer, _currentPipeline->layout(), static_cast<VkShaderStageFlagBits>(stage), offset, data.size(), data.data());
}

void canta::CommandBuffer::draw(u32 count, u32 instanceCount, u32 first, u32 firstInstance) {
    assert(_currentPipeline);
    assert(_currentPipeline->mode() == PipelineMode::GRAPHICS);
    vkCmdDraw(_buffer, count, instanceCount, first, firstInstance);
}

void canta::CommandBuffer::dispatchWorkgroups(u32 x, u32 y, u32 z) {
    assert(_currentPipeline);
    assert(_currentPipeline->mode() == PipelineMode::COMPUTE);
    assert(_currentPipeline->interface().stagePresent(ShaderStage::COMPUTE));
    vkCmdDispatch(_buffer, x, y, z);
}

void canta::CommandBuffer::dispatchThreads(u32 x, u32 y, u32 z) {
    auto localSize = _currentPipeline->interface().localSize(ShaderStage::COMPUTE);
    dispatchWorkgroups(std::ceil(static_cast<f32>(x) / static_cast<f32>(localSize.x())), std::ceil(static_cast<f32>(y) / static_cast<f32>(localSize.y())), std::ceil(static_cast<f32>(z) / static_cast<f32>(localSize.z())));
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
    blit.srcOffsets[0] = { 0, 0, 0 };
    blit.srcOffsets[1] = { static_cast<i32>(info.src->width()), static_cast<i32>(info.src->height()), static_cast<i32>(info.src->depth()) };

    blit.dstSubresource = dstSubresource;
    blit.dstOffsets[0] = { 0, 0, 0 };
    blit.dstOffsets[1] = { static_cast<i32>(info.dst->width()), static_cast<i32>(info.dst->height()), static_cast<i32>(info.dst->depth()) };

    vkCmdBlitImage(_buffer, info.src->image(), static_cast<VkImageLayout>(info.srcLayout), info.dst->image(), static_cast<VkImageLayout>(info.dstLayout), 1, &blit, static_cast<VkFilter>(info.filter));
}

void canta::CommandBuffer::clearImage(ImageHandle handle, ImageLayout layout, const std::array<f32, 4> &clearColour) {
    VkClearColorValue clearValue = { clearColour[0], clearColour[1], clearColour[2], clearColour[3] };
    VkImageSubresourceRange range = {};
    range.aspectMask = aspectMask(handle->format());
    range.baseMipLevel = 0;
    range.levelCount = handle->mips();
    range.baseArrayLayer = 0;
    range.layerCount = handle->layers();

    vkCmdClearColorImage(_buffer, handle->image(), static_cast<VkImageLayout>(layout), &clearValue, 1, &range);
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

    imageBarrier.subresourceRange.layerCount = 1;
    imageBarrier.subresourceRange.baseArrayLayer = 0;
    imageBarrier.subresourceRange.levelCount = 1;
    imageBarrier.subresourceRange.baseMipLevel = 0;
    imageBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

    VkDependencyInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
    info.imageMemoryBarrierCount = 1;
    info.pImageMemoryBarriers = &imageBarrier;
    vkCmdPipelineBarrier2(_buffer, &info);
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
    info.imageMemoryBarrierCount = 1;
    info.pBufferMemoryBarriers = &bufferBarrier;
    vkCmdPipelineBarrier2(_buffer, &info);
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
    info.imageMemoryBarrierCount = 1;
    info.pMemoryBarriers = &memoryBarrier;
    vkCmdPipelineBarrier2(_buffer, &info);
}