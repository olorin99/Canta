#include "Ende/math/Quaternion.h"
#include <Canta/Camera.h>

template <typename T>
constexpr inline ende::math::Mat<4, T> perspective(T fov, T aspect, T near, T far) {
    const f32 tanHalfFov = std::tan(fov / 2.f);

    ende::math::Mat<4, T> result;
    result(0, 0) = 1.f / (tanHalfFov * aspect);
    result(0, 1) = 0.f;
    result(0, 2) = 0.f;
    result(0, 3) = 0.f;

    result(1, 0) = 0.f;
    result(1, 1) = 1.f / tanHalfFov;
    result(1, 2) = 0.f;
    result(1, 3) = 0.f;

    result(2, 0) = 0.f;
    result(2, 1) = 0.f;
    result(2, 2) = 0.f;
    result(2, 3) = T(1);

    result(3, 0) = 0.f;
    result(3, 1) = 0.f;
    result(3, 2) = near;
    result(3, 3) = 0.f;

    return result;
}

auto canta::Camera::create(canta::Camera::CreatePerspectiveInfo info) -> Camera {
    Camera camera = {};

    camera._projection = perspective(info.fov, info.width / info.height, info.near, info.far);
    camera._position = info.position;
    camera._rotation = info.rotation;
    camera._fov = info.fov;
    camera._width = info.width;
    camera._height = info.height;
    camera._near = info.near;
    camera._far = info.far;

    return camera;
}

auto canta::Camera::create(canta::Camera::CreateOrthographicInfo info) -> Camera {
    Camera camera = {};

    camera._projection = ende::math::orthographic(info.left, info.right, info.bottom, info.top, info.near, info.far);
    camera._position = info.position;
    camera._rotation = info.rotation;
    camera._near = info.near;
    camera._far = info.far;

    return camera;
}

auto canta::Camera::view() const -> ende::math::Mat4f {
    auto translation = ende::math::translation<4, f32>(_position);
    auto rotation = _rotation.inverse().toMat();
    return translation * rotation;
}

void canta::Camera::setNear(f32 near) {
    _near = near;
    _projection = perspective(_fov, _width / _height, _near, _far);
}

void canta::Camera::setFar(f32 far) {
    _far = far;
    _projection = perspective(_fov, _width / _height, _near, _far);
}

void canta::Camera::setFov(f32 fov) {
    _fov = fov;
    _projection = perspective(_fov, _width / _height, _near, _far);
}

auto canta::Camera::gpuCamera() const -> GPUCamera {
    auto planes = _frustum.planes();
    auto corners = frustumCorners();
    canta::Frustum frustum = {};
    for (u32 i = 0; i < 6; i++)
        frustum.planes[i] = planes[i];
    for (u32 i = 0; i < 8; i++)
        frustum.corners[i] = corners[i];

    return {
        .projection = projection(),
        .view = view(),
        .position = _position,
        .near = _near,
        .far = _far,
        .frustum = frustum
    };
}

auto canta::Camera::frustumCorners() const -> std::array<ende::math::Vec4f, 8> {
    const auto inverseViewProjection = (ende::math::perspective(_fov, _width / _height, _near, _far) * view()).inverse();
    std::array<ende::math::Vec4f, 8> corners = {};
    constexpr std::array offsets = {
            ende::math::Vec<2, f32>{-1,  1}, ende::math::Vec<2, f32>{-1, -1}, ende::math::Vec<2, f32>{1, -1}, ende::math::Vec<2, f32>{1,  1},
            ende::math::Vec<2, f32>{1,  1}, ende::math::Vec<2, f32>{-1,  1}, ende::math::Vec<2, f32>{-1, -1}, ende::math::Vec<2, f32>{1, -1}
    };
    for (u32 i = 0; i < 8; i++) {
        const auto pt = inverseViewProjection.transform(ende::math::Vec4f{
            offsets[i].x(),
            offsets[i].y(),
            i > 4 ? 0.f : 1.f,
            1
        });
        corners[i] = pt / pt.w();
    }
    return corners;
}

void canta::Camera::lookAt(const ende::math::Vec3f& dst) {
    const auto up = ende::math::Vec3f({0, 1, 0});

    const auto direction = dst - _position;
    const auto right = up.cross(direction * -1);
    const auto s = 1.f / std::sqrt(std::max(0.00001f, right.dot(right)));
    const auto b = right * s;
    const auto c = direction.cross(b);

    auto mat = ende::math::identity<4, f32>();
    mat(0, 0) = b[0];
    mat(0, 1) = b[1];
    mat(0, 2) = b[2];

    mat(1, 0) = c[0];
    mat(1, 1) = c[1];
    mat(1, 2) = c[2];

    mat(2, 0) = direction[0];
    mat(2, 1) = direction[1];
    mat(2, 2) = direction[2];

    const auto q = ende::math::Quaternion(mat);

    _rotation = q;
}