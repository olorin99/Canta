#ifndef CANTA_SAMPLER_H
#define CANTA_SAMPLER_H

#include <Ende/platform.h>
#include <volk.h>
#include <Canta/Enums.h>

namespace canta {

    class Device;

    class Sampler {
    public:

        struct CreateInfo {
            Filter filter = Filter::NEAREST;
            AddressMode addressMode = AddressMode::REPEAT;
            bool anisotropy = true;
            f32 maxAnisotropy = 0.f;
            bool compare = false;
            CompareOp compareOp = CompareOp::ALWAYS;
            MipmapMode mipmapMode = MipmapMode::NEAREST;
            f32 mipLodBias = 0.f;
            f32 minLod = 0.f;
            f32 maxLod = 10.f;
            BorderColour borderColour = BorderColour::TRANSPARENT_BLACK_FLOAT;
            bool unnormalisedCoordinates = false;
        };

        Sampler() = default;

        ~Sampler();

        Sampler(Sampler&& rhs) noexcept;
        auto operator=(Sampler&& rhs) noexcept -> Sampler&;

        auto sampler() const -> VkSampler { return _sampler; }

    private:
        friend Device;

        Device* _device = nullptr;
        VkSampler _sampler = VK_NULL_HANDLE;

    };

}

#endif //CANTA_SAMPLER_H
