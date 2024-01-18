#ifndef CANTA_PIPELINEMANAGER_H
#define CANTA_PIPELINEMANAGER_H

#include <Canta/Device.h>
#include <Canta/Pipeline.h>
#include <tsl/robin_map.h>
#include <filesystem>

namespace canta {
    struct ShaderDescription {
        std::filesystem::path path = {};
        std::span<u32> spirv = {};
        ShaderStage stage = ShaderStage::NONE;
    };
}

namespace std {
    template <> struct hash<canta::Pipeline::CreateInfo> {
        size_t operator()(const canta::Pipeline::CreateInfo& object) const;
    };
    template <> struct hash<canta::ShaderDescription> {
        size_t operator()(const canta::ShaderDescription& object) const;
    };
}

namespace canta {

    bool operator==(const Pipeline::CreateInfo& lhs, const Pipeline::CreateInfo& rhs);
    bool operator==(const ShaderDescription& lhs, const ShaderDescription& rhs);

    class PipelineManager {
    public:

        struct CreateInfo {
            Device* device = nullptr;
            std::filesystem::path rootPath = {};
        };

        PipelineManager(CreateInfo info);

        struct ShaderInfo {
            std::filesystem::path path = {};
            ShaderStage stage = ShaderStage::NONE;
        };

        auto getShader(ShaderDescription info) -> ShaderHandle;

        auto getPipeline(Pipeline::CreateInfo info) -> PipelineHandle;

    private:

        Device* _device = nullptr;
        std::filesystem::path _rootPath = {};
        tsl::robin_map<ShaderDescription, ShaderHandle> _shaders;
        tsl::robin_map<Pipeline::CreateInfo, PipelineHandle, std::hash<Pipeline::CreateInfo>> _pipelines;

        std::vector<std::filesystem::path> _watchedPaths = {};

    };

}

#endif //CANTA_PIPELINEMANAGER_H
