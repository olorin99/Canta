#include "Canta/ShaderInterface.h"
#include <spirv_cross.hpp>

auto typeToMemberType(const spirv_cross::SPIRType& type) {
    switch (type.basetype) {
        case spirv_cross::SPIRType::BaseType::Struct:
            return canta::ShaderInterface::MemberType::STRUCT;
        case spirv_cross::SPIRType::Unknown:
            break;
        case spirv_cross::SPIRType::Void:
            break;
        case spirv_cross::SPIRType::Boolean:
            return canta::ShaderInterface::MemberType::BOOL;
        case spirv_cross::SPIRType::SByte:
            break;
        case spirv_cross::SPIRType::UByte:
            break;
        case spirv_cross::SPIRType::Short:
            break;
        case spirv_cross::SPIRType::UShort:
            break;
        case spirv_cross::SPIRType::Int:
            return canta::ShaderInterface::MemberType::INT;
        case spirv_cross::SPIRType::UInt:
            return canta::ShaderInterface::MemberType::UINT;
        case spirv_cross::SPIRType::Int64:
            break;
        case spirv_cross::SPIRType::UInt64:
            break;
        case spirv_cross::SPIRType::AtomicCounter:
            break;
        case spirv_cross::SPIRType::Half:
            break;
        case spirv_cross::SPIRType::Float:
            return canta::ShaderInterface::MemberType::FLOAT;
        case spirv_cross::SPIRType::Double:
            break;
        case spirv_cross::SPIRType::Image:
            break;
        case spirv_cross::SPIRType::SampledImage:
            break;
        case spirv_cross::SPIRType::Sampler:
            break;
        case spirv_cross::SPIRType::AccelerationStructure:
            break;
        case spirv_cross::SPIRType::RayQuery:
            break;
        case spirv_cross::SPIRType::ControlPointArray:
            break;
        case spirv_cross::SPIRType::Interpolant:
            break;
        case spirv_cross::SPIRType::Char:
            break;
    }
    return canta::ShaderInterface::MemberType::STRUCT;
};

std::vector<canta::ShaderInterface::Member> getStructMembers(const spirv_cross::SPIRType& type, spirv_cross::Compiler& compiler) {
    std::vector<canta::ShaderInterface::Member> members;
    u32 memberCount = type.member_types.size();
    for (u32 memberIndex = 0; memberIndex < memberCount; memberIndex++) {
        auto& memberType = compiler.get_type(type.member_types[memberIndex]);
        const std::string& name = compiler.get_member_name(type.self, memberIndex);
        u32 a = compiler.get_declared_struct_size_runtime_array(type, 1);
        u32 memberSize = compiler.get_declared_struct_member_size(type, memberIndex);
        u32 memberOffset = compiler.type_struct_member_offset(type, memberIndex);
        members.push_back({ name, memberSize, memberOffset, typeToMemberType(memberType) });
    }
    return members;
}

auto getStruct(tsl::robin_map<std::string, canta::ShaderInterface::Member>& types, std::vector<std::string>& members, const spirv_cross::SPIRType& type, spirv_cross::Compiler& compiler, u32 depth) -> u32 {
    if (depth > 3)
        return 0;
    u32 structSize = 0;
    for (u32 memberIndex = 0; memberIndex < type.member_types.size(); memberIndex++) {
        auto& memberType = compiler.get_type(type.member_types[memberIndex]);
        const std::string& name = compiler.get_member_name(type.self, memberIndex);
        u32 a = compiler.get_declared_struct_size_runtime_array(type, 1);
        u32 memberSize = compiler.get_declared_struct_member_size(type, memberIndex);
        if (memberSize == 0 && a > 0)
            memberSize = a;
        u32 memberOffset = compiler.type_struct_member_offset(type, memberIndex);
        canta::ShaderInterface::Member member{ name, memberSize, memberOffset, typeToMemberType(memberType) };
        if (memberType.basetype == spirv_cross::SPIRType::Struct)
            getStruct(types, member.members, memberType, compiler, depth + 1);
        types[name] = member;
        members.push_back(name);
        structSize += memberSize;
    }
    return structSize;
}

