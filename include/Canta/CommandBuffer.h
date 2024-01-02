#ifndef CANTA_COMMANDBUFFER_H
#define CANTA_COMMANDBUFFER_H

#include <Ende/platform.h>
#include <volk.h>
#include <Canta/Semaphore.h>
#include <span>

namespace canta {

    class CommandPool;
    class Device;

    class CommandBuffer {
    public:

        CommandBuffer(CommandBuffer&& rhs) noexcept;
        auto operator=(CommandBuffer&& rhs) noexcept -> CommandBuffer&;

        auto buffer() const -> VkCommandBuffer { return _buffer; }


        auto begin() -> bool;
        auto end() -> bool;


        auto submit(std::span<Semaphore::Pair> waitSemaphores, std::span<Semaphore::Pair> signalSemaphores, VkFence fence = VK_NULL_HANDLE) -> std::expected<bool, Error>;

    private:
        friend CommandPool;

        CommandBuffer() = default;

        Device* _device = nullptr;
        VkCommandBuffer _buffer = VK_NULL_HANDLE;
        QueueType _queueType = QueueType::NONE;

    };

}

#endif //CANTA_COMMANDBUFFER_H
