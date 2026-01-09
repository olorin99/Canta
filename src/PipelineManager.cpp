#include <cstring>
#include "Canta/PipelineManager.h"
#include <Ende/util/hash.h>
#include <Ende/filesystem/File.h>
#include <format>
#include <rapidjson/document.h>
#include "embeded_shaders_Canta.h"

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
    if (!object.slang.empty())
        hash = ende::util::combineHash(hash, std::hash<std::string_view>()(object.slang));
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
            lhs.slang.size() == rhs.slang.size() && lhs.macros.size() == rhs.macros.size() &&
            memcmp(reinterpret_cast<const void*>(lhs.spirv.data()), reinterpret_cast<const void*>(rhs.spirv.data()), sizeof(u32) * lhs.spirv.size()) == 0;
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
    manager._rowMajor = info.rowMajor;

    registerEmbededShadersCanta(manager);

    slang::createGlobalSession(manager._slangGlobalSession.writeRef());

    return manager;
}

canta::PipelineManager::PipelineManager(canta::PipelineManager &&rhs) noexcept {
    std::swap(_device, rhs._device);
    std::swap(_rootPath, rhs._rootPath);
    std::swap(_rowMajor, rhs._rowMajor);
    std::swap(_shaders, rhs._shaders);
    std::swap(_pipelines, rhs._pipelines);
    std::swap(_fileWatcher, rhs._fileWatcher);
    std::swap(_watchedPipelines, rhs._watchedPipelines);
    std::swap(_virtualFiles, rhs._virtualFiles);
    std::swap(_slangGlobalSession, rhs._slangGlobalSession);
}

auto canta::PipelineManager::operator=(canta::PipelineManager &&rhs) noexcept -> PipelineManager & {
    std::swap(_device, rhs._device);
    std::swap(_rootPath, rhs._rootPath);
    std::swap(_rowMajor, rhs._rowMajor);
    std::swap(_shaders, rhs._shaders);
    std::swap(_pipelines, rhs._pipelines);
    std::swap(_fileWatcher, rhs._fileWatcher);
    std::swap(_watchedPipelines, rhs._watchedPipelines);
    std::swap(_virtualFiles, rhs._virtualFiles);
    std::swap(_slangGlobalSession, rhs._slangGlobalSession);
    return *this;
}

auto canta::PipelineManager::getShader(canta::ShaderDescription info) -> std::expected<ShaderHandle, Error> {
    std::string name;
    if (info.name.empty() && !info.path.empty()) {
        name = info.path.filename().string();
        info.name = name;
    }

    if (const auto it = _shaders.find(info); it != _shaders.end())
        return it->second;

    return createShader(info);
}

