#include "Canta/Semaphore.h"
#include <Canta/Device.h>

canta::Semaphore::~Semaphore() {
    if (_device && _semaphore)
        vkDestroySemaphore(_device->logicalDevice(), _semaphore, nullptr);
}

canta::Semaphore::Semaphore(canta::Semaphore &&rhs) noexcept {
    std::swap(_device, rhs._device);
    std::swap(_semaphore, rhs._semaphore);
    std::swap(_value, rhs._value);
    std::swap(_isTimeline, rhs._isTimeline);
}

auto canta::Semaphore::operator=(canta::Semaphore &&rhs) noexcept -> Semaphore & {
    std::swap(_device, rhs._device);
    std::swap(_semaphore, rhs._semaphore);
    std::swap(_value, rhs._value);
    std::swap(_isTimeline, rhs._isTimeline);
    return *this;
}

auto canta::Semaphore::gpuValue() const -> u64 {
    u64 value = 0;
    vkGetSemaphoreCounterValue(_device->logicalDevice(), _semaphore, &value);
    return value;
}

auto canta::Semaphore::wait(u64 value, u64 timeout) -> std::expected<bool, VulkanError> {
    VkSemaphoreWaitInfo waitInfo = {};
    waitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO;
    waitInfo.semaphoreCount = 1;
    waitInfo.pSemaphores = &_semaphore;
    waitInfo.pValues = &value;
    auto result = vkWaitSemaphores(_device->logicalDevice(), &waitInfo, timeout);
    if (result != VK_SUCCESS && result != VK_TIMEOUT)
        return std::unexpected(static_cast<VulkanError>(result));
    return true;
}

auto canta::Semaphore::signal(u64 value) -> std::expected<bool, VulkanError> {
    if (!_device || !_semaphore)
        return std::unexpected(VulkanError::VK_ERROR_UNKNOWN);
    VkSemaphoreSignalInfo signalInfo = {};
    signalInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SIGNAL_INFO;
    signalInfo.semaphore = _semaphore;
    signalInfo.value = value;
    auto result = vkSignalSemaphore(_device->logicalDevice(), &signalInfo);
    if (result != VK_SUCCESS && result != VK_TIMEOUT)
        return std::unexpected(static_cast<VulkanError>(result));
    if (result != VK_TIMEOUT)
        _value = value;
    return true;
}