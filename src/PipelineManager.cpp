#include <cstring>
#include "Canta/PipelineManager.h"
#include <Ende/util/hash.h>
#include <Ende/filesystem/File.h>
#include <format>
#include <rapidjson/document.h>
#include "embedded_shaders_Canta.h"

size_t std::hash<canta::PipelineDescription>::operator()(const canta::PipelineDescription &object) const noexcept {
    u64 hash = 0;
    if (object.vertex) hash = ende::util::combineHash(hash, static_cast<u64>(ende::util::MurmurHash<canta::ShaderDescription>()(object.vertex)));
    if (object.tesselationControl) hash = ende::util::combineHash(hash, static_cast<u64>(ende::util::MurmurHash<canta::ShaderDescription>()(object.tesselationControl)));
    if (object.tesselationEvaluation) hash = ende::util::combineHash(hash, static_cast<u64>(ende::util::MurmurHash<canta::ShaderDescription>()(object.tesselationEvaluation)));
    if (object.geometry) hash = ende::util::combineHash(hash, static_cast<u64>(ende::util::MurmurHash<canta::ShaderDescription>()(object.geometry)));
    if (object.fragment) hash = ende::util::combineHash(hash, static_cast<u64>(ende::util::MurmurHash<canta::ShaderDescription>()(object.fragment)));
    if (object.compute) hash = ende::util::combineHash(hash, static_cast<u64>(ende::util::MurmurHash<canta::ShaderDescription>()(object.compute)));
    if (object.rayGen) hash = ende::util::combineHash(hash, static_cast<u64>(ende::util::MurmurHash<canta::ShaderDescription>()(object.rayGen)));
    if (object.anyHit) hash = ende::util::combineHash(hash, static_cast<u64>(ende::util::MurmurHash<canta::ShaderDescription>()(object.anyHit)));
    if (object.closestHit) hash = ende::util::combineHash(hash, static_cast<u64>(ende::util::MurmurHash<canta::ShaderDescription>()(object.closestHit)));
    if (object.miss) hash = ende::util::combineHash(hash, static_cast<u64>(ende::util::MurmurHash<canta::ShaderDescription>()(object.miss)));
    if (object.intersection) hash = ende::util::combineHash(hash, static_cast<u64>(ende::util::MurmurHash<canta::ShaderDescription>()(object.intersection)));
    if (object.callable) hash = ende::util::combineHash(hash, static_cast<u64>(ende::util::MurmurHash<canta::ShaderDescription>()(object.callable)));
    if (object.task) hash = ende::util::combineHash(hash, static_cast<u64>(ende::util::MurmurHash<canta::ShaderDescription>()(object.task)));
    if (object.mesh) hash = ende::util::combineHash(hash, static_cast<u64>(ende::util::MurmurHash<canta::ShaderDescription>()(object.mesh)));

    hash = ende::util::combineHash(hash, static_cast<u64>(ende::util::MurmurHash<canta::RasterState>()(object.rasterState)));
    hash = ende::util::combineHash(hash, static_cast<u64>(ende::util::MurmurHash<canta::DepthState>()(object.depthState)));
    hash = ende::util::combineHash(hash, static_cast<u64>(ende::util::MurmurHash<canta::BlendState>()(object.blendState)));
    for (auto& binding : object.inputBindings)
        hash = ende::util::combineHash(hash, static_cast<u64>(ende::util::MurmurHash<canta::VertexInputBinding>()(binding)));
    for (auto& attribute : object.inputAttributes)
        hash = ende::util::combineHash(hash, static_cast<u64>(ende::util::MurmurHash<canta::VertexInputAttribute>()(attribute)));
    for (auto& format : object.colourFormats)
        hash = ende::util::combineHash(hash, static_cast<u64>(format));
    hash = ende::util::combineHash(hash, static_cast<u64>(object.topology));
    hash = ende::util::combineHash(hash, object.primitiveRestart);
    hash = ende::util::combineHash(hash, static_cast<u64>(object.depthFormat));
    return hash;
}

size_t std::hash<canta::ShaderDescription>::operator()(const canta::ShaderDescription &object) const noexcept {
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
    // hash = ende::util::combineHash(hash, (u64)object.stage);
    return hash;
}

