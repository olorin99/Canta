#ifndef CANTA_COMMANDPOOL_H
#define CANTA_COMMANDPOOL_H

#include <Ende/platform.h>
#include <volk.h>
#include <Canta/Enums.h>
#include <vector>
#include <Canta/CommandBuffer.h>

namespace canta {

    class Device;

    class CommandPool {
    public:

        struct CreateInfo {
            QueueType queueType = QueueType::GRAPHICS;
            std::string_view name = {};
        };

        CommandPool() = default;

        ~CommandPool();

        CommandPool(CommandPool&& rhs) noexcept;
        auto operator=(CommandPool&& rhs) noexcept -> CommandPool&;

        void reset();

        auto getBuffer() -> CommandBuffer&;

        auto buffers() -> std::span<CommandBuffer> { return _buffers; }

        auto bufferCount() const -> u32 { return _buffers.size(); }

    private:
        friend Device;

        Device* _device = nullptr;
        VkCommandPool _pool = VK_NULL_HANDLE;
        std::vector<CommandBuffer> _buffers = {};
        QueueType _queueType = QueueType::NONE;
        u32 _index = 0;

    };

}

#endif //CANTA_COMMANDPOOL_H
