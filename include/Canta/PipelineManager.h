#ifndef CANTA_PIPELINEMANAGER_H
#define CANTA_PIPELINEMANAGER_H

#include <Canta/Device.h>
#include <Canta/Pipeline.h>
#include <tsl/robin_map.h>
#include <filesystem>
#include <Ende/filesystem/FileWatcher.h>
#include <slang.h>
#include <slang-com-ptr.h>

namespace canta {

    struct Macro {
        std::string name;
        std::string value;
    };

    struct ShaderDescription {
        std::filesystem::path path = {};
        std::vector<u32> spirv = {};
        std::string_view slang = {};
        std::vector<Macro> macros = {};
        std::string_view entry = "main";

        explicit operator bool() const {
            return !spirv.empty() || !path.empty() || !slang.empty();
        }
    };

    struct PipelineDescription {
        ShaderDescription vertex = {};
        ShaderDescription tesselationControl = {};
        ShaderDescription tesselationEvaluation = {};
        ShaderDescription geometry = {};
        ShaderDescription fragment = {};
        ShaderDescription compute = {};
        ShaderDescription rayGen = {};
        ShaderDescription anyHit = {};
        ShaderDescription closestHit = {};
        ShaderDescription miss = {};
        ShaderDescription intersection = {};
        ShaderDescription callable = {};
        ShaderDescription task = {};
        ShaderDescription mesh = {};
        std::optional<ende::math::Vec<3, u32>> localSize = {};
        std::vector<SpecializationConstant> specializationConstants = {};
        RasterState rasterState = {};
        DepthState depthState = {};
        BlendState blendState = {};
        std::vector<VertexInputBinding> inputBindings = {};
        std::vector<VertexInputAttribute> inputAttributes = {};
        PrimitiveTopology topology = PrimitiveTopology::TRIANGLE_LIST;
        bool primitiveRestart = false;
        std::vector<Format> colourFormats = {};
        Format depthFormat = Format::UNDEFINED;
        std::string name = {};
    };
}

namespace std {
    template <> struct hash<canta::PipelineDescription> {
        size_t operator()(const canta::PipelineDescription& object) const noexcept;
    };
    template <> struct hash<canta::ShaderDescription> {
        size_t operator()(const canta::ShaderDescription& object) const noexcept;
    };
}

namespace canta {

    bool operator==(const PipelineDescription& lhs, const PipelineDescription& rhs);
    bool operator==(const ShaderDescription& lhs, const ShaderDescription& rhs);

    class PipelineManager {
    public:

        struct CreateInfo {
            Device* device = nullptr;
            std::filesystem::path rootPath = CANTA_SRC_DIR;
            std::span<std::filesystem::path> searchPaths = {};
            bool rowMajor = true;
        };

        static auto create(CreateInfo info) -> PipelineManager;

        PipelineManager() = default;

        PipelineManager(PipelineManager&& rhs) noexcept;
        auto operator=(PipelineManager&& rhs) noexcept -> PipelineManager&;

        [[nodiscard]] auto getPipeline(PipelineDescription info, const PipelineHandle& oldPipeline = {}) -> std::expected<PipelineHandle, Error>;

        [[nodiscard]] auto getPipeline(const std::filesystem::path& path, std::span<const Macro> additionalMacros = {}, const std::vector<SpecializationConstant>& specializationConstants = {}) -> std::expected<PipelineHandle, Error>;

        [[nodiscard]] auto reload() -> std::expected<bool, Error>;

        [[nodiscard]] auto reload(const PipelineHandle& pipeline) -> std::expected<PipelineHandle, Error>;


        void addVirtualFile(const std::filesystem::path& path, const std::string& contents);

    private:

        [[nodiscard]] auto findVirtualFile(const std::filesystem::path& path) const -> std::expected<std::string, Error>;

        [[nodiscard]] auto compileSlang(std::string_view name, std::string_view slang, ShaderStage stage, std::span<const Macro> macros = {}) -> std::expected<std::vector<u32>, std::string>;
        [[nodiscard]] auto createSlangSession(std::span<const Macro> macros = {}) -> std::expected<Slang::ComPtr<slang::ISession>, std::string>;

        Device* _device = nullptr;
        std::vector<std::filesystem::path> _searchPaths = {};
        bool _rowMajor = true;

        tsl::robin_map<PipelineDescription, PipelineHandle, std::hash<PipelineDescription>> _pipelines = {};

        ende::fs::FileWatcher _fileWatcher = {};
        std::vector<std::pair<std::string, std::string>> _virtualFiles = {};
        tsl::robin_map<std::filesystem::path, PipelineDescription> _watchedFiles = {};

        Slang::ComPtr<slang::IGlobalSession> _slangGlobalSession = {};
        Slang::ComPtr<slang::ISession> _slangMainSession = {};

    };

}

#endif //CANTA_PIPELINEMANAGER_H
