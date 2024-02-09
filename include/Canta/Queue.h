#ifndef CANTA_QUEUE_H
#define CANTA_QUEUE_H

#include <Ende/platform.h>
#include <volk.h>

namespace canta {

    class Device;

    class Queue {
    public:

        auto queue() const -> VkQueue { return _queue; }
        auto familyIndex() const -> u32 { return _familyIndex; }
        auto queueIndex() const -> u32 { return _queueIndex; }

    private:
        friend Device;

        Queue() = default;

        Device* _device = nullptr;
        VkQueue _queue = VK_NULL_HANDLE;
        u32 _familyIndex = 0;
        u32 _queueIndex = 0;

    };

}

#endif //CANTA_QUEUE_H
