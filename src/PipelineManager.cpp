#include <cstring>
#include "Canta/PipelineManager.h"
#include <Ende/util/hash.h>
#include <Canta/util.h>
#include <Ende/filesystem/File.h>
#include <format>
#include <shaderc/shaderc.hpp>

size_t std::hash<canta::Pipeline::CreateInfo>::operator()(const canta::Pipeline::CreateInfo &object) const {
    u64 hash = 0;
    if (object.vertex.module) hash = ende::util::combineHash(hash, (u64)object.vertex.module.hash());
    if (object.tesselationControl.module) hash = ende::util::combineHash(hash, (u64)object.tesselationControl.module.hash());
    if (object.tesselationEvaluation.module) hash = ende::util::combineHash(hash, (u64)object.tesselationEvaluation.module.hash());
    if (object.geometry.module) hash = ende::util::combineHash(hash, (u64)object.geometry.module.hash());
    if (object.fragment.module) hash = ende::util::combineHash(hash, (u64)object.fragment.module.hash());
    if (object.compute.module) hash = ende::util::combineHash(hash, (u64)object.compute.module.hash());
    if (object.rayGen.module) hash = ende::util::combineHash(hash, (u64)object.rayGen.module.hash());
    if (object.anyHit.module) hash = ende::util::combineHash(hash, (u64)object.anyHit.module.hash());
    if (object.closestHit.module) hash = ende::util::combineHash(hash, (u64)object.closestHit.module.hash());
    if (object.miss.module) hash = ende::util::combineHash(hash, (u64)object.miss.module.hash());
    if (object.intersection.module) hash = ende::util::combineHash(hash, (u64)object.intersection.module.hash());
    if (object.callable.module) hash = ende::util::combineHash(hash, (u64)object.callable.module.hash());
    if (object.task.module) hash = ende::util::combineHash(hash, (u64)object.task.module.hash());
    if (object.mesh.module) hash = ende::util::combineHash(hash, (u64)object.mesh.module.hash());

    hash = ende::util::combineHash(hash, (u64)ende::util::MurmurHash<canta::RasterState>()(object.rasterState));
    hash = ende::util::combineHash(hash, (u64)ende::util::MurmurHash<canta::DepthState>()(object.depthState));
    hash = ende::util::combineHash(hash, (u64)ende::util::MurmurHash<canta::BlendState>()(object.blendState));
    for (auto& binding : object.inputBindings)
        hash = ende::util::combineHash(hash, (u64)ende::util::MurmurHash<canta::VertexInputBinding>()(binding));
    for (auto& attribute : object.inputAttributes)
        hash = ende::util::combineHash(hash, (u64)ende::util::MurmurHash<canta::VertexInputAttribute>()(attribute));
    for (auto& format : object.colourFormats)
        hash = ende::util::combineHash(hash, (u64)format);
    hash = ende::util::combineHash(hash, (u64)object.topology);
    hash = ende::util::combineHash(hash, (u64)object.primitiveRestart);
    hash = ende::util::combineHash(hash, (u64)object.depthFormat);
    return hash;
}

size_t std::hash<canta::ShaderDescription>::operator()(const canta::ShaderDescription &object) const {
    u64 hash = 0;
    hash = ende::util::combineHash(hash, std::hash<std::filesystem::path>()(object.path));
    hash = ende::util::combineHash(hash, (u64)object.stage);
    return hash;
}

