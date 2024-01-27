#include "Canta/util.h"

#include <shaderc/shaderc.hpp>
#include <Ende/filesystem/File.h>
#include <unordered_set>
#include <cassert>
#include <format>

//class FileFinder {
//public:
//
//    ~FileFinder() {
//        std::printf("Destroy");
//    }
//
//    void addSearchPath(const std::string& path) {
//        search_path.push_back(path);
//    }
//
//    std::string FindReadableFilepath(const std::string& filename) const {
//        assert(!filename.empty());
//
//        for (const auto& prefix : search_path) {
//            std::string prefixed_filename = prefix + ((prefix.empty() || prefix.back() == '/') ? "" : "/") + filename;
//            if (std::filesystem::exists(prefixed_filename))
//                return prefixed_filename;
//        }
//        return "";
//    }
//
//    std::string FindRelativeReadableFilepath(const std::string& requesting_file, const std::string& filename) const {
//        assert(!filename.empty());
//        size_t last_slash = requesting_file.find_last_of("/\\");
//        std::string dir_name = requesting_file;
//        if (last_slash != std::string::npos) {
//            dir_name = requesting_file.substr(0, last_slash);
//        }
//        if (dir_name.size() == requesting_file.size()) {
//            dir_name = {};
//        }
//
//        std::string relative_filename = dir_name + ((dir_name.empty() || dir_name.back() == '/') ? "" : "/") + filename;
//        if (std::filesystem::exists(relative_filename))
//            return relative_filename;
//        return FindReadableFilepath(filename);
//    }
//
//private:
//    std::vector<std::string> search_path;
//
//};
//
//struct FileInfo {
//    std::string path;
//    std::string source;
//};
//
//class FileIncluder : public shaderc::CompileOptions::IncluderInterface {
//public:
//
//    explicit FileIncluder(const FileFinder* file_finder) : file_finder(file_finder) {}
//
//    ~FileIncluder() override = default;
//
//    shaderc_include_result* GetInclude(const char* requested_source, shaderc_include_type type, const char* requesting_source, size_t include_depth) override {
//        std::string path = (type == shaderc_include_type_relative) ? file_finder->FindRelativeReadableFilepath(requesting_source, requested_source)
//                                                                   : file_finder->FindReadableFilepath(requested_source);
//
//        if (path.empty())
//            return new shaderc_include_result{"", 0, "unable to open file", 15};
//
//        std::string f = path;
//        auto file = ende::fs::File::open(std::move(path), ende::fs::in);
//        if (!file)
//            return new shaderc_include_result{"", 0, "unable to open file", 15};
//
//        std::string source = file->read();
//        if (source.empty())
//            return new shaderc_include_result{"", 0, "unable to read file", 15};
//
//        included_files.insert(f);
//        FileInfo* info = new FileInfo{ f, source };
//
//        return new shaderc_include_result{info->path.data(), info->path.size(),
//                                          info->source.data(), info->source.size(),
//                                          info};
//    }
//
//    void ReleaseInclude(shaderc_include_result* include_result) override {
//        delete static_cast<FileInfo*>(include_result->user_data);
//        delete include_result;
//    }
//
//    const std::unordered_set<std::string>& file_path_trace() const {
//        return included_files;
//    }
//
//private:
//
//    const FileFinder* file_finder;
//    std::unordered_set<std::string> included_files;
//
//};

struct FileInfo {
    std::string path = {};
    std::string contents = {};
    bool isVirtual = false;
};

class FileFinder {
public:

    explicit FileFinder(std::vector<std::pair<std::string, std::string>>* virtualFileList)
            : _virtualFileList(virtualFileList),
              _searchPaths()
    {}

    explicit FileFinder(FileFinder&& rhs) noexcept {
        std::swap(_virtualFileList, rhs._virtualFileList);
        std::swap(_searchPaths, rhs._searchPaths);
    }

    auto operator=(FileFinder&& rhs) noexcept -> FileFinder& {
        std::swap(_virtualFileList, rhs._virtualFileList);
        std::swap(_searchPaths, rhs._searchPaths);
        return *this;
    }

    FileInfo* findReadableFilepath(const std::string& path) const {
        if (_virtualFileList) {
            for (auto& virtualFile : *_virtualFileList) {
                if (virtualFile.first == path) {
                    return new FileInfo{
                            .path = virtualFile.first,
                            .contents = virtualFile.second,
                            .isVirtual = true
                    };
                }
            }
        }

        for (const auto& prefix : _searchPaths) {
            std::string prefixed_filename = prefix + ((prefix.empty() || prefix.back() == '/') ? "" : "/") + path;
            if (std::filesystem::exists(prefixed_filename)) {
                auto file = ende::fs::File::open(prefixed_filename, ende::fs::in);
                return new FileInfo{
                        .path = prefixed_filename,
                        .contents = file->read(),
                        .isVirtual = false
                };
            }
        }
        return nullptr;
    }

