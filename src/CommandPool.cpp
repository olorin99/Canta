#include "Canta/CommandPool.h"
#include <Canta/Device.h>

template<> u32 canta::CommandHandle ::s_hash = 0;

canta::CommandPool::~CommandPool() {
    if (!_device)
        return;
    reset();

    for (auto& buffer : _buffers) {
        auto b = buffer.buffer();
        vkFreeCommandBuffers(_device->logicalDevice(), _pool, 1, &b);
    }

    vkDestroyCommandPool(_device->logicalDevice(), _pool, nullptr);
}

canta::CommandPool::CommandPool(CommandPool &&rhs) noexcept {
    std::swap(_device, rhs._device);
    std::swap(_pool, rhs._pool);
    std::swap(_buffers, rhs._buffers);
    std::swap(_index, rhs._index);
    std::swap(_queueType, rhs._queueType);
    std::swap(_commandBuffers, rhs._commandBuffers);
}

auto canta::CommandPool::operator=(CommandPool &&rhs) noexcept -> CommandPool & {
    std::swap(_device, rhs._device);
    std::swap(_pool, rhs._pool);
    std::swap(_buffers, rhs._buffers);
    std::swap(_index, rhs._index);
    std::swap(_queueType, rhs._queueType);
    std::swap(_commandBuffers, rhs._commandBuffers);
    return *this;
}

void canta::CommandPool::reset() {
    _commandBuffers.clearQueue([this] (auto& buffer) {
        auto b = buffer.buffer();
        vkFreeCommandBuffers(_device->logicalDevice(), _pool, 1, &b);
    });
    VK_TRY(vkResetCommandPool(_device->logicalDevice(), _pool, 0));
    _index = 0;
}

auto canta::CommandPool::getBuffer() -> CommandHandle {
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = _pool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;
    VkCommandBuffer commandBuffer;
    VK_TRY(vkAllocateCommandBuffers(_device->logicalDevice(), &allocInfo, &commandBuffer));

    CommandBuffer buffer = {};
    buffer._device = _device;
    buffer._buffer = commandBuffer;
    buffer._queueType = _queueType;

    auto handle = _commandBuffers.allocate();
    *handle = std::move(buffer);
    return handle;
}