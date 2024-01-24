#ifndef CANTA_PIPELINEMANAGER_H
#define CANTA_PIPELINEMANAGER_H

#include <Canta/Device.h>
#include <Canta/Pipeline.h>
#include <tsl/robin_map.h>
#include <filesystem>
#include <Ende/filesystem/FileWatcher.h>

namespace canta {
    struct ShaderDescription {
        std::filesystem::path path = {};
        std::span<u32> spirv = {};
        std::string_view glsl = {};
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

        static auto create(CreateInfo info) -> PipelineManager;

        PipelineManager() = default;

        auto getShader(ShaderDescription info) -> ShaderHandle;
        auto getPipeline(Pipeline::CreateInfo info) -> PipelineHandle;

        auto reload(ShaderHandle shader) -> ShaderHandle;
        auto reload(PipelineHandle pipeline) -> PipelineHandle;

        void reloadAll(bool force = false);

        void addVirtualFile(const std::filesystem::path& path, const std::string& contents);

    private:

        auto reload(ShaderDescription description) -> ShaderHandle;
        auto reload(Pipeline::CreateInfo info) -> PipelineHandle;

        auto createShader(ShaderDescription info, ShaderHandle handle = {}) -> ShaderHandle;

        struct Macro {
            std::string name;
            std::string value;
        };
        auto compileGLSL(std::string_view glsl, ShaderStage stage, std::span<Macro> macros = {}) -> std::expected<std::vector<u32>, std::string>;

        Device* _device = nullptr;
        std::filesystem::path _rootPath = {};
        tsl::robin_map<ShaderDescription, ShaderHandle> _shaders;
        tsl::robin_map<Pipeline::CreateInfo, PipelineHandle, std::hash<Pipeline::CreateInfo>> _pipelines;

        ende::fs::FileWatcher _fileWatcher = {};
        tsl::robin_map<std::filesystem::path, std::pair<ShaderDescription, std::vector<Pipeline::CreateInfo>>> _watchedPipelines;

        std::vector<std::pair<std::string, std::string>> _virtualFiles = {};

    };

}

#endif //CANTA_PIPELINEMANAGER_H