    FileInfo* findRelativeReadableFilepath(const std::string& requestingPath, const std::string& fileName) const {
        if (_virtualFileList) {
            for (auto& virtualFile : *_virtualFileList) {
                if (virtualFile.first == fileName) {
                    return new FileInfo{
                            .path = virtualFile.first,
                            .contents = virtualFile.second,
                            .isVirtual = true
                    };
                }
            }
        }

        size_t last_slash = requestingPath.find_last_of("/\\");
        std::string dir_name = requestingPath;
        if (last_slash != std::string::npos) {
            dir_name = requestingPath.substr(0, last_slash);
        }
        if (dir_name.size() == requestingPath.size()) {
            dir_name = {};
        }

        std::string relative_filename = dir_name + ((dir_name.empty() || dir_name.back() == '/') ? "" : "/") + fileName;

        if (std::filesystem::exists(relative_filename)) {
            auto file = ende::fs::File::open(relative_filename, ende::fs::in);
            return new FileInfo{
                    .path = relative_filename,
                    .contents = file->read(),
                    .isVirtual = false
            };
        }
        return findReadableFilepath(fileName);
    }

    void addSearchPath(const std::string& path) {
        _searchPaths.push_back(path);
    }

private:

    std::vector<std::pair<std::string, std::string>>* _virtualFileList;
    std::vector<std::string> _searchPaths;

};

class FileIncluder : public shaderc::CompileOptions::IncluderInterface {
public:

    explicit FileIncluder(const FileFinder* fileFinder)
            : _fileFinder(fileFinder)
    {}

    ~FileIncluder() override = default;

    shaderc_include_result* GetInclude(const char* requestedSource, shaderc_include_type type, const char* requestingSource, size_t includeDepth) override {

        auto file = (type == shaderc_include_type_relative) ? _fileFinder->findRelativeReadableFilepath(requestingSource, requestedSource)
                                                            : _fileFinder->findReadableFilepath(requestedSource);

        if (!file) return new shaderc_include_result{"", 0, "unable to open file", 15};
        auto result = new shaderc_include_result{file->path.data(), file->path.size(), file->contents.data(), file->contents.size(), file};
        return result;
    }

    void ReleaseInclude(shaderc_include_result* result) override {
        delete static_cast<FileInfo*>(result->user_data);
        delete result;
    }

private:

    const FileFinder* _fileFinder = nullptr;

};

std::string macroize(std::string_view str) {
    std::string result = str.data();
    size_t index = 0;
    size_t last = 0;

    while ((index = result.find('\n', index)) != std::string::npos) {
        result.insert(index++, "\\");
        last = ++index;
    }
    return result;
}

auto canta::util::compileGLSLToSpirv(std::string_view name, std::string_view glsl, ShaderStage stage, std::span<Macro> macros) -> std::expected<std::vector<u32>, std::string> {
    shaderc_shader_kind kind = {};
    switch (stage) {
        case ShaderStage::VERTEX:
            kind = shaderc_shader_kind::shaderc_vertex_shader;
            break;
        case ShaderStage::TESS_CONTROL:
            kind = shaderc_shader_kind::shaderc_tess_control_shader;
            break;
        case ShaderStage::TESS_EVAL:
            kind = shaderc_shader_kind::shaderc_tess_evaluation_shader;
            break;
        case ShaderStage::GEOMETRY:
            kind = shaderc_shader_kind::shaderc_geometry_shader;
            break;
        case ShaderStage::FRAGMENT:
            kind = shaderc_shader_kind::shaderc_fragment_shader;
            break;
        case ShaderStage::COMPUTE:
            kind = shaderc_shader_kind::shaderc_compute_shader;
            break;
        case ShaderStage::RAYGEN:
            kind = shaderc_shader_kind::shaderc_raygen_shader;
            break;
        case ShaderStage::ANY_HIT:
            kind = shaderc_shader_kind::shaderc_anyhit_shader;
            break;
        case ShaderStage::CLOSEST_HIT:
            kind = shaderc_shader_kind::shaderc_closesthit_shader;
            break;
        case ShaderStage::MISS:
            kind = shaderc_shader_kind::shaderc_miss_shader;
            break;
        case ShaderStage::INTERSECTION:
            kind = shaderc_shader_kind::shaderc_intersection_shader;
            break;
        case ShaderStage::CALLABLE:
            kind = shaderc_shader_kind::shaderc_callable_shader;
            break;
        case ShaderStage::TASK:
            kind = shaderc_shader_kind::shaderc_task_shader;
            break;
        case ShaderStage::MESH:
            kind = shaderc_shader_kind::shaderc_mesh_shader;
            break;
        default:
            return std::unexpected("invalid shader stage used for compilation");
    }


    shaderc::Compiler compiler = {};
    shaderc::CompileOptions options = {};

    options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_3);
    options.SetTargetSpirv(shaderc_spirv_version_1_6);

    FileFinder finder(nullptr);
    options.SetIncluder(std::make_unique<FileIncluder>(&finder));

    for (auto& macro : macros)
        options.AddMacroDefinition(macro.name, macro.value);

    shaderc::SpvCompilationResult result = compiler.CompileGlslToSpv(glsl.data(), kind, name.data(), options);
    if (result.GetCompilationStatus() != shaderc_compilation_status_success) {
        return std::unexpected(std::format("Failed to compiler shader {}:\nErrors: {}\nWarnings: {}\nMessage: {}", name, result.GetNumErrors(), result.GetNumWarnings(), result.GetErrorMessage()));
    }

    return std::vector<u32>(result.cbegin(), result.cend());
}