bool canta::operator==(const Pipeline::CreateInfo &lhs, const Pipeline::CreateInfo &rhs) {
    return lhs.vertex.module == rhs.vertex.module && lhs.vertex.entryPoint == rhs.vertex.entryPoint &&
        lhs.tesselationControl.module == rhs.tesselationControl.module && lhs.tesselationControl.entryPoint == rhs.tesselationControl.entryPoint &&
        lhs.tesselationEvaluation.module == rhs.tesselationEvaluation.module && lhs.tesselationEvaluation.entryPoint == rhs.tesselationEvaluation.entryPoint &&
        lhs.geometry.module == rhs.geometry.module && lhs.geometry.entryPoint == rhs.geometry.entryPoint &&
        lhs.fragment.module == rhs.fragment.module && lhs.fragment.entryPoint == rhs.fragment.entryPoint &&
        lhs.compute.module == rhs.compute.module && lhs.compute.entryPoint == rhs.compute.entryPoint &&
        lhs.rayGen.module == rhs.rayGen.module && lhs.rayGen.entryPoint == rhs.rayGen.entryPoint &&
        lhs.anyHit.module == rhs.anyHit.module && lhs.anyHit.entryPoint == rhs.anyHit.entryPoint &&
        lhs.closestHit.module == rhs.closestHit.module && lhs.closestHit.entryPoint == rhs.closestHit.entryPoint &&
        lhs.miss.module == rhs.miss.module && lhs.miss.entryPoint == rhs.miss.entryPoint &&
        lhs.intersection.module == rhs.intersection.module && lhs.intersection.entryPoint == rhs.intersection.entryPoint &&
        lhs.callable.module == rhs.callable.module && lhs.callable.entryPoint == rhs.callable.entryPoint &&
        lhs.task.module == rhs.task.module && lhs.task.entryPoint == rhs.task.entryPoint &&
        lhs.mesh.module == rhs.mesh.module && lhs.mesh.entryPoint == rhs.mesh.entryPoint &&
        memcmp(reinterpret_cast<const void*>(&lhs.rasterState), reinterpret_cast<const void*>(&rhs.rasterState), sizeof(lhs.rasterState)) == 0 &&
        memcmp(reinterpret_cast<const void*>(&lhs.depthState), reinterpret_cast<const void*>(&rhs.depthState), sizeof(lhs.depthState)) == 0 &&
        memcmp(reinterpret_cast<const void*>(&lhs.blendState), reinterpret_cast<const void*>(&rhs.blendState), sizeof(lhs.blendState)) == 0 &&
        lhs.primitiveRestart == rhs.primitiveRestart && lhs.depthFormat == rhs.depthFormat &&
        lhs.inputBindings.size() == rhs.inputBindings.size() && lhs.inputAttributes.size() == rhs.inputAttributes.size() &&
        lhs.colourFormats.size() == rhs.colourFormats.size() &&
        memcmp(reinterpret_cast<const void*>(lhs.inputBindings.data()), reinterpret_cast<const void*>(rhs.inputBindings.data()), sizeof(VertexInputBinding) * lhs.inputBindings.size()) == 0 &&
        memcmp(reinterpret_cast<const void*>(lhs.inputAttributes.data()), reinterpret_cast<const void*>(rhs.inputAttributes.data()), sizeof(VertexInputAttribute) * lhs.inputAttributes.size()) == 0 &&
        memcmp(reinterpret_cast<const void*>(lhs.colourFormats.data()), reinterpret_cast<const void*>(rhs.colourFormats.data()), sizeof(Format) * lhs.colourFormats.size()) == 0;
}

bool canta::operator==(const ShaderDescription &lhs, const ShaderDescription &rhs) {
    return lhs.stage == rhs.stage && lhs.path == rhs.path && lhs.spirv.size() == rhs.spirv.size() &&
            memcmp(reinterpret_cast<const void*>(lhs.spirv.data()), reinterpret_cast<const void*>(rhs.spirv.data()), sizeof(u32) * lhs.spirv.size()) == 0;
}

auto canta::PipelineManager::create(canta::PipelineManager::CreateInfo info) -> PipelineManager {
    PipelineManager manager = {};
    manager._device = info.device;
    manager._rootPath = info.rootPath;
    const char* cantaFile =
#include <Canta/canta.glsl>
    ;
    manager.addVirtualFile("canta.glsl", cantaFile);
    return manager;
}

auto canta::PipelineManager::getShader(canta::ShaderDescription info) -> ShaderHandle {
    auto it = _shaders.find(info);
    if (it != _shaders.end())
        return it->second;

    return createShader(info);
}

auto canta::PipelineManager::getPipeline(Pipeline::CreateInfo info) -> PipelineHandle {
    auto it = _pipelines.find(info);
    if (it != _pipelines.end())
        return it->second;

    const auto addShaderDependency = [&](ShaderHandle shader, PipelineHandle pipeline) {
        ShaderDescription description = {};
        for (auto& [key, value] : _shaders) {
            if (shader == value) {
                description = key;
                break;
            }
        }
        auto it = _watchedPipelines.find(description.path);
        if (it != _watchedPipelines.end()) {
            it.value().second.push_back(info);
        } else {
            auto it1 = _watchedPipelines.insert(std::make_pair(description.path, std::make_pair(description, std::vector<Pipeline::CreateInfo>())));
            it1.first.value().second.push_back(info);
        }
    };

    auto it1 = _pipelines.insert(std::make_pair(info, _device->createPipeline(info)));


    if (info.vertex.module)
        addShaderDependency(info.vertex.module, it1.first->second);
    if (info.tesselationControl.module)
        addShaderDependency(info.tesselationControl.module, it1.first->second);
    if (info.tesselationEvaluation.module)
        addShaderDependency(info.tesselationEvaluation.module, it1.first->second);
    if (info.geometry.module)
        addShaderDependency(info.geometry.module, it1.first->second);
    if (info.fragment.module)
        addShaderDependency(info.fragment.module, it1.first->second);
    if (info.compute.module)
        addShaderDependency(info.compute.module, it1.first->second);
    if (info.rayGen.module)
        addShaderDependency(info.rayGen.module, it1.first->second);
    if (info.anyHit.module)
        addShaderDependency(info.anyHit.module, it1.first->second);
    if (info.closestHit.module)
        addShaderDependency(info.closestHit.module, it1.first->second);
    if (info.miss.module)
        addShaderDependency(info.miss.module, it1.first->second);
    if (info.intersection.module)
        addShaderDependency(info.intersection.module, it1.first->second);
    if (info.callable.module)
        addShaderDependency(info.callable.module, it1.first->second);
    if (info.task.module)
        addShaderDependency(info.task.module, it1.first->second);
    if (info.mesh.module)
        addShaderDependency(info.mesh.module, it1.first->second);

    return it1.first->second;
}

