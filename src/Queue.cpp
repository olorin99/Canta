#include "Canta/Queue.h"
#include <Canta/Device.h>

auto canta::Queue::submit(std::span<CommandHandle> commandBuffers, std::span<SemaphorePair> waits, std::span<SemaphorePair> signals, VkFence fence) -> std::expected<bool, VulkanError> {
    VkSemaphoreSubmitInfo waitInfos[waits.size()];
    VkSemaphoreSubmitInfo signalInfos[signals.size() + 1];

    for (u32 i = 0; i < waits.size(); i++) {
        auto& wait = waits[i];
        waitInfos[i].sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
        waitInfos[i].semaphore = wait.semaphore->semaphore();
        waitInfos[i].value = wait.value;
        waitInfos[i].stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
        waitInfos[i].deviceIndex = 0;
        waitInfos[i].pNext = nullptr;
    }
    u32 signalCount = 0;
    for (; signalCount < signals.size(); signalCount++) {
        auto& signal = signals[signalCount];
        signalInfos[signalCount].sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
        signalInfos[signalCount].semaphore = signal.semaphore->semaphore();
        signalInfos[signalCount].value = signal.value;
        signalInfos[signalCount].stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
        signalInfos[signalCount].deviceIndex = 0;
        signalInfos[signalCount].pNext = nullptr;
    }
    if (_device->resourceTimeline()) {
        // resource timeline signal on each queue submit
        signalInfos[signalCount].sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
        signalInfos[signalCount].semaphore = _device->resourceTimeline()->semaphore();
        signalInfos[signalCount].value = _device->resourceTimeline()->increment();
        signalInfos[signalCount].stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
        signalInfos[signalCount].deviceIndex = 0;
        signalInfos[signalCount].pNext = nullptr;
        signalCount++;
    }

    VkCommandBufferSubmitInfo commandInfos[commandBuffers.size()];
    for (u32 i = 0; i < commandBuffers.size(); i++) {
        auto& commandBuffer = commandBuffers[i];
        commandBuffer->end();
        commandInfos[i].sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
        commandInfos[i].commandBuffer = commandBuffer->buffer();
        commandInfos[i].deviceMask = 0;
        commandInfos[i].pNext = nullptr;
    }

    VkSubmitInfo2 submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
    submitInfo.waitSemaphoreInfoCount = waits.size();
    submitInfo.pWaitSemaphoreInfos = waitInfos;
    submitInfo.signalSemaphoreInfoCount = signalCount;
    submitInfo.pSignalSemaphoreInfos = signalInfos;
    submitInfo.commandBufferInfoCount = commandBuffers.size();
    submitInfo.pCommandBufferInfos = commandInfos;

    std::unique_lock<std::mutex> lock(_mutex);

    auto result = vkQueueSubmit2(_queue, 1, &submitInfo, fence);
    if (result != VK_SUCCESS)
        return std::unexpected(static_cast<VulkanError>(result));
    return true;
}