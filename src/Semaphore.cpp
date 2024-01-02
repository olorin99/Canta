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