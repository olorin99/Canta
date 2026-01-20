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

auto canta::Pipeline::localSize(ShaderStage stage) const -> std::optional<ende::math::Vec<3, u32>> {
    const auto interfaceLocalSize = _interface.localSize(stage);

    auto localSize = interfaceLocalSize ? interfaceLocalSize : ende::math::Vec<3, u32>{1, 1, 1};

    if (const auto specializationLocalSize = _size) {
        if (specializationLocalSize->x() != 1)
            (*localSize)[0] = specializationLocalSize->x();
        if (specializationLocalSize->y() != 1)
            (*localSize)[1] = specializationLocalSize->z();
        if (specializationLocalSize->z() != 1)
            (*localSize)[2] = specializationLocalSize->z();
    }

    return localSize;
}