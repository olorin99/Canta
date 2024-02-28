#ifndef CANTA_SHADERMODULE_H
#define CANTA_SHADERMODULE_H

#include <Ende/platform.h>
#include <volk.h>
#include <Canta/Enums.h>
#include <span>
#include <string_view>
#include <Canta/ShaderInterface.h>

namespace canta {

    class Device;

    class ShaderModule {
    public:

        struct CreateInfo {
            std::span<const u32> spirv = {};
            ShaderStage stage = ShaderStage::NONE;
            std::string_view name = {};
        };

        ShaderModule() = default;

        ~ShaderModule();

        ShaderModule(ShaderModule&& rhs) noexcept;
        auto operator=(ShaderModule&& rhs) noexcept -> ShaderModule&;

        auto stage() const -> ShaderStage { return _stage; }

        auto module() const -> VkShaderModule { return _module; }

        auto interface() -> ShaderInterface& { return _interface; }

    private:
        friend Device;

        Device* _device = nullptr;
        VkShaderModule _module = VK_NULL_HANDLE;
        ShaderStage _stage = ShaderStage::NONE;
        ShaderInterface _interface = {};
        std::string _name = {};

    };

}

#endif //CANTA_SHADERMODULE_H
