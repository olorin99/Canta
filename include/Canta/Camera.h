#ifndef CAMERA_H
#define CAMERA_H

#include <Ende/math/Quaternion.h>
#include <Ende/math/Mat.h>
#include <Ende/math/Frustum.h>

namespace canta {

    struct Frustum {
        ende::math::float4 planes[6] = {};
        ende::math::float4 corners[8] = {};
    };
    struct GPUCamera {
        ende::math::float4x4 projection;
        ende::math::float4x4 view;
        ende::math::float3 position;
        f32 near;
        f32 far;
        Frustum frustum;
    };

    class Camera {
    public:

        struct CreatePerspectiveInfo {
            ende::math::float3 position = { 0, 0, 0 };
            ende::math::Quaternion rotation = { 0, 0, 0, 1 };
            f32 fov = ende::math::rad(45);
            f32 width = 1;
            f32 height = 1;
            f32 near = 0.1;
            f32 far = 100;
        };
        static auto create(CreatePerspectiveInfo info) -> Camera;

        struct CreateOrthographicInfo {
            ende::math::float3 position = { 0, 0, 0 };
            ende::math::Quaternion rotation = { 0, 0, 0, 1 };
            f32 left = 0;
            f32 right = 0;
            f32 top = 0;
            f32 bottom = 0;
            f32 near = 0;
            f32 far = 0;
        };
        static auto create(CreateOrthographicInfo info) -> Camera;

        [[nodiscard]] auto view() const -> ende::math::float4x4;
        [[nodiscard]] auto projection() const -> const ende::math::float4x4& { return _projection; }
        [[nodiscard]] auto viewProjection() const -> ende::math::float4x4 { return projection() * view(); }

        void setProjection(const ende::math::float4x4& projection) { _projection = projection; }

        void setPosition(const ende::math::float3& pos) { _position = pos; }
        [[nodiscard]] auto position() const -> const ende::math::float3& { return _position; }

        void setRotation(const ende::math::Quaternion& rot) { _rotation = rot; }
        [[nodiscard]] auto rotation() const -> const ende::math::Quaternion& { return _rotation; }

        void setNear(f32 near);
        [[nodiscard]] auto near() const -> f32 { return _near; }

        void setFar(f32 far);
        [[nodiscard]] auto far() const -> f32 { return _far; }

        void setFov(f32 fov);
        [[nodiscard]] auto fov() const -> f32 { return _fov; }

        [[nodiscard]] auto gpuCamera() const -> GPUCamera;

        [[nodiscard]] auto frustum() const -> const ende::math::Frustum& { return _frustum; }
        [[nodiscard]] auto frustumCorners() const -> std::array<ende::math::float4, 8>;
        void updateFrustum() { _frustum.update(viewProjection()); }

        void lookAt(const ende::math::float3& dst);

    private:

        Camera() = default;

        ende::math::float4x4 _projection = {};
        ende::math::float3 _position = {};
        ende::math::Quaternion _rotation = {};
        f32 _fov = 0;
        f32 _width = 0;
        f32 _height = 0;
        f32 _near = 0;
        f32 _far = 0;

        ende::math::Frustum _frustum = {{}};

    };

}

#endif //CAMERA_H
