#include "Canta/Timer.h"
#include <Canta/Device.h>

canta::Timer::~Timer() {
    if (!_device)
        return;
    _device->destroyTimer(_queryPoolIndex, _index);
}

canta::Timer::Timer(canta::Timer &&rhs) noexcept {
    std::swap(_device, rhs._device);
    std::swap(_queryPoolIndex, rhs._queryPoolIndex);
    std::swap(_index, rhs._index);
    std::swap(_value, rhs._value);
}

auto canta::Timer::operator=(canta::Timer &&rhs) noexcept -> Timer & {
    std::swap(_device, rhs._device);
    std::swap(_queryPoolIndex, rhs._queryPoolIndex);
    std::swap(_index, rhs._index);
    std::swap(_value, rhs._value);
    return *this;
}

void canta::Timer::begin(canta::CommandBuffer &commandBuffer, canta::PipelineStage stage) {
    auto pool = _device->timestampPools()[_queryPoolIndex];
    vkCmdResetQueryPool(commandBuffer.buffer(), pool, _index * 2, 2);
    vkCmdWriteTimestamp2(commandBuffer.buffer(), static_cast<VkPipelineStageFlagBits>(stage), pool, _index * 2);
    _value = 0;
}

void canta::Timer::end(canta::CommandBuffer &commandBuffer, canta::PipelineStage stage) {
    auto pool = _device->timestampPools()[_queryPoolIndex];
    vkCmdWriteTimestamp2(commandBuffer.buffer(), static_cast<VkPipelineStageFlagBits>(stage), pool, _index * 2 + 1);
}

auto canta::Timer::result() -> std::expected<u64, VulkanError> {
    if (_value)
        return _value;

    u64 buffer[2] = {};
    auto pool = _device->timestampPools()[_queryPoolIndex];
    auto res = vkGetQueryPoolResults(_device->logicalDevice(), pool, _index * 2, 2, sizeof(u64) * 2, buffer, sizeof(u64), VK_QUERY_RESULT_64_BIT);
    if (res == VK_NOT_READY)
        return 0;
    if (res != VK_SUCCESS)
        return std::unexpected(static_cast<VulkanError>(res));
    _value = (buffer[1] - buffer[0]) * static_cast<u64>(_device->limits().timestampPeriod);
    return _value;
}