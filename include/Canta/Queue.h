#ifndef CANTA_QUEUE_H
#define CANTA_QUEUE_H

#include <Canta/CommandBuffer.h>
#include <Canta/CommandPool.h>
#include <Canta/Semaphore.h>
#include <Ende/platform.h>
#include <expected>
#include <volk.h>

namespace canta {

class Device;
struct SemaphorePair;

class Queue {
  public:
    Queue() = default;

    auto queue() const -> VkQueue { return _queue; }
    auto familyIndex() const -> u32 { return _familyIndex; }
    auto queueIndex() const -> u32 { return _queueIndex; }
    auto timeline() const -> SemaphoreHandle { return _timeline; }

    [[nodiscard]] auto submit(std::span<CommandHandle> commandBuffers, std::span<SemaphorePair> waits = {}, std::span<SemaphorePair> signals = {}, VkFence fence = VK_NULL_HANDLE) -> std::expected<bool, VulkanError>;

  private:
    friend Device;

    Device *_device = nullptr;
    VkQueue _queue = VK_NULL_HANDLE;
    u32 _familyIndex = 0;
    u32 _queueIndex = 0;
    SemaphoreHandle _timeline = {};
    std::mutex _mutex = {};
};

} // namespace canta

#endif // CANTA_QUEUE_H
