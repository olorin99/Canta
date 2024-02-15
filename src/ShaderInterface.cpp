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

void getStruct(tsl::robin_map<std::string, canta::ShaderInterface::Member>& types, const spirv_cross::SPIRType& type, spirv_cross::Compiler& compiler, u32 depth) {
    if (depth > 3)
        return;
    for (u32 memberIndex = 0; memberIndex < type.member_types.size(); memberIndex++) {
        auto& memberType = compiler.get_type(type.member_types[memberIndex]);
        const std::string& name = compiler.get_member_name(type.self, memberIndex);
        u32 a = compiler.get_declared_struct_size_runtime_array(type, 1);
        u32 memberSize = compiler.get_declared_struct_member_size(type, memberIndex);
        if (memberSize == 0 && a > 0)
            memberSize = a;
        u32 memberOffset = compiler.type_struct_member_offset(type, memberIndex);
        types[name] = { name, memberSize, memberOffset, typeToMemberType(memberType) };
        if (memberType.basetype == spirv_cross::SPIRType::Struct)
            getStruct(types, memberType, compiler, depth + 1);
    }
}

auto canta::ShaderInterface::create(std::span<CreateInfo> infos) -> ShaderInterface {
    ShaderInterface interface = {};

    for (auto& info : infos) {
        spirv_cross::Compiler compiler(info.spirv.data(), info.spirv.size());
        auto resources = compiler.get_shader_resources();

        interface._stages |= info.stage;

        if (info.stage == ShaderStage::COMPUTE || info.stage == ShaderStage::MESH || info.stage == ShaderStage::TASK) {
            u32 x = compiler.get_execution_mode_argument(spv::ExecutionModeLocalSize, 0);
            u32 y = compiler.get_execution_mode_argument(spv::ExecutionModeLocalSize, 1);
            u32 z = compiler.get_execution_mode_argument(spv::ExecutionModeLocalSize, 2);
            interface._localSizes.push_back(std::make_pair(ende::math::Vec<3, u32>{ x, y, z }, info.stage));
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
                    auto structMembers = getStructMembers(memberType, compiler);
                    for (auto& member : structMembers) {
                        member.offset += memberOffset;
                        members.push_back(name);
                        interface._types[name] = { name, memberSize, memberOffset, typeToMemberType(memberType) };
                    }
                    getStruct(interface._types, memberType, compiler, 0);
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

        for (auto& inputPushRange : moduleInterface._pushRanges)
            interface._pushRanges.push_back(inputPushRange);

        for (auto& inputLocalSize : moduleInterface._localSizes)
            interface._localSizes.push_back(inputLocalSize);

        interface._types.insert(moduleInterface._types.begin(), moduleInterface._types.end());
    }

    return interface;
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

auto canta::ShaderInterface::localSize(canta::ShaderStage stage) const -> ende::math::Vec<3, u32> {
    for (auto& sizePair : _localSizes) {
        if (sizePair.second == stage)
            return sizePair.first;
    }
    return { 1, 1, 1 };
}

auto canta::ShaderInterface::stagePresent(canta::ShaderStage stage) const -> bool {
    return (_stages & stage) == stage;
}