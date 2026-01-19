#ifndef CANTA_SEMAPHORE_H
#define CANTA_SEMAPHORE_H

#include <Ende/platform.h>
#include <volk.h>
#include <expected>
#include <string_view>
#include <Canta/Enums.h>
#include <Canta/ResourceList.h>

namespace canta {

    class Device;
    class Semaphore;
    using SemaphoreHandle = Handle<Semaphore, ResourceList<Semaphore>>;

    class Semaphore {
    public:

        struct CreateInfo {
            i64 initialValue = -1;
            std::string_view name = {};
        };

        Semaphore() = default;
        ~Semaphore();

        Semaphore(Semaphore&& rhs) noexcept;
        auto operator=(Semaphore&& rhs) noexcept -> Semaphore&;

        [[nodiscard]] auto semaphore() const -> VkSemaphore { return _semaphore; }

        [[nodiscard]] auto isTimeline() const -> bool { return _isTimeline; }

        [[nodiscard]] auto value() const -> u64 { return _value; }

        [[nodiscard]] auto gpuValue() const -> u64;

        [[nodiscard]] auto increment() -> u64 { return ++_value; }

        [[nodiscard]] auto wait(u64 value, u64 timeout = 1000000000) -> std::expected<bool, VulkanError>;

        [[nodiscard]] auto signal(u64 value) -> std::expected<bool, VulkanError>;

    private:
        friend Device;

        Device* _device = nullptr;
        VkSemaphore _semaphore = VK_NULL_HANDLE;
        u64 _value = 0;
        bool _isTimeline = false;

    };

}

#endif //CANTA_SEMAPHORE_H
