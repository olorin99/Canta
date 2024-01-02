#ifndef CANTA_SEMAPHORE_H
#define CANTA_SEMAPHORE_H

#include <Ende/platform.h>
#include <volk.h>

namespace canta {

    class Device;

    class Semaphore {
    public:

        struct CreateInfo {
            i64 initialValue = -1;
        };

        ~Semaphore();

        Semaphore(Semaphore&& rhs) noexcept;
        auto operator=(Semaphore&& rhs) noexcept -> Semaphore&;

        auto semaphore() const -> VkSemaphore { return _semaphore; }

    private:
        friend Device;

        Semaphore() = default;

        Device* _device = nullptr;
        VkSemaphore _semaphore = VK_NULL_HANDLE;
        u64 _value = 0;
        bool _isTimeline = false;

    };

}

#endif //CANTA_SEMAPHORE_H
