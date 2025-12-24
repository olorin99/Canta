module canta;

#define CANTA_BINDLESS_SAMPLERS 0
#define CANTA_BINDLESS_SAMPLED_IMAGES 1
#define CANTA_BINDLESS_STORAGE_IMAGES 2
#define CANTA_BINDLESS_STORAGE_BUFFERS 3

[[vk::binding(0, CANTA_BINDLESS_SAMPLERS)]] uniform SamplerState samplers[];

[[vk::binding(CANTA_BINDLESS_SAMPLED_IMAGES, 0)]] __DynamicResource g_sampledImages[];
[[vk::binding(CANTA_BINDLESS_STORAGE_IMAGES, 0)]] __DynamicResource g_storageImages[];

namespace canta {

public struct Image1D<T : ITexelElement> {

    private int index;

    public __init(uint imageIndex) {
        index = imageIndex;
    }

    public __subscript(uint pos) -> T {
        get { return Texture1D<T>(g_sampledImages[NonUniformResourceIndex(index)])[pos]; }
        set { static_assert(false, "attempted to write to read only texture"); }
    }

    public func isValid() -> bool {
        return index >= 0;
    }

    public func get() -> Texture1D<T> {
        return Texture1D<T>(g_sampledImages[NonUniformResourceIndex(index)]);
    }

    public func size() -> uint {
        uint value = 0;
        get().GetDimensions(value);
        return value;
    }
}

public struct RWImage1D<T : ITexelElement> {

    private int index;

    public __init(uint imageIndex) {
        index = imageIndex;
    }

    public __subscript(uint pos) -> T {
        get { return RWTexture1D<T>(g_storageImages[NonUniformResourceIndex(index)])[pos]; }
        set { RWTexture1D<T>(g_storageImages[NonUniformResourceIndex(index)])[pos] = newValue; }
    }

    public func isValid() -> bool {
        return index >= 0;
    }

    public func get() -> RWTexture1D<T> {
        return RWTexture1D<T>(g_storageImages[NonUniformResourceIndex(index)]);
    }

    public func size() -> uint {
        uint value = 0;
        get().GetDimensions(value);
        return value;
    }
}

public struct Image2D<T : ITexelElement> {

    private int index;

    public __init(uint imageIndex) {
        index = imageIndex;
    }

    public __subscript(uint2 pos) -> T {
        get { return Texture2D<T>(g_sampledImages[NonUniformResourceIndex(index)])[pos]; }
        set { static_assert(false, "attempted to write to read only texture"); }
    }

    public func isValid() -> bool {
        return index >= 0;
    }

    public func get() -> Texture2D<T> {
        return Texture2D<T>(g_sampledImages[NonUniformResourceIndex(index)]);
    }

    public func size() -> uint2 {
        uint2 value = 0;
        get().GetDimensions(value.x, value.y);
        return value;
    }
}

public struct RWImage2D<T : ITexelElement> {

    private int index;

    public __init(uint imageIndex) {
        index = imageIndex;
    }

    public __subscript(uint2 pos) -> T {
        get { return RWTexture2D<T>(g_storageImages[NonUniformResourceIndex(index)])[pos]; }
        set { RWTexture2D<T>(g_storageImages[NonUniformResourceIndex(index)])[pos] = newValue; }
    }

    public func isValid() -> bool {
        return index >= 0;
    }

    public func get() -> RWTexture2D<T> {
        return RWTexture2D<T>(g_storageImages[NonUniformResourceIndex(index)]);
    }

    public func size() -> uint2 {
        uint2 value = 0;
        get().GetDimensions(value.x, value.y);
        return value;
    }
}

public struct Image3D<T : ITexelElement> {

    private int index;

    public __init(uint imageIndex) {
        index = imageIndex;
    }

    public __subscript(uint3 pos) -> T {
        get { return Texture3D<T>(g_sampledImages[NonUniformResourceIndex(index)])[pos]; }
        set { static_assert(false, "attempted to write to read only texture"); }
    }

    public func isValid() -> bool {
        return index >= 0;
    }

    public func get() -> Texture3D<T> {
        return Texture3D<T>(g_sampledImages[NonUniformResourceIndex(index)]);
    }

    public func size() -> uint3 {
        uint3 value = 0;
        get().GetDimensions(value.x, value.y, value.z);
        return value;
    }
}

public struct RWImage3D<T : ITexelElement> {

    private int index;

    public __init(uint imageIndex) {
        index = imageIndex;
    }

    public __subscript(uint3 pos) -> T {
        get { return RWTexture3D<T>(g_storageImages[NonUniformResourceIndex(index)])[pos]; }
        set { RWTexture3D<T>(g_storageImages[NonUniformResourceIndex(index)])[pos] = newValue; }
    }

    public func isValid() -> bool {
        return index >= 0;
    }

    public func get() -> RWTexture3D<T> {
        return RWTexture3D<T>(g_storageImages[NonUniformResourceIndex(index)]);
    }

    public func size() -> uint3 {
        uint3 value = 0;
        get().GetDimensions(value.x, value.y, value.z);
        return value;
    }
}

public struct Frustum {
    private float4 planes[6];
    private float4 corners[8];

    public func check(float3 pos, float radius) -> bool {
        for (int i = 0; i < 6; i++) {
            if (dot(float4(pos, 1.0), planes[i]) + radius < 0.0)
                return false;
        }
        return true;
    }
};

public struct Camera {
    public float4x4 projection;
    public float4x4 view;
    public float3 position;
    public float near;
    public float far;
    public Frustum frustum;
};

public float rand(float seed) {
    float noise = fract(sin(dot(float2(seed), float2(12.9898,78.233)*2.0)) * 43758.5453);
    return abs(noise.x) * 0.5;
}

public float rand(float2 uv) {
    float2 noise = (fract(float2(sin(dot(uv, float2(12.9898,78.233)*2.0)) * 43758.5453)));
    return abs(noise.x + noise.y) * 0.5;
}

public float2 rand2(float2 uv) {
    float noiseX = (fract(sin(dot(uv, float2(12.9898,78.233) * 2.0)) * 43758.5453));
    float noiseY = sqrt(1 - noiseX * noiseX);
    return float2(noiseX, noiseY);
}

public float3 rand3(uint3 v) {
	v = v * 1664525u + 1013904223u;
	v.x += v.y * v.z;
	v.y += v.z * v.x;
	v.z += v.x * v.y;
	v ^= v >> 16u;
	v.x += v.y * v.z;
	v.y += v.z * v.x;
	v.z += v.x * v.y;
	return float3(v) * (1.0 / float(0xffffffffu));
}

}

// matrix and vec multiplication overload
public float2 operator*(float2x2 matrix, float2 vec) {
    return mul(matrix, vec);
}

public float2 operator*(float2 vec, float2x2 matrix) {
    return mul(vec, matrix);
}

public float2x2 operator*(float2x2 lhs, float2x2 rhs) {
    return mul(lhs, rhs);
}

public float3 operator*(float3x3 matrix, float3 vec) {
    return mul(matrix, vec);
}

public float3 operator*(float3 vec, float3x3 matrix) {
    return mul(vec, matrix);
}

public float3x3 operator*(float3x3 lhs, float3x3 rhs) {
    return mul(lhs, rhs);
}

public float4 operator*(float4x4 matrix, float4 vec) {
    return mul(matrix, vec);
}

public float4 operator*(float4 vec, float4x4 matrix) {
    return mul(vec, matrix);
}

public float4x4 operator*(float4x4 lhs, float4x4 rhs) {
    return mul(lhs, rhs);
}