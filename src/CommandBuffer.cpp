#include "Canta/CommandBuffer.h"
#include <Canta/Device.h>

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

auto canta::CommandBuffer::begin() -> bool {
    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    return vkBeginCommandBuffer(_buffer, &beginInfo) == VK_SUCCESS;
}

auto canta::CommandBuffer::end() -> bool {
    return vkEndCommandBuffer(_buffer);
}

auto canta::CommandBuffer::submit(std::span<Semaphore::Pair> waitSemaphores, std::span<Semaphore::Pair> signalSemaphores, VkFence fence) -> std::expected<bool, Error> {
    VkSemaphoreSubmitInfo waits[waitSemaphores.size()] = {};
    VkSemaphoreSubmitInfo signals[signalSemaphores.size()] = {};

    for (u32 i = 0; i < waitSemaphores.size(); i++) {
        auto& wait = waitSemaphores[i];
        waits[i].sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
        waits[i].semaphore = wait.semaphore->semaphore();
        waits[i].value = wait.value;
        waits[i].stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
        waits[i].deviceIndex = 0;
    }
    for (u32 i = 0; i < signalSemaphores.size(); i++) {
        auto& signal = signalSemaphores[i];
        signals[i].sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
        signals[i].semaphore = signal.semaphore->semaphore();
        signals[i].value = signal.value;
        signals[i].stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
        signals[i].deviceIndex = 0;
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