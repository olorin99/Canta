
#define CANTA_BINDLESS_SAMPLERS 0
#define CANTA_BINDLESS_SAMPLED_IMAGES 1
#define CANTA_BINDLESS_STORAGE_IMAGES 2
#define CANTA_BINDLESS_STORAGE_BUFFERS 3

[[vk::binding(0, CANTA_BINDLESS_SAMPLERS)]] uniform SamplerState samplers[];

[[vk::binding(CANTA_BINDLESS_SAMPLED_IMAGES, 0)]] __DynamicResource g_sampledImages[];
[[vk::binding(CANTA_BINDLESS_STORAGE_IMAGES, 0)]] __DynamicResource g_storageImages[];

struct Image1D<T : ITexelElement> {

    int index;

    __init(uint imageIndex) {
        index = imageIndex;
    }

    __subscript(uint pos) -> T {
        get { return Texture1D<T>(g_sampledImages[NonUniformResourceIndex(index)])[pos]; }
        set { static_assert(false, "attempted to write to read only texture"); }
    }

    func isValid() -> bool {
        return index >= 0;
    }

    func get() -> Texture1D<T> {
        return Texture1D<T>(g_sampledImages[NonUniformResourceIndex(index)]);
    }

    func size() -> uint {
        uint value = 0;
        get().GetDimensions(value);
        return value;
    }
}

struct RWImage1D<T : ITexelElement> {

    int index;

    __init(uint imageIndex) {
        index = imageIndex;
    }

    __subscript(uint pos) -> T {
        get { return RWTexture1D<T>(g_storageImages[NonUniformResourceIndex(index)])[pos]; }
        set { RWTexture1D<T>(g_storageImages[NonUniformResourceIndex(index)])[pos] = newValue; }
    }

    func isValid() -> bool {
        return index >= 0;
    }

    func get() -> RWTexture1D<T> {
        return RWTexture1D<T>(g_storageImages[NonUniformResourceIndex(index)]);
    }

    func size() -> uint {
        uint value = 0;
        get().GetDimensions(value);
        return value;
    }
}

struct Image2D<T : ITexelElement> {

    int index;

    __init(uint imageIndex) {
        index = imageIndex;
    }

    __subscript(uint2 pos) -> T {
        get { return Texture2D<T>(g_sampledImages[NonUniformResourceIndex(index)])[pos]; }
        set { static_assert(false, "attempted to write to read only texture"); }
    }

    func isValid() -> bool {
        return index >= 0;
    }

    func get() -> Texture2D<T> {
        return Texture2D<T>(g_sampledImages[NonUniformResourceIndex(index)]);
    }

    func size() -> uint2 {
        uint2 value = 0;
        get().GetDimensions(value.x, value.y);
        return value;
    }
}

struct RWImage2D<T : ITexelElement> {

    int index;

    __init(uint imageIndex) {
        index = imageIndex;
    }

    __subscript(uint2 pos) -> T {
        get { return RWTexture2D<T>(g_storageImages[NonUniformResourceIndex(index)])[pos]; }
        set { RWTexture2D<T>(g_storageImages[NonUniformResourceIndex(index)])[pos] = newValue; }
    }

    func isValid() -> bool {
        return index >= 0;
    }

    func get() -> RWTexture2D<T> {
        return RWTexture2D<T>(g_storageImages[NonUniformResourceIndex(index)]);
    }

    func size() -> uint2 {
        uint2 value = 0;
        get().GetDimensions(value.x, value.y);
        return value;
    }
}

struct Image3D<T : ITexelElement> {

    int index;

    __init(uint imageIndex) {
        index = imageIndex;
    }

    __subscript(uint3 pos) -> T {
        get { return Texture3D<T>(g_sampledImages[NonUniformResourceIndex(index)])[pos]; }
        set { static_assert(false, "attempted to write to read only texture"); }
    }

    func isValid() -> bool {
        return index >= 0;
    }

    func get() -> Texture3D<T> {
        return Texture3D<T>(g_sampledImages[NonUniformResourceIndex(index)]);
    }

    func size() -> uint3 {
        uint3 value = 0;
        get().GetDimensions(value.x, value.y, value.z);
        return value;
    }
}

struct RWImage3D<T : ITexelElement> {

    int index;

    __init(uint imageIndex) {
        index = imageIndex;
    }

    __subscript(uint3 pos) -> T {
        get { return RWTexture3D<T>(g_storageImages[NonUniformResourceIndex(index)])[pos]; }
        set { RWTexture3D<T>(g_storageImages[NonUniformResourceIndex(index)])[pos] = newValue; }
    }

    func isValid() -> bool {
        return index >= 0;
    }

    func get() -> RWTexture3D<T> {
        return RWTexture3D<T>(g_storageImages[NonUniformResourceIndex(index)]);
    }

    func size() -> uint3 {
        uint3 value = 0;
        get().GetDimensions(value.x, value.y, value.z);
        return value;
    }
}

// matrix and vec multiplication overload
float2 operator*(float2x2 matrix, float2 vec) {
    return mul(matrix, vec);
}

float2 operator*(float2 vec, float2x2 matrix) {
    return mul(vec, matrix);
}

float2x2 operator*(float2x2 lhs, float2x2 rhs) {
    return mul(lhs, rhs);
}

float3 operator*(float3x3 matrix, float3 vec) {
    return mul(matrix, vec);
}

float3 operator*(float3 vec, float3x3 matrix) {
    return mul(vec, matrix);
}

float3x3 operator*(float3x3 lhs, float3x3 rhs) {
    return mul(lhs, rhs);
}

float4 operator*(float4x4 matrix, float4 vec) {
    return mul(matrix, vec);
}

float4 operator*(float4 vec, float4x4 matrix) {
    return mul(vec, matrix);
}

float4x4 operator*(float4x4 lhs, float4x4 rhs) {
    return mul(lhs, rhs);
}
