#ifndef CANTA_WINDOW_H
#define CANTA_WINDOW_H

#include <volk.h>
#include <Ende/math/Vec.h>
#include <vector>

namespace canta {

    class Device;

    class Window {
    public:

        virtual ~Window() = default;

        virtual auto requiredExtensions() -> std::vector<const char*> = 0;

        virtual auto surface(Device&) -> VkSurfaceKHR = 0;

        virtual auto extent() -> ende::math::Vec<2, u32> = 0;

    };

}

#endif //CANTA_WINDOW_H
