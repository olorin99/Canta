#ifndef CANTA_SEMAPHORE_H
#define CANTA_SEMAPHORE_H

#include <Ende/platform.h>
#include <volk.h>
#include <expected>
#include <string_view>
#include <Canta/Enums.h>

namespace canta {

    class Device;

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

        auto semaphore() const -> VkSemaphore { return _semaphore; }

        auto isTimeline() const -> bool { return _isTimeline; }

        auto value() const -> u64 { return _value; }

        auto gpuValue() const -> u64;

        auto increment() -> u64 { return ++_value; }

        auto wait(u64 value, u64 timeout = 1000000000) -> std::expected<bool, Error>;

        auto signal(u64 value) -> std::expected<bool, Error>;

        struct Pair {
            Semaphore const* semaphore = nullptr;
            u64 value = 0;
        };
        auto getPair() const -> Pair { return { this, value() }; }

    private:
        friend Device;

        Device* _device = nullptr;
        VkSemaphore _semaphore = VK_NULL_HANDLE;
        u64 _value = 0;
        bool _isTimeline = false;

    };

}

#endif //CANTA_SEMAPHORE_H
