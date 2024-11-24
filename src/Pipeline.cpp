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
    std::swap(_interface, rhs._interface);
    std::swap(_name, rhs._name);
    std::swap(_info, rhs._info);
}

auto canta::Pipeline::operator=(canta::Pipeline &&rhs) noexcept -> Pipeline & {
    std::swap(_device, rhs._device);
    std::swap(_pipeline, rhs._pipeline);
    std::swap(_layout, rhs._layout);
    std::swap(_mode, rhs._mode);
    std::swap(_interface, rhs._interface);
    std::swap(_name, rhs._name);
    std::swap(_info, rhs._info);
    return *this;
}