bool canta::operator==(const PipelineDescription &lhs, const PipelineDescription &rhs) {
    return lhs.vertex == rhs.vertex &&
        lhs.tesselationControl == rhs.tesselationControl && lhs.tesselationEvaluation == rhs.tesselationEvaluation &&
        lhs.geometry == rhs.geometry && lhs.fragment == rhs.fragment &&
        lhs.compute == rhs.compute && lhs.rayGen == rhs.rayGen &&
        lhs.anyHit == rhs.anyHit && lhs.closestHit == rhs.closestHit &&
        lhs.miss == rhs.miss && lhs.intersection == rhs.intersection &&
        lhs.callable == rhs.callable && lhs.task == rhs.task && lhs.mesh == rhs.mesh &&
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
    auto result = lhs.path == rhs.path && lhs.spirv.size() == rhs.spirv.size() &&
            lhs.slang.size() == rhs.slang.size() && lhs.macros.size() == rhs.macros.size() &&
            memcmp(reinterpret_cast<const void*>(lhs.spirv.data()), reinterpret_cast<const void*>(rhs.spirv.data()), sizeof(u32) * lhs.spirv.size()) == 0 &&
                lhs.entry == rhs.entry;
    if (!result)
        return result;
    for (u32 i = 0; i < lhs.macros.size(); i++) {
        result = result && lhs.macros[i].name == rhs.macros[i].name && lhs.macros[i].value == rhs.macros[i].value;
    }
    return result;
}

auto canta::PipelineManager::create(CreateInfo info) -> PipelineManager {
    PipelineManager manager = {};
    manager._device = info.device;
    manager._searchPaths.emplace_back(info.rootPath);
    manager._searchPaths.insert(manager._searchPaths.end(), info.searchPaths.begin(), info.searchPaths.end());
    manager._rowMajor = info.rowMajor;

    registerEmbededShadersCanta(manager);

    slang::createGlobalSession(manager._slangGlobalSession.writeRef());
    return manager;
}

canta::PipelineManager::PipelineManager(PipelineManager &&rhs) noexcept {
    std::swap(_device, rhs._device);
    std::swap(_searchPaths, rhs._searchPaths);
    std::swap(_rowMajor, rhs._rowMajor);
    std::swap(_pipelines, rhs._pipelines);
    std::swap(_fileWatcher, rhs._fileWatcher);
    std::swap(_slangGlobalSession, rhs._slangGlobalSession);
    std::swap(_slangMainSession, rhs._slangMainSession);
}

auto canta::PipelineManager::operator=(PipelineManager &&rhs) noexcept -> PipelineManager & {
    std::swap(_device, rhs._device);
    std::swap(_searchPaths, rhs._searchPaths);
    std::swap(_rowMajor, rhs._rowMajor);
    std::swap(_pipelines, rhs._pipelines);
    std::swap(_fileWatcher, rhs._fileWatcher);
    std::swap(_slangGlobalSession, rhs._slangGlobalSession);
    std::swap(_slangMainSession, rhs._slangMainSession);
    return *this;
}

auto canta::PipelineManager::getPipeline(PipelineDescription info, const PipelineHandle& oldPipeline) -> std::expected<PipelineHandle, Error> {
    const auto it = _pipelines.find(info);
    if (it != _pipelines.end() && !oldPipeline)
        return it->second;

    const auto evalShader = [&](ShaderDescription& shaderInfo, ShaderStage stage) -> std::expected<ShaderInfo, Error> {
        if (!shaderInfo) {
            return ShaderInfo{};
        }

        ShaderInfo value = {};
        value.entry = shaderInfo.entry;

        if (!shaderInfo.spirv.empty()) {
            value.spirv = shaderInfo.spirv;
            return value;
        }

        std::string source = {};
        std::vector<u32> spirv = {};

        if (!shaderInfo.slang.empty()) {
            source = shaderInfo.slang;
        }
        if (!shaderInfo.path.empty()) {
            const auto shaderFile = ende::fs::File::open(_searchPaths.front() / shaderInfo.path);
            if (!shaderFile) {
                _device->logger().error("Invalid shader path: {}", (_searchPaths.front() / shaderInfo.path).string());
                return std::unexpected(Error::InvalidPath);
            }
            source = shaderFile->read();
            _fileWatcher.addWatch(_searchPaths.front() / shaderInfo.path);
        }
        spirv = TRY(compileSlang(shaderInfo.path.stem().string(), source, stage, shaderInfo.macros).transform_error([this](const auto& error) {
            _device->logger().error("Shader VulkanError: {}", error.c_str());
            return Error::InvalidShader;
        }));

        value.spirv = spirv;
        return value;
    };

    Pipeline::CreateInfo createInfo = {};
    createInfo.vertex = TRY(evalShader(info.vertex, ShaderStage::VERTEX));
    createInfo.tesselationControl = TRY(evalShader(info.tesselationControl, ShaderStage::TESS_CONTROL));
    createInfo.tesselationEvaluation = TRY(evalShader(info.tesselationEvaluation, ShaderStage::TESS_EVAL));
    createInfo.geometry = TRY(evalShader(info.geometry, ShaderStage::GEOMETRY));
    createInfo.fragment = TRY(evalShader(info.fragment, ShaderStage::FRAGMENT));
    createInfo.compute = TRY(evalShader(info.compute, ShaderStage::COMPUTE));
    createInfo.rayGen = TRY(evalShader(info.rayGen, ShaderStage::RAYGEN));
    createInfo.anyHit = TRY(evalShader(info.anyHit, ShaderStage::ANY_HIT));
    createInfo.closestHit = TRY(evalShader(info.closestHit, ShaderStage::CLOSEST_HIT));
    createInfo.miss = TRY(evalShader(info.miss, ShaderStage::MISS));
    createInfo.intersection = TRY(evalShader(info.intersection, ShaderStage::INTERSECTION));
    createInfo.callable = TRY(evalShader(info.callable, ShaderStage::CALLABLE));
    createInfo.task = TRY(evalShader(info.task, ShaderStage::TASK));
    createInfo.mesh = TRY(evalShader(info.mesh, ShaderStage::MESH));

    auto handle = _device->createPipeline(createInfo, oldPipeline);
    if (!handle) return std::unexpected(Error::InvalidPipeline);

    if (it != _pipelines.end()) it.value<>() = handle;
    else _pipelines.insert(std::make_pair(info, handle));

    return handle;
}

