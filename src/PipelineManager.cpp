#include <cstring>
#include "Canta/PipelineManager.h"
#include <Ende/util/hash.h>
#include <Canta/util.h>
#include <Ende/filesystem/File.h>
#include <format>
#include <shaderc/shaderc.hpp>
#include <rapidjson/document.h>
#include <Ende/filesystem/File.h>

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
    if (!object.path.empty())
        hash = ende::util::combineHash(hash, std::hash<std::filesystem::path>()(object.path));
    if (!object.spirv.empty()) {
        for (auto& code : object.spirv)
            hash = ende::util::combineHash(hash, static_cast<u64>(code));
    }
    if (!object.glsl.empty())
        hash = ende::util::combineHash(hash, std::hash<std::string_view>()(object.glsl));
    for (auto& macro : object.macros) {
        hash = ende::util::combineHash(hash, std::hash<std::string>()(macro.name));
        hash = ende::util::combineHash(hash, std::hash<std::string>()(macro.value));
    }
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
    auto result = lhs.stage == rhs.stage && lhs.path == rhs.path && lhs.spirv.size() == rhs.spirv.size() &&
            lhs.glsl.size() == rhs.glsl.size() && lhs.macros.size() == rhs.macros.size() &&
            memcmp(reinterpret_cast<const void*>(lhs.spirv.data()), reinterpret_cast<const void*>(rhs.spirv.data()), sizeof(u32) * lhs.spirv.size()) == 0 &&
            memcmp(reinterpret_cast<const void*>(lhs.glsl.data()), reinterpret_cast<const void*>(rhs.glsl.data()), sizeof(u8) * lhs.glsl.size()) == 0;
    if (!result)
        return result;
    for (u32 i = 0; i < lhs.macros.size(); i++) {
        result = result && lhs.macros[i].name == rhs.macros[i].name && lhs.macros[i].value == rhs.macros[i].value;
    }
    return result;
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

canta::PipelineManager::PipelineManager(canta::PipelineManager &&rhs) noexcept {
    std::swap(_device, rhs._device);
    std::swap(_rootPath, rhs._rootPath);
    std::swap(_shaders, rhs._shaders);
    std::swap(_pipelines, rhs._pipelines);
    std::swap(_fileWatcher, rhs._fileWatcher);
    std::swap(_watchedPipelines, rhs._watchedPipelines);
    std::swap(_virtualFiles, rhs._virtualFiles);
}