auto matchExecutionModeToStage(const spv::ExecutionModel mode) -> canta::ShaderStage {
    switch (mode) {
        case spv::ExecutionModelVertex:
            return canta::ShaderStage::VERTEX;
        case spv::ExecutionModelTessellationControl:
            return canta::ShaderStage::TESS_CONTROL;
        case spv::ExecutionModelTessellationEvaluation:
            return canta::ShaderStage::TESS_EVAL;
        case spv::ExecutionModelGeometry:
            return canta::ShaderStage::GEOMETRY;
        case spv::ExecutionModelFragment:
            return canta::ShaderStage::FRAGMENT;
        case spv::ExecutionModelGLCompute:
            return canta::ShaderStage::COMPUTE;
        case spv::ExecutionModelKernel:
            return canta::ShaderStage::COMPUTE;
        case spv::ExecutionModelRayGenerationKHR:
            return canta::ShaderStage::RAYGEN;
        case spv::ExecutionModelIntersectionKHR:
            return canta::ShaderStage::INTERSECTION;
        case spv::ExecutionModelAnyHitKHR:
            return canta::ShaderStage::ANY_HIT;
        case spv::ExecutionModelClosestHitKHR:
            return canta::ShaderStage::CLOSEST_HIT;
        case spv::ExecutionModelMissKHR:
            return canta::ShaderStage::MISS;
        case spv::ExecutionModelCallableKHR:
            return canta::ShaderStage::CALLABLE;
        case spv::ExecutionModelTaskEXT:
            return canta::ShaderStage::TASK;
        case spv::ExecutionModelMeshEXT:
            return canta::ShaderStage::MESH;
        case spv::ExecutionModelMax:
            return canta::ShaderStage::NONE;
    }
    return canta::ShaderStage::NONE;
}

auto matchStageToExecutionModel(const canta::ShaderStage stage) -> spv::ExecutionModel {
    switch (stage) {
        case canta::ShaderStage::NONE:
            break;
        case canta::ShaderStage::VERTEX:
            return spv::ExecutionModelVertex;
        case canta::ShaderStage::TESS_CONTROL:
            return spv::ExecutionModelTessellationControl;
        case canta::ShaderStage::TESS_EVAL:
            return spv::ExecutionModelTessellationEvaluation;
        case canta::ShaderStage::GEOMETRY:
            return spv::ExecutionModelGeometry;
        case canta::ShaderStage::FRAGMENT:
            return spv::ExecutionModelFragment;
        case canta::ShaderStage::COMPUTE:
            return spv::ExecutionModelKernel;
        case canta::ShaderStage::ALL_GRAPHICS:
            return spv::ExecutionModelVertex;
        case canta::ShaderStage::ALL:
            return spv::ExecutionModelVertex;
        case canta::ShaderStage::RAYGEN:
            return spv::ExecutionModelVertex;
        case canta::ShaderStage::ANY_HIT:
            return spv::ExecutionModelVertex;
        case canta::ShaderStage::CLOSEST_HIT:
            return spv::ExecutionModelVertex;
        case canta::ShaderStage::MISS:
            return spv::ExecutionModelVertex;
        case canta::ShaderStage::INTERSECTION:
            return spv::ExecutionModelVertex;
        case canta::ShaderStage::CALLABLE:
            return spv::ExecutionModelVertex;
        case canta::ShaderStage::TASK:
            return spv::ExecutionModelVertex;
        case canta::ShaderStage::MESH:
            return spv::ExecutionModelVertex;
    }
    return spv::ExecutionModelMax;
}

