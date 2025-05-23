#ifndef CANTA_PIPELINEMANAGER_H
#define CANTA_PIPELINEMANAGER_H

#include <Canta/Device.h>
#include <Canta/Pipeline.h>
#include <tsl/robin_map.h>
#include <filesystem>
#include <Ende/filesystem/FileWatcher.h>
#ifdef CANTA_USE_SLANG
#include <slang.h>
#include <slang-com-ptr.h>
#endif

namespace canta {

    struct Macro {
        std::string name;
        std::string value;
    };

    struct ShaderDescription {
        std::filesystem::path path = {};
        std::vector<u32> spirv = {};
        std::string_view glsl = {};
        std::string_view slang = {};
        std::vector<Macro> macros = {};
        ShaderStage stage = ShaderStage::NONE;
        std::string_view name = {};
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

        PipelineManager(PipelineManager&& rhs) noexcept;
        auto operator=(PipelineManager&& rhs) noexcept -> PipelineManager&;

        PipelineManager() = default;

        auto getShader(ShaderDescription info) -> std::expected<ShaderHandle, Error>;
        auto getPipeline(Pipeline::CreateInfo info) -> std::expected<PipelineHandle, Error>;
        auto getPipeline(const Pipeline& old, Pipeline::CreateInfo overrideInfo) -> std::expected<PipelineHandle, Error>;
        auto getPipeline(const std::filesystem::path& path, std::span<const Macro> additionalMacros = {}, const std::vector<SpecializationConstant>& specializationConstants = {}) -> std::expected<PipelineHandle, Error>;

        auto reload(ShaderHandle shader) -> std::expected<ShaderHandle, Error>;
        auto reload(PipelineHandle pipeline) -> std::expected<PipelineHandle, Error>;

        auto reload(ShaderDescription description) -> std::expected<ShaderHandle, Error>;
        auto reload(Pipeline::CreateInfo info) -> std::expected<PipelineHandle, Error>;

        void reloadAll(bool force = false);

        void addVirtualFile(const std::filesystem::path& path, const std::string& contents);
        auto shaders() const -> const tsl::robin_map<ShaderDescription, ShaderHandle>& { return _shaders; }

        auto pipelines() const -> const tsl::robin_map<Pipeline::CreateInfo, PipelineHandle, std::hash<Pipeline::CreateInfo>>& { return _pipelines; }

    private:

        auto createShader(ShaderDescription info, ShaderHandle handle = {}) -> std::expected<ShaderHandle, Error>;

        auto compileGLSL(std::string_view name, std::string_view glsl, ShaderStage stage, std::span<const Macro> macros = {}) -> std::expected<std::vector<u32>, std::string>;
        auto compileSlang(std::string_view name, std::string_view slang, ShaderStage stage, std::span<const Macro> macros = {}) -> std::expected<std::vector<u32>, std::string>;

        auto findVirtualFile(const std::filesystem::path& path) -> std::expected<std::string, Error>;

        Device* _device = nullptr;
        std::filesystem::path _rootPath = {};
        tsl::robin_map<ShaderDescription, ShaderHandle> _shaders;
        tsl::robin_map<Pipeline::CreateInfo, PipelineHandle, std::hash<Pipeline::CreateInfo>> _pipelines;

        ende::fs::FileWatcher _fileWatcher = {};
        tsl::robin_map<std::filesystem::path, std::pair<ShaderDescription, std::vector<Pipeline::CreateInfo>>> _watchedPipelines;

        std::vector<std::pair<std::string, std::string>> _virtualFiles = {};

#ifdef CANTA_USE_SLANG
        Slang::ComPtr<slang::IGlobalSession> _slangGlobalSession = {};
#endif

    };

}

#endif //CANTA_PIPELINEMANAGER_H