auto canta::PipelineManager::operator=(canta::PipelineManager &&rhs) noexcept -> PipelineManager & {
    std::swap(_device, rhs._device);
    std::swap(_rootPath, rhs._rootPath);
    std::swap(_shaders, rhs._shaders);
    std::swap(_pipelines, rhs._pipelines);
    std::swap(_fileWatcher, rhs._fileWatcher);
    std::swap(_watchedPipelines, rhs._watchedPipelines);
    std::swap(_virtualFiles, rhs._virtualFiles);
    return *this;
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

auto loadFromFile(canta::PipelineManager& manager, const std::filesystem::path &path, std::span<const canta::Macro> additionalMacros = {}) -> canta::Pipeline::CreateInfo;
auto canta::PipelineManager::getPipeline(const std::filesystem::path &path, std::span<const Macro> additionalMacros) -> PipelineHandle {
    auto createInfo = loadFromFile(*this, path, additionalMacros);
    return getPipeline(createInfo);
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
        auto compilationResult = compileGLSL(info.path.string(), glsl, info.stage, info.macros).transform_error([](const auto& error) {
            std::printf("%s", error.c_str());
            return error;
        });
        if (!compilationResult)
            return {};
        spirv = compilationResult.value();
        if (!handle) {
            auto watcher = _watchedPipelines.find(info.path);
            if (watcher == _watchedPipelines.end())
                _fileWatcher.addWatch(_rootPath / info.path);
        }
    } else if (!info.glsl.empty()) {
        auto compilationResult = compileGLSL(info.path.string(), info.glsl, info.stage).transform_error([](const auto& error) {
            std::printf("%s", error.c_str());
            return error;
        });
        if (!compilationResult)
            return {};
        spirv = compilationResult.value();
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

auto canta::PipelineManager::compileGLSL(std::string_view name, std::string_view glsl, canta::ShaderStage stage, std::span<const Macro> macros) -> std::expected<std::vector<u32>, std::string> {
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

    shaderc::SpvCompilationResult result = compiler.CompileGlslToSpv(glsl.data(), kind, name.data(), options);
    if (result.GetCompilationStatus() != shaderc_compilation_status_success) {
        return std::unexpected(std::format("Failed to compiler shader :\nErrors: {}\nWarnings: {}\nMessage: {}", result.GetNumErrors(), result.GetNumWarnings(), result.GetErrorMessage()));
    }

    return std::vector<u32>(result.cbegin(), result.cend());
}


auto loadShaderDescription(canta::PipelineManager& manager, rapidjson::Value& node, std::span<const canta::Macro> additionalMacros = {}) -> canta::ShaderDescription {
    canta::ShaderDescription description = {};
    if (node.HasMember("path")) {
        assert(node["path"].IsString());
        description.path = node["path"].GetString();
    }
//    if (node.HasMember("spirv")) {
//        assert(node["path"].Is)
//    }
    if (node.HasMember("glsl")) {
        assert(node["glsl"].IsString());
        description.glsl = node["glsl"].GetString();
    }
    if (node.HasMember("macros")) {
        assert(node["macros"].IsArray());
        for (auto i = 0; i < node["macros"].Size(); i++) {
            canta::Macro macro = {};
            macro.name = node["macros"][i]["name"].GetString();
            macro.value = node["macros"][i]["value"].GetString();
        }
    }
    description.macros.insert(description.macros.end(), additionalMacros.begin(), additionalMacros.end());
    if (node.HasMember("stage")) {
        assert(node["stage"].IsString());
        std::string stage = node["stage"].GetString();
        if (stage == "NONE")
            description.stage = canta::ShaderStage::NONE;
        else if (stage == "VERTEX")
            description.stage = canta::ShaderStage::VERTEX;
        else if (stage == "TESS_CONTROL")
            description.stage = canta::ShaderStage::TESS_CONTROL;
        else if (stage == "TESS_EVAL")
            description.stage = canta::ShaderStage::TESS_EVAL;
        else if (stage == "GEOMETRY")
            description.stage = canta::ShaderStage::GEOMETRY;
        else if (stage == "FRAGMENT")
            description.stage = canta::ShaderStage::FRAGMENT;
        else if (stage == "COMPUTE")
            description.stage = canta::ShaderStage::COMPUTE;
        else if (stage == "ALL_GRAPHICS")
            description.stage = canta::ShaderStage::ALL_GRAPHICS;
        else if (stage == "ALL")
            description.stage = canta::ShaderStage::ALL;
        else if (stage == "RAYGEN")
            description.stage = canta::ShaderStage::RAYGEN;
        else if (stage == "ANY_HIT")
            description.stage = canta::ShaderStage::ANY_HIT;
        else if (stage == "CLOSEST_HIT")
            description.stage = canta::ShaderStage::CLOSEST_HIT;
        else if (stage == "MISS")
            description.stage = canta::ShaderStage::MISS;
        else if (stage == "INTERSECTION")
            description.stage = canta::ShaderStage::INTERSECTION;
        else if (stage == "CALLABLE")
            description.stage = canta::ShaderStage::CALLABLE;
        else if (stage == "TASK")
            description.stage = canta::ShaderStage::TASK;
        else if (stage == "MESH")
            description.stage = canta::ShaderStage::MESH;
    }

    return description;
}

auto loadRasterState(canta::PipelineManager& manager, rapidjson::Value& node) -> canta::RasterState {
    canta::RasterState rasterState = {};
    if (node.HasMember("cullMode")) {
        auto& cullMode = node["cullMode"];
        assert(cullMode.IsString());
        std::string mode = cullMode.GetString();
        if (mode == "NONE")
            rasterState.cullMode = canta::CullMode::NONE;
        else if (mode == "FRONT")
            rasterState.cullMode = canta::CullMode::FRONT;
        else if (mode == "BACK")
            rasterState.cullMode = canta::CullMode::BACK;
        else if (mode == "FRONT_BACK")
            rasterState.cullMode = canta::CullMode::FRONT_BACK;
    }
    if (node.HasMember("frontFace")) {
        auto& frontFace = node["frontFace"];
        assert(frontFace.IsString());
        std::string face = frontFace.GetString();
        if (face == "CCW")
            rasterState.frontFace = canta::FrontFace::CCW;
        else if (face == "CW")
            rasterState.frontFace = canta::FrontFace::CW;
    }
    if (node.HasMember("polygonMode")) {
        auto& polygonMode = node["polygonMode"];
        assert(polygonMode.IsString());
        std::string mode = polygonMode.GetString();
        if (mode == "FILL")
            rasterState.polygonMode = canta::PolygonMode::FILL;
        else if (mode == "LINE")
            rasterState.polygonMode = canta::PolygonMode::LINE;
        else if (mode == "POINT")
            rasterState.polygonMode = canta::PolygonMode::POINT;
    }
    if (node.HasMember("lineWidth")) {
        auto& lineWidth = node["lineWidth"];
        assert(lineWidth.IsFloat());
        rasterState.lineWidth = lineWidth.GetFloat();
    }
    if (node.HasMember("depthClamp")) {
        auto& depthClamp = node["depthClamp"];
        assert(depthClamp.IsBool());
        rasterState.depthClamp = depthClamp.GetBool();
    }
    if (node.HasMember("rasterDiscard")) {
        auto& rasterDiscard = node["rasterDiscard"];
        assert(rasterDiscard.IsBool());
        rasterState.rasterDiscard = rasterDiscard.GetBool();
    }
    if (node.HasMember("depthBias")) {
        auto& depthBias = node["depthBias"];
        assert(depthBias.IsBool());
        rasterState.depthBias = depthBias.GetBool();
    }
    return rasterState;
}

auto loadDepthState(canta::PipelineManager& manager, rapidjson::Value& node) -> canta::DepthState {
    canta::DepthState depthState = {};
    if (node.HasMember("test")) {
        auto& test = node["test"];
        assert(test.IsBool());
        depthState.test = test.GetBool();
    }
    if (node.HasMember("write")) {
        auto& write = node["write"];
        assert(write.IsBool());
        depthState.write = write.GetBool();
    }
    if (node.HasMember("compareOp")) {
        auto& compareOp = node["compareOp"];
        assert(compareOp.IsString());
        std::string op = compareOp.GetString();
        if (op == "NEVER")
            depthState.compareOp = canta::CompareOp::NEVER;
        else if (op == "LESS")
            depthState.compareOp = canta::CompareOp::LESS;
        else if (op == "EQUAL")
            depthState.compareOp = canta::CompareOp::EQUAL;
        else if (op == "LEQUAL")
            depthState.compareOp = canta::CompareOp::LEQUAL;
        else if (op == "GREATER")
            depthState.compareOp = canta::CompareOp::GREATER;
        else if (op == "NEQUAL")
            depthState.compareOp = canta::CompareOp::NEQUAL;
        else if (op == "GEQUAL")
            depthState.compareOp = canta::CompareOp::GEQUAL;
        else if (op == "ALWAYS")
            depthState.compareOp = canta::CompareOp::ALWAYS;
    }
    return depthState;
}

auto loadBlendState(canta::PipelineManager& manager, rapidjson::Value& node) -> canta::BlendState {
    canta::BlendState blendState = {};
    //TODO: blendstate
    return blendState;
}

auto loadFormat(std::string_view format) -> canta::Format {
#define ELIF_FORMAT(f) else if (format == #f) return canta::Format::f;

    if (format == "UNDEFINED")
        return canta::Format::UNDEFINED;
    ELIF_FORMAT(RGBA8_UNORM)
    ELIF_FORMAT(R32_UINT)
    ELIF_FORMAT(R32_SINT)
    ELIF_FORMAT(RGBA32_SFLOAT)
    ELIF_FORMAT(D32_SFLOAT)
    return canta::Format::UNDEFINED;
}

auto loadFromFile(canta::PipelineManager& manager, const std::filesystem::path &path, std::span<const canta::Macro> additionalMacros) -> canta::Pipeline::CreateInfo {
    auto file = ende::fs::File::open(path);

    rapidjson::Document document;
    document.Parse(file->read().c_str());
    assert(document.IsObject());

    canta::Pipeline::CreateInfo createInfo = {};

    if (document.HasMember("vertex")) {
        rapidjson::Value& vertexShader = document["vertex"];
        assert(vertexShader.IsObject());
        createInfo.vertex = {
            .module = manager.getShader(loadShaderDescription(manager, vertexShader, additionalMacros))
        };
    }
    if (document.HasMember("tesselationControl")) {
        rapidjson::Value& tesselationControl = document["tesselationControl"];
        assert(tesselationControl.IsObject());
        createInfo.tesselationControl = {
            .module = manager.getShader(loadShaderDescription(manager, tesselationControl, additionalMacros))
        };
    }
    if (document.HasMember("tesselationEvaluation")) {
        rapidjson::Value& tesselationEvaluation = document["tesselationEvaluation"];
        assert(tesselationEvaluation.IsObject());
        createInfo.tesselationEvaluation = {
            .module = manager.getShader(loadShaderDescription(manager, tesselationEvaluation, additionalMacros))
        };
    }
    if (document.HasMember("geometry")) {
        rapidjson::Value& geometry = document["geometry"];
        assert(geometry.IsObject());
        createInfo.geometry = {
            .module = manager.getShader(loadShaderDescription(manager, geometry, additionalMacros))
        };
    }
    if (document.HasMember("fragment")) {
        rapidjson::Value& fragment = document["fragment"];
        assert(fragment.IsObject());
        createInfo.fragment = {
            .module = manager.getShader(loadShaderDescription(manager, fragment, additionalMacros))
        };
    }
    if (document.HasMember("compute")) {
        rapidjson::Value& compute = document["compute"];
        assert(compute.IsObject());
        createInfo.compute = {
            .module = manager.getShader(loadShaderDescription(manager, compute, additionalMacros))
        };
    }
    if (document.HasMember("rayGen")) {
        rapidjson::Value& rayGen = document["rayGen"];
        assert(rayGen.IsObject());
        createInfo.rayGen = {
            .module = manager.getShader(loadShaderDescription(manager, rayGen, additionalMacros))
        };
    }
    if (document.HasMember("rayGen")) {
        rapidjson::Value& anyHit = document["anyHit"];
        assert(anyHit.IsObject());
        createInfo.anyHit = {
            .module = manager.getShader(loadShaderDescription(manager, anyHit, additionalMacros))
        };
    }
    if (document.HasMember("rayGen")) {
        rapidjson::Value& closestHit = document["closestHit"];
        assert(closestHit.IsObject());
        createInfo.closestHit = {
            .module = manager.getShader(loadShaderDescription(manager, closestHit, additionalMacros))
        };
    }
    if (document.HasMember("rayGen")) {
        rapidjson::Value& miss = document["miss"];
        assert(miss.IsObject());
        createInfo.miss = {
            .module = manager.getShader(loadShaderDescription(manager, miss, additionalMacros))
        };
    }
    if (document.HasMember("rayGen")) {
        rapidjson::Value& intersection = document["intersection"];
        assert(intersection.IsObject());
        createInfo.intersection = {
            .module = manager.getShader(loadShaderDescription(manager, intersection, additionalMacros))
        };
    }
    if (document.HasMember("rayGen")) {
        rapidjson::Value& callable = document["callable"];
        assert(callable.IsObject());
        createInfo.callable = {
            .module = manager.getShader(loadShaderDescription(manager, callable, additionalMacros))
        };
    }
    if (document.HasMember("rayGen")) {
        rapidjson::Value& task = document["task"];
        assert(task.IsObject());
        createInfo.task = {
            .module = manager.getShader(loadShaderDescription(manager, task, additionalMacros))
        };
    }
    if (document.HasMember("rayGen")) {
        rapidjson::Value& mesh = document["mesh"];
        assert(mesh.IsObject());
        createInfo.mesh = {
            .module = manager.getShader(loadShaderDescription(manager, mesh, additionalMacros))
        };
    }
    if (document.HasMember("rasterState")) {
        rapidjson::Value& rasterState = document["rasterState"];
        assert(rasterState.IsObject());
        createInfo.rasterState = loadRasterState(manager, rasterState);
    }
    if (document.HasMember("depthState")) {
        rapidjson::Value& depthState = document["depthState"];
        assert(depthState.IsObject());
        createInfo.depthState = loadDepthState(manager, depthState);
    }
    if (document.HasMember("blendState")) {
        rapidjson::Value& blendState = document["blendState"];
        assert(blendState.IsObject());
        createInfo.blendState = loadBlendState(manager, blendState);
    }
    //TODO: bindings
//    rapidjson::Value& inputBindings = document["inputBindings"];
//    rapidjson::Value& inputAttributes = document["inputAttributes"];
//    rapidjson::Value& topology = document["topology"];
    if (document.HasMember("primitiveRestart")) {
        rapidjson::Value& primitiveRestart = document["primitiveRestart"];
        assert(primitiveRestart.IsBool());
        createInfo.primitiveRestart = primitiveRestart.GetBool();
    }
    if (document.HasMember("colourFormats")) {
        rapidjson::Value& colourFormats = document["colourFormats"];
        assert(colourFormats.IsArray());
        for (auto i = 0; i < colourFormats.Size(); i++) {
            assert(colourFormats[i].IsString());
            createInfo.colourFormats.push_back(loadFormat(colourFormats[i].GetString()));
        }
    }
    if (document.HasMember("depthFormat")) {
        rapidjson::Value& depthFormat = document["depthFormat"];
        assert(depthFormat.IsString());
        createInfo.depthFormat = loadFormat(depthFormat.GetString());
    }

    return createInfo;
}