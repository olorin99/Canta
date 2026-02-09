import canta;

#define BLOCK_SIZE 32
[vk::constant_id(0)] const uint x_size = BLOCK_SIZE;
[vk::constant_id(1)] const uint y_size = 1;
[vk::constant_id(2)] const uint z_size = 1;


[shader("compute")]
[numthreads(x_size, y_size, z_size)]
void generateUints(
    uint3 threadId : SV_DispatchThreadID,
    uniform uint* buffer,
    uniform const uint count,
    uniform const uint offset,
    uniform const uint max,
    uniform const uint min,
    uniform const uint seed,
) {
    if (threadId.x > count)
        return;

    let value = uint(canta.random.rand(seed + threadId.x, min, max));

    buffer[offset + threadId.x] = value;
}

[shader("compute")]
[numthreads(x_size, y_size, z_size)]
void generateFloats(
    uint3 threadId : SV_DispatchThreadID,
    uniform float* buffer,
    uniform const uint count,
    uniform const uint offset,
    uniform const float max,
    uniform const float min,
    uniform const uint seed,
) {
    if (threadId.x > count)
        return;

    let value = canta.random.rand(seed + threadId.x, min, max);

    buffer[offset + threadId.x] = value;
}