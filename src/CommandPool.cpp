#include "Canta/CommandPool.h"
#include <Canta/Device.h>

canta::CommandPool::~CommandPool() {
    if (!_device)
        return;

//    for (auto& buffer : _commandBuffers)
//        vkFreeCommandBuffers(_device->logicalDevice(), _pool, 1, &buffer);

    vkDestroyCommandPool(_device->logicalDevice(), _pool, nullptr);
}

canta::CommandPool::CommandPool(canta::CommandPool &&rhs) noexcept {
    std::swap(_device, rhs._device);
    std::swap(_pool, rhs._pool);
    std::swap(_buffers, rhs._buffers);
    std::swap(_index, rhs._index);
    std::swap(_queueType, rhs._queueType);
}

auto canta::CommandPool::operator=(canta::CommandPool &&rhs) noexcept -> CommandPool & {
    std::swap(_device, rhs._device);
    std::swap(_pool, rhs._pool);
    std::swap(_buffers, rhs._buffers);
    std::swap(_index, rhs._index);
    std::swap(_queueType, rhs._queueType);
    return *this;
}

void canta::CommandPool::reset() {
    VK_TRY(vkResetCommandPool(_device->logicalDevice(), _pool, 0));
}

auto canta::CommandPool::getBuffer() -> CommandBuffer & {
    if (_index < _buffers.size())
        return _buffers[_index++];

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

    _buffers.push_back(std::move(buffer));
    _index++;
    return _buffers.back();
}