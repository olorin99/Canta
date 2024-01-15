#include "Canta/Pipeline.h"
#include <Canta/Device.h>

canta::Pipeline::~Pipeline() {
    if (!_device)
        return;
    vkDestroyPipeline(_device->logicalDevice(), _pipeline, nullptr);
    vkDestroyPipelineLayout(_device->logicalDevice(), _layout, nullptr);
}

canta::Pipeline::Pipeline(canta::Pipeline &&rhs) noexcept {
    std::swap(_device, rhs._device);
    std::swap(_pipeline, rhs._pipeline);
    std::swap(_layout, rhs._layout);
    std::swap(_mode, rhs._mode);
}

auto canta::Pipeline::operator=(canta::Pipeline &&rhs) noexcept -> Pipeline & {
    std::swap(_device, rhs._device);
    std::swap(_pipeline, rhs._pipeline);
    std::swap(_layout, rhs._layout);
    std::swap(_mode, rhs._mode);
    return *this;
}