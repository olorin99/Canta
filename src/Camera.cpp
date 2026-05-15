#include "Ende/math/Mat.h"
#include "Ende/math/Quaternion.h"
#include <Canta/Camera.h>

template <typename T>
constexpr inline ende::math::Matrix<T, 4> perspective(T fov, T aspect, T near, T far) {
    const f32 tanHalfFov = std::tan(fov / 2.f);

    ende::math::Matrix<T, 4> result;
    result[0][0] = 1.f / (tanHalfFov * aspect);
    result[1][1] = 1.f / tanHalfFov;
    result[3][2] = T(1);
    result[2][3] = near;

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

auto canta::Camera::view() const -> ende::math::float4x4 {
    auto translation = ende::math::translation<f32, 4>(_position);
    auto rotation = _rotation.toMat();
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
        .frustum = frustum};
}

auto canta::Camera::frustumCorners() const -> std::array<ende::math::float4, 8> {
    const auto inverseViewProjection = (ende::math::perspective(_fov, _width / _height, _near, _far) * view()).inverse();
    std::array<ende::math::float4, 8> corners = {};
    std::array offsets = {
        ende::math::float2(-1, 1), ende::math::float2(-1, -1), ende::math::float2(1, -1), ende::math::float2(1, 1),
        ende::math::float2(1, 1), ende::math::float2(-1, 1), ende::math::float2(-1, -1), ende::math::float2(1, -1)};
    for (u32 i = 0; i < 8; i++) {
        const auto pt = inverseViewProjection * (ende::math::float4{
                                                    offsets[i].x(),
                                                    offsets[i].y(),
                                                    i > 4 ? 0.f : 1.f,
                                                    1});
        corners[i] = pt / pt.w();
    }
    return corners;
}

void canta::Camera::lookAt(const ende::math::float3 &dst) {
    const auto up = ende::math::float3({0, 1, 0});

    const auto forwards = (dst - _position).unit();
    const auto right = up.cross(forwards * -1).unit();
    const auto upwards = forwards.cross(right).unit();

    const auto mat = ende::math::lookAt(_position, dst, up);

    const auto q = ende::math::Quaternion(mat);

    _rotation = q;
}
