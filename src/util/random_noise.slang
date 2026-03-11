import canta;

[shader("compute")]
[numthreads(8, 8, 1)]
void main(
    uint3 threadId : SV_DispatchThreadID,
    uniform canta.RWImage2D<float4> image,
    uniform const uint seed
) {
    if (any(threadId.xy >= image.size()))
        return;

    image[threadId.xy] = canta.random.noise(uint3(threadId.xy, seed));
}