auto canta::PipelineManager::reload(canta::ShaderHandle shader) -> ShaderHandle {
    for (auto& [key, value] : _shaders) {
        if (value == shader) {
            return reload(key);
        }
    }
    return {};
}

auto canta::PipelineManager::reload(canta::PipelineHandle pipeline) -> PipelineHandle {
    for (auto& [key, value]: _pipelines) {
        if (value == pipeline) {
            return _device->createPipeline(key, pipeline);
        }
    }
    return {};
}

void canta::PipelineManager::reloadAll(bool force) {
    auto events = _fileWatcher.read();
    for (auto& event : events) {
        auto it = _watchedPipelines.find(event.path);
        if (it != _watchedPipelines.end()) {
            reload(it->second.first);
            for (auto& pipeline : it->second.second)
                reload(pipeline);
        }
    }

    if (force) {
        for (auto& shader : _shaders)
            reload(shader.second);
        for (auto& pipeline : _pipelines)
            reload(pipeline.second);
    }
}

auto canta::PipelineManager::reload(canta::ShaderDescription description) -> ShaderHandle {
    auto it = _shaders.find(description);
    ShaderHandle handle = {};
    if (it != _shaders.end())
        handle = it->second;
    return createShader(description, handle);
}

auto canta::PipelineManager::reload(Pipeline::CreateInfo info) -> PipelineHandle {
    auto it = _pipelines.find(info);
    PipelineHandle handle = {};
    if (it != _pipelines.end())
        handle = it->second;
    return _device->createPipeline(info, handle);
}

auto canta::PipelineManager::createShader(canta::ShaderDescription info, ShaderHandle handle) -> ShaderHandle {
    ShaderModule::CreateInfo createInfo = {};
    createInfo.spirv = info.spirv;
    createInfo.stage = info.stage;
    if (!_device->meshShadersEnabled() && info.stage == ShaderStage::MESH) //TODO: log
        return {};
    if (!_device->taskShadersEnabled() && info.stage == ShaderStage::TASK)
        return {};

    std::vector<u32> spirv = {};
    if (!info.path.empty()) {
        auto shaderFile = ende::fs::File::open(_rootPath / info.path);
        auto glsl = shaderFile->read();
        spirv = compileGLSL(glsl, info.stage).transform_error([](const auto& error) {
            std::printf("%s", error.c_str());
            return error;
        }).value();
        if (!handle) {
            auto watcher = _watchedPipelines.find(info.path);
            if (watcher == _watchedPipelines.end())
                _fileWatcher.addWatch(_rootPath / info.path);
        }
    } else if (!info.glsl.empty()) {
        spirv = compileGLSL(info.glsl, info.stage).transform_error([](const auto& error) {
            std::printf("%s", error.c_str());
            return error;
        }).value();
    }
    if (!spirv.empty())
        createInfo.spirv = spirv;


    auto h = _device->createShaderModule(createInfo, handle);
    if (!handle) {
        auto it1 = _shaders.insert(std::make_pair(info, h));
        return it1.first->second;
    }
    return h;
}

void canta::PipelineManager::addVirtualFile(const std::filesystem::path &path, const std::string& contents) {
    _virtualFiles.push_back({ path.string(), contents });
}


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

auto canta::PipelineManager::compileGLSL(std::string_view glsl, canta::ShaderStage stage, std::span<Macro> macros) -> std::expected<std::vector<u32>, std::string> {
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

    FileFinder finder(&_virtualFiles);
    finder.addSearchPath(_rootPath);
    options.SetIncluder(std::make_unique<FileIncluder>(&finder));

    for (auto& macro : macros)
        options.AddMacroDefinition(macro.name, macro.value);

    shaderc::SpvCompilationResult result = compiler.CompileGlslToSpv(glsl.data(), kind, "canta_shader", options);
    if (result.GetCompilationStatus() != shaderc_compilation_status_success) {
        return std::unexpected(std::format("Failed to compiler shader :\nErrors: {}\nWarnings: {}\nMessage: {}", result.GetNumErrors(), result.GetNumWarnings(), result.GetErrorMessage()));
    }

    return std::vector<u32>(result.cbegin(), result.cend());
}