import canta;

#define BLOCK_SIZE 32
[vk::constant_id(0)] const uint x_size = BLOCK_SIZE;
[vk::constant_id(1)] const uint y_size = 1;
[vk::constant_id(2)] const uint z_size = 1;


[shader("compute")]
[numthreads(x_size, y_size, z_size)]
void main(
    uint3 threadId : SV_DispatchThreadID,
    uniform float* buffer,
    uniform const uint count,
    uniform const uint offset,
    uniform const float scale,
    uniform const float scaleOffset,
    uniform const float seed,
) {
    if (threadId.x > count)
        return;

    let value = canta.rand(float2(seed, threadId.x));

    buffer[offset + threadId.x] = value * scale + scaleOffset;
}