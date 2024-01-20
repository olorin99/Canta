#include <cstring>
#include "Canta/PipelineManager.h"
#include <Ende/util/hash.h>
#include <Canta/util.h>
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
    if (!info.path.empty()) {
        auto shaderFile = ende::fs::File::open(_rootPath / info.path);
        auto glsl = shaderFile->read();
        auto spirv = util::compileGLSLToSpirv("", glsl, info.stage).transform_error([](const auto& error) {
            std::printf("%s", error.c_str());
            return error;
        }).value();
        createInfo.spirv = spirv;
        auto h = _device->createShaderModule(createInfo, handle);
        if (!handle) {
            auto watcher = _watchedPipelines.find(info.path);
            if (watcher == _watchedPipelines.end())
                _fileWatcher.addWatch(_rootPath / info.path);
            auto it1 = _shaders.insert(std::make_pair(info, h));
        }
        return h;
    }
    auto h = _device->createShaderModule(createInfo, handle);
    if (!handle) {
        auto it1 = _shaders.insert(std::make_pair(info, h));
        return it1.first->second;
    }
    return h;
}