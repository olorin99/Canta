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

    let hash = canta.random.rand(threadId.xy + uint2(seed));

    let r = canta.random.rand(float4(threadId.xy, hash, canta.random.rand(hash)));
    let g = canta.random.rand(float4(hash, threadId.xy, canta.random.rand(hash)));
    let b = canta.random.rand(float4(canta.random.rand(hash), threadId.xy, hash));
    let a = canta.random.rand(float4(hash, canta.random.rand(hash), threadId.yx));

    image[threadId.xy] = float4(r, g, b, a);
}