auto canta::ShaderInterface::create(std::span<CreateInfo> infos) -> ShaderInterface {
    ShaderInterface interface = {};

    for (auto& info : infos) {
        spirv_cross::Compiler compiler(info.spirv.data(), info.spirv.size());

        // auto entryPoint = compiler.get_entry_point(info.entryPoint, )

        for (auto entryPoints = compiler.get_entry_points_and_stages(); auto&[name, execution_model] : entryPoints) {
            if (const auto stage = matchExecutionModeToStage(execution_model); info.stage == stage && info.entry == name) {
                compiler.set_entry_point(name, execution_model);
                break;
            }
        }

        auto resources = compiler.get_shader_resources();

        interface._stages |= info.stage;

        if (info.stage == ShaderStage::COMPUTE || info.stage == ShaderStage::MESH || info.stage == ShaderStage::TASK) {
            u32 x = compiler.get_execution_mode_argument(spv::ExecutionModeLocalSize, 0);
            u32 y = compiler.get_execution_mode_argument(spv::ExecutionModeLocalSize, 1);
            u32 z = compiler.get_execution_mode_argument(spv::ExecutionModeLocalSize, 2);
            interface._localSizes.emplace_back(ende::math::Vec<3, u32>{ x, y, z }, info.stage);
        }

        for (auto &c : compiler.get_specialization_constants())
        {
            const spirv_cross::SPIRConstant &value = compiler.get_constant(c.id);

            interface._specConstants.push_back({
                .id = c.constant_id,
                .name = compiler.get_name(c.id),
                .type = typeToMemberType(compiler.get_type(value.constant_type))
            });
        }

        for (u32 i = 0; i < resources.push_constant_buffers.size(); i++) {
            auto& push = resources.push_constant_buffers[i];
            auto& type = compiler.get_type(push.base_type_id);

            u32 size = compiler.get_declared_struct_size(type);
            u32 offset = 10000;
            std::vector<std::string> members = {};
            for (u32 memberIndex = 0; memberIndex < type.member_types.size(); memberIndex++) {
                auto& memberType = compiler.get_type(type.member_types[memberIndex]);
                auto& name = compiler.get_member_name(type.self, memberIndex);
                u32 memberSize = compiler.get_declared_struct_member_size(type, memberIndex);
                u32 memberOffset = compiler.type_struct_member_offset(type, memberIndex);
                offset = std::min(offset, memberOffset);

                if (memberType.basetype == spirv_cross::SPIRType::Struct) {
//                    auto structMembers = getStructMembers(memberType, compiler);
//                    for (auto& member : structMembers) {
//                        member.offset += memberOffset;
//                        members.push_back(name);
//                        interface._types[name] = { name, memberSize, memberOffset, typeToMemberType(memberType) };
//                    }
                    std::vector<std::string> structMembers = {};
                    auto structSize = getStruct(interface._types, structMembers, memberType, compiler, 0);
                    if (memberType.op == spv::OpTypePointer)
                        memberSize = structSize;
                    interface._types[name] = { name, memberSize, memberOffset, typeToMemberType(memberType), structMembers };
                } else {
                    members.push_back(name);
                    interface._types[name] = { name, memberSize, memberOffset, typeToMemberType(memberType) };
                }
            }

            interface._pushRanges.push_back({
                size,
                offset,
                info.stage,
                members
            });
        }

        for (u32 i = 0; i < resources.uniform_buffers.size(); i++) {
            auto& uniform = resources.uniform_buffers[i];
            auto& type = compiler.get_type(uniform.base_type_id);

            auto set = compiler.get_decoration(uniform.id, spv::DecorationDescriptorSet);
            auto binding = compiler.get_decoration(uniform.id, spv::DecorationBinding);

            auto size = compiler.get_declared_struct_size(type);

            u32 memberCount = type.member_types.size();
            for (u32 memberIndex = 0; memberIndex < memberCount; memberIndex++) {
                auto& memberType = compiler.get_type(type.member_types[memberIndex]);
                auto& name = compiler.get_member_name(type.self, memberIndex);
                u32 memberSize = compiler.get_declared_struct_member_size(type, memberIndex);
                u32 memberOffset = compiler.type_struct_member_offset(type, memberIndex);

                if (memberType.basetype == spirv_cross::SPIRType::Struct) {
                    auto structMembers = getStructMembers(memberType, compiler);
                    for (auto& member : structMembers) {
                        member.offset += memberOffset;
                        interface._sets[set].bindings[binding].members[name] = { name, memberSize, memberOffset, typeToMemberType(memberType) };
                    }
                } else {
                    interface._sets[set].bindings[binding].members[name] = { name, memberSize, memberOffset, typeToMemberType(memberType) };
                }
            }

            interface._sets[set].bindings[binding].binding = binding;
            interface._sets[set].bindings[binding].size = size;
            interface._sets[set].bindings[binding].type = BindingType::UNIFORM_BUFFER;
            interface._sets[set].bindingCount = std::max(interface._sets[set].bindingCount, binding + 1);
            if (interface._sets[set].bindingCount > 0)
                interface._setCount = std::max(interface._setCount, set + 1);
        }

        for (u32 i = 0; i < resources.storage_buffers.size(); i++) {
            auto& storage = resources.storage_buffers[i];
            auto& type = compiler.get_type(storage.base_type_id);

            auto set = compiler.get_decoration(storage.id, spv::DecorationDescriptorSet);
            auto binding = compiler.get_decoration(storage.id, spv::DecorationBinding);

            auto size = compiler.get_declared_struct_size(type);

            u32 memberCount = type.member_types.size();
            for (u32 memberIndex = 0; memberIndex < memberCount; memberIndex++) {
                auto& memberType = compiler.get_type(type.member_types[memberIndex]);
                auto& name = compiler.get_member_name(type.self, memberIndex);
                u32 memberSize = compiler.get_declared_struct_member_size(type, memberIndex);
                u32 memberOffset = compiler.type_struct_member_offset(type, memberIndex);

                if (memberType.basetype == spirv_cross::SPIRType::Struct) {
                    auto structMembers = getStructMembers(memberType, compiler);
                    for (auto& member : structMembers) {
                        member.offset += memberOffset;
                        interface._sets[set].bindings[binding].members[name] = { name, memberSize, memberOffset, typeToMemberType(memberType) };
                    }
                } else {
                    interface._sets[set].bindings[binding].members[name] = { name, memberSize, memberOffset, typeToMemberType(memberType) };
                }
            }

            interface._sets[set].bindings[binding].binding = binding;
            interface._sets[set].bindings[binding].size = size;
            interface._sets[set].bindings[binding].type = BindingType::UNIFORM_BUFFER;
            interface._sets[set].bindingCount = std::max(interface._sets[set].bindingCount, binding + 1);
            if (interface._sets[set].bindingCount > 0)
                interface._setCount = std::max(interface._setCount, set + 1);
        }

        for (u32 i = 0; i < resources.sampled_images.size(); i++) {
            auto& sampledImage = resources.sampled_images[i];

            u32 set = compiler.get_decoration(sampledImage.id, spv::DecorationDescriptorSet);
            u32 binding = compiler.get_decoration(sampledImage.id, spv::DecorationBinding);

            const spirv_cross::SPIRType &type = compiler.get_type(sampledImage.base_type_id);

            interface._sets[set].bindings[binding].binding = binding;
            interface._sets[set].bindings[binding].size = 1;
            interface._sets[set].bindings[binding].type = BindingType::SAMPLED_IMAGE;
            interface._sets[set].bindingCount = std::max(interface._sets[set].bindingCount, binding + 1);
            if (interface._sets[set].bindingCount > 0)
                interface._setCount = std::max(interface._setCount, set + 1);
        }

        for (u32 i = 0; i < resources.storage_images.size(); i++) {
            auto& storageImage = resources.storage_images[i];

            u32 set = compiler.get_decoration(storageImage.id, spv::DecorationDescriptorSet);
            u32 binding = compiler.get_decoration(storageImage.id, spv::DecorationBinding);

            const spirv_cross::SPIRType &type = compiler.get_type(storageImage.base_type_id);

            interface._sets[set].bindings[binding].binding = binding;
            interface._sets[set].bindings[binding].size = 1;
            interface._sets[set].bindings[binding].type = BindingType::STORAGE_IMAGE;
            interface._sets[set].bindingCount = std::max(interface._sets[set].bindingCount, binding + 1);
            if (interface._sets[set].bindingCount > 0)
                interface._setCount = std::max(interface._setCount, set + 1);
        }

    }

    return interface;
}

