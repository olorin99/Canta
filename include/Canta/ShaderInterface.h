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
            std::span<const u32> spirv = {};
            ShaderStage stage = ShaderStage::NONE;
        };

        [[nodiscard]] static auto create(std::span<CreateInfo> infos) -> ShaderInterface;

        [[nodiscard]] static auto merge(std::span<ShaderInterface> inputs) -> ShaderInterface;

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
            std::vector<std::string> members = {};
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

        struct PushRange {
            u32 size = 0;
            u32 offset = 0;
            ShaderStage stage = ShaderStage::NONE;
            std::vector<std::string> members = {};
        };

        struct SpecConstant {
            u32 id = 0;
            std::string name = "";
            MemberType type = MemberType::INT;
        };

        [[nodiscard]] auto getSpecConstant(std::string_view name) const -> std::optional<SpecConstant>;

        [[nodiscard]] auto getPushRange(u32 range) const -> const PushRange& { return _pushRanges[range]; }

        [[nodiscard]] auto getSet(u32 set) const -> const Set& { return _sets[set]; }

        [[nodiscard]] auto getBinding(u32 set, u32 binding) const -> const Binding& { return _sets[set].bindings[binding]; }

        [[nodiscard]] auto bindingHasMember(u32 set, u32 binding, std::string_view name) const -> bool;

        [[nodiscard]] auto getBindingMember(u32 set, u32 binding, std::string_view name) const -> Member;

        [[nodiscard]] auto getBindingMemberList(u32 set, u32 binding) const -> std::vector<Member>;

        [[nodiscard]] auto getType(std::string_view name) const -> Member;

        [[nodiscard]] auto getTypeList() const -> std::vector<Member>;

        [[nodiscard]] auto pushRangeCount() const -> u32 { return _pushRanges.size(); }

        [[nodiscard]] auto setCount() const -> u32 { return _setCount; }

        [[nodiscard]] auto bindingCount(u32 set) const -> u32 { return _sets[set].bindingCount; }

        [[nodiscard]] auto localSize(ShaderStage stage) const -> std::optional<ende::math::Vec<3, u32>>;

        [[nodiscard]] auto stagePresent(ShaderStage stage) const -> bool;

    private:


        std::array<Set, 5> _sets = {};
        u32 _setCount = 0;

        std::vector<PushRange> _pushRanges = {};
        std::vector<SpecConstant> _specConstants = {};
        std::vector<std::pair<ende::math::Vec<3, u32>, ShaderStage>> _localSizes = {};
        ShaderStage _stages = ShaderStage::NONE;
        tsl::robin_map<std::string, Member> _types = {};

    };

}

#endif //CANTA_SHADERINTERFACE_H
