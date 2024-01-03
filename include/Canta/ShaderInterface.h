#ifndef CANTA_SHADERINTERFACE_H
#define CANTA_SHADERINTERFACE_H

#include <Ende/platform.h>
#include <span>
#include <Canta/Enums.h>
#include <Ende/math/Vec.h>
#include <vector>
#include <tsl/robin_map.h>

namespace canta {

    class ShaderInterface {
    public:

        struct CreateInfo {
            std::span<u32> spirv = {};
            ShaderStage stage = ShaderStage::NONE;
        };

        static auto create(std::span<CreateInfo> infos) -> ShaderInterface;

        static auto merge(std::span<ShaderInterface> inputs) -> ShaderInterface;

        ShaderInterface() = default;

        enum class BindingType {
            NONE = 0,
            PUSH_CONSTANT = 1,
            UNIFORM_BUFFER = 2,
            STORAGE_BUFFER = 3,
            SAMPLED_IMAGE = 4,
            STORAGE_IMAGE = 5
        };

        enum class MemberType {
            STRUCT,
            BOOL,
            INT,
            UINT,
            FLOAT,
            VEC3F,
            VEC4F,
            IVEC3,
            IVEC4,
            UVEC3,
            UVEC4,
            MAT3F,
            MAT4F
        };

        struct Member {
            std::string name;
            u32 size = 0;
            u32 offset = 0;
            MemberType type = MemberType::STRUCT;
        };

        struct Binding {
            u32 binding = 0;
            u32 size = 0;
            u32 arrayLength = 0;
            BindingType type = BindingType::NONE;
            ShaderStage stages = ShaderStage::NONE;
            tsl::robin_map<std::string, Member> members = {};
        };

        struct Set {
            u32 set = 0;
            u32 bindingCount = 0;
            u32 size = 0;
            std::array<Binding, 5> bindings = {};
        };

        auto getSet(u32 set) const -> Set { return _sets[set]; }

        auto getBinding(u32 set, u32 binding) const -> Binding { return _sets[set].bindings[binding]; }

        auto bindingHasMember(u32 set, u32 binding, std::string_view name) const -> bool;

        auto getBindingMember(u32 set, u32 binding, std::string_view name) const -> Member;

        auto getBindingMemberList(u32 set, u32 binding) const -> std::vector<Member>;

    private:


        std::array<Set, 5> _sets = {};
        u32 _setCount = 0;

        struct PushRange {
            u32 size = 0;
            u32 offset = 0;
            ShaderStage stage = ShaderStage::NONE;
        };
        std::vector<PushRange> _pushRanges;
        std::vector<std::pair<ende::math::Vec<3, u32>, ShaderStage>> _localSizes;
        ShaderStage _stages;

    };

}

#endif //CANTA_SHADERINTERFACE_H