auto canta::ShaderInterface::merge(std::span<ShaderInterface> inputs) -> ShaderInterface {
    ShaderInterface interface = {};

    for (auto& moduleInterface : inputs) {
        interface._stages |= moduleInterface._stages;
        interface._setCount = std::max(interface._setCount, moduleInterface._setCount);

        for (u32 set = 0; set < moduleInterface._setCount; set++) {
            auto& inputSet = moduleInterface._sets[set];
            auto& outputSet = interface._sets[set];

            outputSet.bindingCount = std::max(outputSet.bindingCount, inputSet.bindingCount);
            outputSet.size = std::max(outputSet.size, inputSet.size);

            for (u32 binding = 0; binding < inputSet.bindingCount; binding++) {
                auto& inputBinding = inputSet.bindings[binding];
                auto& outputBinding = outputSet.bindings[binding];

                outputBinding.binding = inputBinding.binding;
                outputBinding.size = inputBinding.size;
                outputBinding.arrayLength = inputBinding.arrayLength;
                outputBinding.type = inputBinding.type;
                outputBinding.stages = inputBinding.stages;
                outputBinding.members = inputBinding.members;
            }
        }

        for (auto& inputSpecConst :moduleInterface._specConstants) {
            interface._specConstants.push_back(inputSpecConst);
        }

        for (auto& inputPushRange : moduleInterface._pushRanges)
            interface._pushRanges.push_back(inputPushRange);

        for (auto& inputLocalSize : moduleInterface._localSizes)
            interface._localSizes.push_back(inputLocalSize);

        interface._types.insert(moduleInterface._types.begin(), moduleInterface._types.end());
    }

    return interface;
}

