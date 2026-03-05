#ifndef CANTA_COMMANDPOOL_H
#define CANTA_COMMANDPOOL_H

#include <Ende/platform.h>
#include <volk.h>
#include <Canta/Enums.h>
#include <vector>
#include <Canta/CommandBuffer.h>

namespace canta {

    class Device;

    using CommandHandle = Handle<CommandBuffer, ResourceList<CommandBuffer>>;

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

        [[nodiscard]] auto getBuffer() -> CommandHandle;

        auto bufferCount() const -> u32 { return _commandBuffers.used(); }

    private:
        friend Device;

        Device* _device = nullptr;
        VkCommandPool _pool = VK_NULL_HANDLE;
        std::vector<CommandBuffer> _buffers = {};
        QueueType _queueType = QueueType::NONE;
        u32 _index = 0;
        ResourceList<CommandBuffer> _commandBuffers = {};

    };

}

#endif //CANTA_COMMANDPOOL_H
