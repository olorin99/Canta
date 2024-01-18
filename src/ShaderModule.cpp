#include "Canta/ShaderModule.h"
#include <Canta/Device.h>

canta::ShaderModule::~ShaderModule() {
    if (!_device)
        return;
    vkDestroyShaderModule(_device->logicalDevice(), _module, nullptr);
}

canta::ShaderModule::ShaderModule(canta::ShaderModule &&rhs) noexcept {
    std::swap(_device, rhs._device);
    std::swap(_module, rhs._module);
    std::swap(_stage, rhs._stage);
    std::swap(_interface, rhs._interface);
}

auto canta::ShaderModule::operator=(canta::ShaderModule &&rhs) noexcept -> ShaderModule & {
    std::swap(_device, rhs._device);
    std::swap(_module, rhs._module);
    std::swap(_stage, rhs._stage);
    std::swap(_interface, rhs._interface);
    return *this;
}