auto loadShaderDescription(rapidjson::Value& node, std::span<const canta::Macro> additionalMacros = {}) -> canta::ShaderDescription {
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
    if (node.HasMember("entry")) {
        description.entry = node["entry"].GetString();
    }
    description.macros.insert(description.macros.end(), additionalMacros.begin(), additionalMacros.end());

    return description;
}

auto loadRasterState(rapidjson::Value& node) -> canta::RasterState {
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

auto loadDepthState(rapidjson::Value& node) -> canta::DepthState {
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

auto loadBlendState(rapidjson::Value& node) -> canta::BlendState {
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

auto canta::PipelineManager::getPipeline(const std::filesystem::path& path, std::span<const Macro> additionalMacros, const std::vector<SpecializationConstant> &specializationConstants) -> std::expected<PipelineHandle, Error> {
    const auto file = TRY(ende::fs::File::open(path).transform_error([] (const auto& error) { return Error::InvalidPath; }));

    rapidjson::Document document;
    document.Parse(file.read().c_str());
    assert(document.IsObject());

    PipelineDescription createInfo = {};

    if (document.HasMember("vertex")) {
        rapidjson::Value& vertexShader = document["vertex"];
        assert(vertexShader.IsObject());
        createInfo.vertex = loadShaderDescription(vertexShader, additionalMacros);
    }
    if (document.HasMember("tesselationControl")) {
        rapidjson::Value& tesselationControl = document["tesselationControl"];
        assert(tesselationControl.IsObject());
        createInfo.tesselationControl = loadShaderDescription(tesselationControl, additionalMacros);
    }
    if (document.HasMember("tesselationEvaluation")) {
        rapidjson::Value& tesselationEvaluation = document["tesselationEvaluation"];
        assert(tesselationEvaluation.IsObject());
        createInfo.tesselationEvaluation = loadShaderDescription(tesselationEvaluation, additionalMacros);
    }
    if (document.HasMember("geometry")) {
        rapidjson::Value& geometry = document["geometry"];
        assert(geometry.IsObject());
        createInfo.geometry = loadShaderDescription(geometry, additionalMacros);
    }
    if (document.HasMember("fragment")) {
        rapidjson::Value& fragment = document["fragment"];
        assert(fragment.IsObject());
        createInfo.fragment = loadShaderDescription(fragment, additionalMacros);
    }
    if (document.HasMember("compute")) {
        rapidjson::Value& compute = document["compute"];
        assert(compute.IsObject());
        createInfo.compute = loadShaderDescription(compute, additionalMacros);
    }
    if (document.HasMember("rayGen")) {
        rapidjson::Value& rayGen = document["rayGen"];
        assert(rayGen.IsObject());
        createInfo.rayGen = loadShaderDescription(rayGen, additionalMacros);
    }
    if (document.HasMember("anyHit")) {
        rapidjson::Value& anyHit = document["anyHit"];
        assert(anyHit.IsObject());
        createInfo.anyHit = loadShaderDescription(anyHit, additionalMacros);
    }
    if (document.HasMember("closestHit")) {
        rapidjson::Value& closestHit = document["closestHit"];
        assert(closestHit.IsObject());
        createInfo.closestHit = loadShaderDescription(closestHit, additionalMacros);
    }
    if (document.HasMember("miss")) {
        rapidjson::Value& miss = document["miss"];
        assert(miss.IsObject());
        createInfo.miss = loadShaderDescription(miss, additionalMacros);
    }
    if (document.HasMember("intersection")) {
        rapidjson::Value& intersection = document["intersection"];
        assert(intersection.IsObject());
        createInfo.intersection = loadShaderDescription(intersection, additionalMacros);
    }
    if (document.HasMember("callable")) {
        rapidjson::Value& callable = document["callable"];
        assert(callable.IsObject());
        createInfo.callable = loadShaderDescription(callable, additionalMacros);
    }
    if (document.HasMember("task")) {
        rapidjson::Value& task = document["task"];
        assert(task.IsObject());
        createInfo.task = loadShaderDescription(task, additionalMacros);
    }
    if (document.HasMember("mesh")) {
        rapidjson::Value& mesh = document["mesh"];
        assert(mesh.IsObject());
        createInfo.mesh = loadShaderDescription(mesh, additionalMacros);
    }
    if (document.HasMember("rasterState")) {
        rapidjson::Value& rasterState = document["rasterState"];
        assert(rasterState.IsObject());
        createInfo.rasterState = loadRasterState(rasterState);
    }
    if (document.HasMember("depthState")) {
        rapidjson::Value& depthState = document["depthState"];
        assert(depthState.IsObject());
        createInfo.depthState = loadDepthState(depthState);
    }
    if (document.HasMember("blendState")) {
        rapidjson::Value& blendState = document["blendState"];
        assert(blendState.IsObject());
        createInfo.blendState = loadBlendState(blendState);
    }

    return getPipeline(createInfo);
}

auto canta::PipelineManager::reload() -> std::expected<bool, Error> {
    for (auto events = _fileWatcher.read(); auto& [path, mask] : events) {
        if (auto it = _watchedFiles.find(path); it != _watchedFiles.end()) {
            for (auto& [key, value] : _pipelines) {
                if (key == it->second) {
                    TRY(getPipeline(key, value));
                }
            }
        }
    }
    return true;
}

auto canta::PipelineManager::reload(const PipelineHandle& pipeline) -> std::expected<PipelineHandle, Error> {
    for (auto& [key, value] : _pipelines) {
        if (value == pipeline)
            return getPipeline(key, value);
    }
    return std::unexpected(Error::InvalidPipeline);
}

void canta::PipelineManager::addVirtualFile(const std::filesystem::path &path, const std::string& contents) {
    _virtualFiles.emplace_back( path.string(), contents );
}

auto canta::PipelineManager::findVirtualFile(const std::filesystem::path &path) const -> std::expected<std::string, Error> {
    for (const auto& file : _virtualFiles) {
        if (file.first == path.string()) {
            return file.second;
        }
    }
    return std::unexpected(Error::InvalidPath);
}

#define DIAGNOSE(diagnostics) if ((diagnostics) != nullptr) return std::unexpected(reinterpret_cast<const char*>(diagnostics->getBufferPointer()));

auto canta::PipelineManager::compileSlang(const std::string_view name, const std::string_view slang, ShaderStage stage, const std::span<const Macro> macros) -> std::expected<std::vector<u32>, std::string> {
    auto session = TRY(createSlangSession(macros));

    std::string source = R"(
    #define GROUP_SIZE(x,y,z) [vk::constant_id(0)] const uint x_size = x;\
    [vk::constant_id(1)] const uint y_size = y;\
    [vk::constant_id(2)] const uint z_size = z;\
    static const uint3 groupSize = uint3(x_size, y_size, z_size);

    #define NUM_THREADS [numthreads(x_size, y_size, z_size)]

    #define SPEC_CONSTANT_INDEX(index) 3 + index

    )";

    source += slang;

    Slang::ComPtr<slang::IModule> slangModule = {};
    {
        Slang::ComPtr<slang::IBlob> diagnostics = {};
        slangModule = session->loadModuleFromSourceString(name.data(), name.data(), source.data(), diagnostics.writeRef());
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
    spirv.insert(spirv.begin(), (u32*)kernelBlob->getBufferPointer(), reinterpret_cast<u32 *>((u8 *) kernelBlob->getBufferPointer() + kernelBlob->getBufferSize()));
    return spirv;
}

auto canta::PipelineManager::createSlangSession(const std::span<const Macro> macros) -> std::expected<Slang::ComPtr<slang::ISession>, std::string> {
    if (macros.empty() && _slangMainSession) {
     return _slangMainSession;
    }

    slang::SessionDesc sessionDesc = {};
    std::vector<const char*> searchPaths = {};
    for (auto& path : _searchPaths) {
        searchPaths.emplace_back(path.c_str());
    }
    sessionDesc.searchPaths = searchPaths.data();
    sessionDesc.searchPathCount = searchPaths.size();
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
     return std::unexpected("Failed to compile shader");

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