auto canta::PipelineManager::getPipeline(Pipeline::CreateInfo info) -> std::expected<PipelineHandle, Error> {
    if (const auto it = _pipelines.find(info); it != _pipelines.end())
        return it->second;

    const auto evalShader = [&](ShaderStage stage, ShaderInfo shaderInfo) -> ShaderInfo {
        const auto shader = shaderInfo.module ? shaderInfo.module : getShader({
            .path = shaderInfo.path,
            .stage = stage
        }).value();
        shaderInfo.module = shader;
        return shaderInfo;
    };

    if (info.vertex)
        info.vertex = evalShader(ShaderStage::VERTEX, info.vertex);
    if (info.tesselationControl)
        info.tesselationControl = evalShader(ShaderStage::TESS_CONTROL, info.tesselationControl);
    if (info.tesselationEvaluation)
        info.tesselationEvaluation = evalShader(ShaderStage::TESS_EVAL, info.tesselationEvaluation);
    if (info.geometry)
        info.geometry = evalShader(ShaderStage::GEOMETRY, info.geometry);
    if (info.fragment)
        info.fragment = evalShader(ShaderStage::FRAGMENT, info.fragment);
    if (info.compute)
        info.compute = evalShader(ShaderStage::COMPUTE, info.compute);
    if (info.rayGen)
        info.rayGen = evalShader(ShaderStage::RAYGEN, info.rayGen);
    if (info.anyHit)
        info.anyHit = evalShader(ShaderStage::ANY_HIT, info.anyHit);
    if (info.closestHit)
        info.closestHit = evalShader(ShaderStage::CLOSEST_HIT, info.closestHit);
    if (info.miss)
        info.miss = evalShader(ShaderStage::MISS, info.miss);
    if (info.intersection)
        info.intersection = evalShader(ShaderStage::INTERSECTION, info.intersection);
    if (info.callable)
        info.callable = evalShader(ShaderStage::CALLABLE, info.callable);
    if (info.task)
        info.task = evalShader(ShaderStage::TASK, info.task);
    if (info.mesh)
        info.mesh = evalShader(ShaderStage::MESH, info.mesh);

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

    auto handle = _device->createPipeline(info);
    if (!handle) return std::unexpected(Error::InvalidPipeline);
    auto it1 = _pipelines.insert(std::make_pair(info, handle));

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

auto canta::PipelineManager::getPipeline(const canta::Pipeline &old, Pipeline::CreateInfo overrideInfo) -> std::expected<PipelineHandle, Error> {
    Pipeline::CreateInfo info = {
            .vertex = (old.info().vertex.module == ShaderHandle() && old.info().vertex.entryPoint == "main") ? overrideInfo.vertex : old.info().vertex,
            .tesselationControl = (old.info().tesselationControl.module == ShaderHandle() && old.info().tesselationControl.entryPoint == "main") ? overrideInfo.tesselationControl : old.info().tesselationControl,
            .tesselationEvaluation = (old.info().tesselationEvaluation.module == ShaderHandle() && old.info().tesselationEvaluation.entryPoint == "main") ? overrideInfo.tesselationEvaluation : old.info().tesselationEvaluation,
            .geometry = (old.info().geometry.module == ShaderHandle() && old.info().geometry.entryPoint == "main") ? overrideInfo.geometry : old.info().geometry,
            .fragment = (old.info().fragment.module == ShaderHandle() && old.info().fragment.entryPoint == "main") ? overrideInfo.fragment : old.info().fragment,
            .compute = (old.info().compute.module == ShaderHandle() && old.info().compute.entryPoint == "main") ? overrideInfo.compute : old.info().compute,
            .rayGen = (old.info().rayGen.module == ShaderHandle() && old.info().rayGen.entryPoint == "main") ? overrideInfo.rayGen : old.info().rayGen,
            .anyHit = (old.info().anyHit.module == ShaderHandle() && old.info().anyHit.entryPoint == "main") ? overrideInfo.anyHit : old.info().anyHit,
            .closestHit = (old.info().closestHit.module == ShaderHandle() && old.info().closestHit.entryPoint == "main") ? overrideInfo.closestHit : old.info().closestHit,
            .miss = (old.info().miss.module == ShaderHandle() && old.info().miss.entryPoint == "main") ? overrideInfo.miss : old.info().miss,
            .intersection = (old.info().intersection.module == ShaderHandle() && old.info().intersection.entryPoint == "main") ? overrideInfo.intersection : old.info().intersection,
            .callable = (old.info().callable.module == ShaderHandle() && old.info().callable.entryPoint == "main") ? overrideInfo.callable : old.info().callable,
            .task = (old.info().task.module == ShaderHandle() && old.info().task.entryPoint == "main") ? overrideInfo.task : old.info().task,
            .mesh = (old.info().mesh.module == ShaderHandle() && old.info().mesh.entryPoint == "main") ? overrideInfo.mesh : old.info().mesh,
            .specializationConstants = (overrideInfo.specializationConstants.empty()) ? old.info().specializationConstants : overrideInfo.specializationConstants,
            .rasterState = (old.info().rasterState == RasterState{}) ? overrideInfo.rasterState : old.info().rasterState,
            .depthState = (old.info().depthState == DepthState{}) ? overrideInfo.depthState : old.info().depthState,
            .blendState = (old.info().blendState == BlendState{}) ? overrideInfo.blendState : old.info().blendState,
            .inputBindings = (old.info().inputBindings.empty()) ? overrideInfo.inputBindings : old.info().inputBindings,
            .inputAttributes = (old.info().inputAttributes.empty()) ? overrideInfo.inputAttributes : old.info().inputAttributes,
            .topology = (old.info().topology == PrimitiveTopology::TRIANGLE_LIST) ? overrideInfo.topology : old.info().topology,
            .primitiveRestart = (old.info().primitiveRestart == false) ? overrideInfo.primitiveRestart : old.info().primitiveRestart,
            .colourFormats = (old.info().colourFormats.empty()) ? overrideInfo.colourFormats : old.info().colourFormats,
            .depthFormat = (old.info().depthFormat == Format::UNDEFINED) ? overrideInfo.depthFormat : old.info().depthFormat,
            .name = overrideInfo.name
    };

    return getPipeline(info);
}

auto loadFromFile(canta::PipelineManager& manager, const std::filesystem::path &path, std::span<const canta::Macro> additionalMacros = {}) -> canta::Pipeline::CreateInfo;
auto canta::PipelineManager::getPipeline(const std::filesystem::path &path, std::span<const Macro> additionalMacros, const std::vector<SpecializationConstant>& specializationConstants) -> std::expected<PipelineHandle, Error> {
    auto createInfo = loadFromFile(*this, path, additionalMacros);
    createInfo.specializationConstants = specializationConstants;
    return getPipeline(createInfo);
}

auto canta::PipelineManager::reload(canta::ShaderHandle shader) -> std::expected<ShaderHandle, Error> {
    for (auto& [key, value] : _shaders) {
        if (value == shader) {
            return reload(key);
        }
    }
    return {};
}

auto canta::PipelineManager::reload(canta::PipelineHandle pipeline) -> std::expected<PipelineHandle, Error> {
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

auto canta::PipelineManager::reload(canta::ShaderDescription description) -> std::expected<ShaderHandle, Error> {
    auto it = _shaders.find(description);
    ShaderHandle handle = {};
    if (it != _shaders.end())
        handle = it->second;
    return createShader(description, handle);
}

auto canta::PipelineManager::reload(Pipeline::CreateInfo info) -> std::expected<PipelineHandle, Error> {
    auto it = _pipelines.find(info);
    PipelineHandle handle = {};
    if (it != _pipelines.end())
        handle = it->second;
    return _device->createPipeline(info, handle);
}

auto canta::PipelineManager::createShader(canta::ShaderDescription info, ShaderHandle handle) -> std::expected<ShaderHandle, Error> {
    ShaderModule::CreateInfo createInfo = {};
    createInfo.spirv = info.spirv;
    createInfo.stage = info.stage;
    createInfo.name = info.name;
    if (createInfo.name.empty() && !info.path.empty())
        createInfo.name = info.path.c_str();
    if (!_device->meshShadersEnabled() && info.stage == ShaderStage::MESH) //TODO: log
        return {};
    if (!_device->taskShadersEnabled() && info.stage == ShaderStage::TASK)
        return {};

    std::vector<u32> spirv = {};
    if (!info.path.empty()) {
        auto virtualFile = findVirtualFile(info.path);
        std::string source;
        if (virtualFile)
            source = virtualFile.value();
        else {
            auto shaderFile = ende::fs::File::open(_rootPath / info.path);
            if (!shaderFile) {
                _device->logger().error("Invalid shader path: {}", (_rootPath / info.path).string());
                return std::unexpected(Error::InvalidPath);
            }
            source = shaderFile->read();
        }
        if (info.path.extension() == ".slang") {
            spirv = TRY(compileSlang(info.path.stem().string(), source, info.stage, info.macros)
                .transform_error([this](const auto& error) {
                    _device->logger().error("Shader VulkanError: {}", error.c_str());
                    return Error::InvalidShader;
                }));
        }
        if (!handle) {
            auto watcher = _watchedPipelines.find(info.path);
            if (watcher == _watchedPipelines.end())
                _fileWatcher.addWatch(_rootPath / info.path);
        }
    } else if (!info.slang.empty()) {
        spirv = TRY(compileSlang(info.name, info.slang, info.stage).transform_error([this](const auto& error) {
            _device->logger().error("Shader VulkanError: {}", error.c_str());
            return Error::InvalidShader;
        }));
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

#define DIAGNOSE(diagnostics) if (diagnostics != nullptr) return std::unexpected(reinterpret_cast<const char*>(diagnostics->getBufferPointer()));

auto canta::PipelineManager::compileSlang(std::string_view name, std::string_view slang, canta::ShaderStage stage, std::span<const Macro> macros) -> std::expected<std::vector<u32>, std::string> {
    auto session = TRY(createSlangSession());

    Slang::ComPtr<slang::IModule> slangModule = {};
    {
        Slang::ComPtr<slang::IBlob> diagnostics = {};
        slangModule = session->loadModuleFromSourceString(name.data(), name.data(), slang.data(), diagnostics.writeRef());
        DIAGNOSE(diagnostics);
    }

    Slang::ComPtr<slang::IComponentType> linkedProgram;
    {
        Slang::ComPtr<slang::IBlob> diagnostics;
        SlangResult result = slangModule->link(linkedProgram.writeRef(), diagnostics.writeRef());
        DIAGNOSE(diagnostics);
    }

    Slang::ComPtr<slang::IBlob> kernelBlob;
    {
        Slang::ComPtr<slang::IBlob> diagnostics;
        SlangResult result = linkedProgram->getTargetCode(0, kernelBlob.writeRef(), diagnostics.writeRef());
        DIAGNOSE(diagnostics);
    }

    std::vector<u32> spirv = {};
    spirv.insert(spirv.begin(), (u32*)kernelBlob->getBufferPointer(), (u32*)((u8*)kernelBlob->getBufferPointer() + kernelBlob->getBufferSize()));
    return spirv;
}

auto canta::PipelineManager::createSlangSession(std::span<const Macro> macros) -> std::expected<Slang::ComPtr<slang::ISession>, std::string> {
    if (macros.empty() && _slangMainSession) {
        return _slangMainSession;
    }

    slang::SessionDesc sessionDesc = {};
    const auto rootPath = _rootPath.string();
    const char* searchPaths[] = { rootPath.c_str() };
    sessionDesc.searchPaths = searchPaths;
    sessionDesc.searchPathCount = 1;
    std::vector<slang::PreprocessorMacroDesc> slangMacros = {};
    for (auto& macro : macros) {
        slang::PreprocessorMacroDesc macroDesc = {};
        macroDesc.name = macro.name.c_str();
        macroDesc.value = macro.value.c_str();
        slangMacros.push_back(macroDesc);
    }
    sessionDesc.preprocessorMacros = slangMacros.data();
    sessionDesc.preprocessorMacroCount = slangMacros.size();
    sessionDesc.defaultMatrixLayoutMode = _rowMajor ? SLANG_MATRIX_LAYOUT_ROW_MAJOR : SLANG_MATRIX_LAYOUT_COLUMN_MAJOR;
    slang::TargetDesc targetDesc = {};
    targetDesc.format = SLANG_SPIRV;
    targetDesc.profile = _slangGlobalSession->findProfile("sm_6_6");
    targetDesc.flags = SLANG_TARGET_FLAG_GENERATE_SPIRV_DIRECTLY;
    targetDesc.forceGLSLScalarBufferLayout = true;
    sessionDesc.targets = &targetDesc;
    sessionDesc.targetCount = 1;
    std::vector<slang::CompilerOptionEntry> options = {};
    options.push_back({ slang::CompilerOptionName::DebugInformation, { slang::CompilerOptionValueKind::Int, 0, 0, nullptr, nullptr }});
    options.push_back({ slang::CompilerOptionName::Optimization, { slang::CompilerOptionValueKind::Int, 3, 0, nullptr, nullptr }});
    options.push_back({ slang::CompilerOptionName::EmitSpirvDirectly, { slang::CompilerOptionValueKind::Int, 1, 0, nullptr, nullptr }});
    options.push_back({ slang::CompilerOptionName::GLSLForceScalarLayout, { slang::CompilerOptionValueKind::Int, 1, 0, nullptr, nullptr }});
    options.push_back({ _rowMajor ? slang::CompilerOptionName::MatrixLayoutRow : slang::CompilerOptionName::MatrixLayoutColumn, { slang::CompilerOptionValueKind::Int, 1, 0, nullptr, nullptr }});
    options.push_back({ slang::CompilerOptionName::VulkanUseEntryPointName, { slang::CompilerOptionValueKind::Int, 1, 0, nullptr, nullptr }});
    sessionDesc.compilerOptionEntries = options.data();
    sessionDesc.compilerOptionEntryCount = options.size();

    Slang::ComPtr<slang::ISession> session = {};
    auto res = _slangGlobalSession->createSession(sessionDesc, session.writeRef());
    if (0 != res)
        return std::unexpected("Could not create slang session");

    // load canta module by default
    Slang::ComPtr<slang::IModule> cantaModule = {};
    {
        Slang::ComPtr<slang::IBlob> diagnostics = {};
        for (auto& file : _virtualFiles) {
            if (file.first == "canta.slang") {
                cantaModule = session->loadModuleFromSourceString("canta", file.first.c_str(), file.second.c_str(), diagnostics.writeRef());
                DIAGNOSE(diagnostics);
                break;
            }
        }
    }

    if (macros.empty() && !_slangMainSession) {
        _slangMainSession = session;
    }

    return session;
}

auto canta::PipelineManager::findVirtualFile(const std::filesystem::path &path) -> std::expected<std::string, Error> {
    for (auto& file : _virtualFiles) {
        if (file.first == path.string()) {
            return file.second;
        }
    }
    return std::unexpected(Error::InvalidPath);
}

auto loadShaderDescription(canta::PipelineManager& manager, rapidjson::Value& node, std::span<const canta::Macro> additionalMacros = {}) -> canta::ShaderDescription {
    canta::ShaderDescription description = {};
    if (node.HasMember("path")) {
        assert(node["path"].IsString());
        description.path = node["path"].GetString();
    }
    if (node.HasMember("slang")) {
        assert(node["slang"].IsString());
        description.slang = node["slang"].GetString();
    }
    if (node.HasMember("macros")) {
        assert(node["macros"].IsArray());
        for (auto i = 0; i < node["macros"].Size(); i++) {
            canta::Macro macro = {};
            macro.name = node["macros"][i]["name"].GetString();
            macro.value = node["macros"][i]["value"].GetString();
        }
    }
    if (node.HasMember("entryPoint")) {
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
        std::string entryPoint = "main";
        if (vertexShader.HasMember("entryPoint")) {
            entryPoint = vertexShader["entryPoint"].GetString();
        }
        createInfo.vertex = {
            .module = manager.getShader(loadShaderDescription(manager, vertexShader, additionalMacros)).value(),
            .entryPoint = entryPoint
        };
    }
    if (document.HasMember("tesselationControl")) {
        rapidjson::Value& tesselationControl = document["tesselationControl"];
        assert(tesselationControl.IsObject());
        std::string entryPoint = "main";
        if (tesselationControl.HasMember("entryPoint")) {
            entryPoint = tesselationControl["entryPoint"].GetString();
        }
        createInfo.tesselationControl = {
            .module = manager.getShader(loadShaderDescription(manager, tesselationControl, additionalMacros)).value(),
            .entryPoint = entryPoint
        };
    }
    if (document.HasMember("tesselationEvaluation")) {
        rapidjson::Value& tesselationEvaluation = document["tesselationEvaluation"];
        assert(tesselationEvaluation.IsObject());
        std::string entryPoint = "main";
        if (tesselationEvaluation.HasMember("entryPoint")) {
            entryPoint = tesselationEvaluation["entryPoint"].GetString();
        }
        createInfo.tesselationEvaluation = {
            .module = manager.getShader(loadShaderDescription(manager, tesselationEvaluation, additionalMacros)).value(),
            .entryPoint = entryPoint
        };
    }
    if (document.HasMember("geometry")) {
        rapidjson::Value& geometry = document["geometry"];
        assert(geometry.IsObject());
        std::string entryPoint = "main";
        if (geometry.HasMember("entryPoint")) {
            entryPoint = geometry["entryPoint"].GetString();
        }
        createInfo.geometry = {
            .module = manager.getShader(loadShaderDescription(manager, geometry, additionalMacros)).value(),
            .entryPoint = entryPoint
        };
    }
    if (document.HasMember("fragment")) {
        rapidjson::Value& fragment = document["fragment"];
        assert(fragment.IsObject());
        std::string entryPoint = "main";
        if (fragment.HasMember("entryPoint")) {
            entryPoint = fragment["entryPoint"].GetString();
        }
        createInfo.fragment = {
            .module = manager.getShader(loadShaderDescription(manager, fragment, additionalMacros)).value(),
            .entryPoint = entryPoint
        };
    }
    if (document.HasMember("compute")) {
        rapidjson::Value& compute = document["compute"];
        assert(compute.IsObject());
        std::string entryPoint = "main";
        if (compute.HasMember("entryPoint")) {
            entryPoint = compute["entryPoint"].GetString();
        }
        createInfo.compute = {
            .module = manager.getShader(loadShaderDescription(manager, compute, additionalMacros)).value(),
            .entryPoint = entryPoint
        };
    }
    if (document.HasMember("rayGen")) {
        rapidjson::Value& rayGen = document["rayGen"];
        assert(rayGen.IsObject());
        std::string entryPoint = "main";
        if (rayGen.HasMember("entryPoint")) {
            entryPoint = rayGen["entryPoint"].GetString();
        }
        createInfo.rayGen = {
            .module = manager.getShader(loadShaderDescription(manager, rayGen, additionalMacros)).value(),
            .entryPoint = entryPoint
        };
    }
    if (document.HasMember("anyHit")) {
        rapidjson::Value& anyHit = document["anyHit"];
        assert(anyHit.IsObject());
        std::string entryPoint = "main";
        if (anyHit.HasMember("entryPoint")) {
            entryPoint = anyHit["entryPoint"].GetString();
        }
        createInfo.anyHit = {
            .module = manager.getShader(loadShaderDescription(manager, anyHit, additionalMacros)).value(),
            .entryPoint = entryPoint
        };
    }
    if (document.HasMember("closestHit")) {
        rapidjson::Value& closestHit = document["closestHit"];
        assert(closestHit.IsObject());
        std::string entryPoint = "main";
        if (closestHit.HasMember("entryPoint")) {
            entryPoint = closestHit["entryPoint"].GetString();
        }
        createInfo.closestHit = {
            .module = manager.getShader(loadShaderDescription(manager, closestHit, additionalMacros)).value(),
            .entryPoint = entryPoint
        };
    }
    if (document.HasMember("miss")) {
        rapidjson::Value& miss = document["miss"];
        assert(miss.IsObject());
        std::string entryPoint = "main";
        if (miss.HasMember("entryPoint")) {
            entryPoint = miss["entryPoint"].GetString();
        }
        createInfo.miss = {
            .module = manager.getShader(loadShaderDescription(manager, miss, additionalMacros)).value(),
            .entryPoint = entryPoint
        };
    }
    if (document.HasMember("intersection")) {
        rapidjson::Value& intersection = document["intersection"];
        assert(intersection.IsObject());
        std::string entryPoint = "main";
        if (intersection.HasMember("entryPoint")) {
            entryPoint = intersection["entryPoint"].GetString();
        }
        createInfo.intersection = {
            .module = manager.getShader(loadShaderDescription(manager, intersection, additionalMacros)).value(),
            .entryPoint = entryPoint
        };
    }
    if (document.HasMember("callable")) {
        rapidjson::Value& callable = document["callable"];
        assert(callable.IsObject());
        std::string entryPoint = "main";
        if (callable.HasMember("entryPoint")) {
            entryPoint = callable["entryPoint"].GetString();
        }
        createInfo.callable = {
            .module = manager.getShader(loadShaderDescription(manager, callable, additionalMacros)).value(),
            .entryPoint = entryPoint
        };
    }
    if (document.HasMember("task")) {
        rapidjson::Value& task = document["task"];
        assert(task.IsObject());
        std::string entryPoint = "main";
        if (task.HasMember("entryPoint")) {
            entryPoint = task["entryPoint"].GetString();
        }
        createInfo.task = {
            .module = manager.getShader(loadShaderDescription(manager, task, additionalMacros)).value(),
            .entryPoint = entryPoint
        };
    }
    if (document.HasMember("mesh")) {
        rapidjson::Value& mesh = document["mesh"];
        assert(mesh.IsObject());
        std::string entryPoint = "main";
        if (mesh.HasMember("entryPoint")) {
            entryPoint = mesh["entryPoint"].GetString();
        }
        createInfo.mesh = {
            .module = manager.getShader(loadShaderDescription(manager, mesh, additionalMacros)).value(),
            .entryPoint = entryPoint
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