auto canta::ShaderInterface::getSpecConstant(std::string_view name) const -> std::optional<SpecConstant> {
    for (auto& constant : _specConstants) {
        if (constant.name == name)
            return constant;
    }
    return std::nullopt;
}


auto canta::ShaderInterface::bindingHasMember(u32 set, u32 binding, std::string_view name) const -> bool {
    auto& b = _sets[set].bindings[binding];
    auto it = b.members.find(name.data());
    if (it != b.members.end())
        return true;
    return false;
}

auto canta::ShaderInterface::getBindingMember(u32 set, u32 binding, std::string_view name) const -> Member {
    auto& b = _sets[set].bindings[binding];
    auto it = b.members.find(name.data());
    if (it != b.members.end())
        return it->second;
    return {};
}

auto canta::ShaderInterface::getBindingMemberList(u32 set, u32 binding) const -> std::vector<Member> {
    std::vector<Member> members = {};
    for (auto [key, value] : _sets[set].bindings[binding].members)
        members.push_back(value);
    return members;
}

auto canta::ShaderInterface::getType(std::string_view name) const -> Member {
    auto it = _types.find(name.data());
    if (it != _types.end())
        return it->second;
    return {};
}

auto canta::ShaderInterface::getTypeList() const -> std::vector<Member> {
    std::vector<Member> members = {};
    for (auto [key, value] : _types)
        members.push_back(value);
    return members;
}

auto canta::ShaderInterface::localSize(canta::ShaderStage stage) const -> std::optional<ende::math::Vec<3, u32>> {
    for (auto& sizePair : _localSizes) {
        if (sizePair.second == stage)
            return sizePair.first;
    }
    return std::nullopt;
}

auto canta::ShaderInterface::stagePresent(canta::ShaderStage stage) const -> bool {
    return (_stages & stage) == stage;
}