#ifndef CANTA_WINDOW_H
#define CANTA_WINDOW_H

#include <Ende/math/Mat.h>
#include <vector>
#include <volk.h>

namespace canta {

class Device;

class Window {
  public:
    virtual ~Window() = default;

    virtual auto requiredExtensions() -> std::vector<const char *> = 0;

    virtual auto surface(Device &) -> VkSurfaceKHR = 0;

    virtual auto extent() -> ende::math::uint2 = 0;
};

} // namespace canta

#endif // CANTA_WINDOW_H
