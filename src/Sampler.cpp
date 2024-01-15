#include "Canta/Sampler.h"
#include <Canta/Device.h>

canta::Sampler::~Sampler() {
    if (!_device)
        return;
    vkDestroySampler(_device->logicalDevice(), _sampler, nullptr);
}

canta::Sampler::Sampler(canta::Sampler &&rhs) noexcept {
    std::swap(_device, rhs._device);
    std::swap(_sampler, rhs._sampler);
}

auto canta::Sampler::operator=(canta::Sampler &&rhs) noexcept -> Sampler & {
    std::swap(_device, rhs._device);
    std::swap(_sampler, rhs._sampler);
    return *this;
}