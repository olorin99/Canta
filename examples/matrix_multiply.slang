import canta;

static const uint2 blockDim = uint2(32, 32);

[shader("compute")]
[numthreads(32, 32, 1)]
void main(
    uint3 threadId: SV_DispatchThreadID,
    uniform const float* lhs,
    uniform const float* rhs,
    uniform float* output,
    uniform const uint N,
) {
    const uint x = threadId.x;
    const uint y = threadId.y;

    if (x > N || y > N) return;

    float tmp = 0.0;
    for (int i = 0; i < N; ++i) {
        tmp += lhs[y * N + i] * rhs[x + i * N];
    }

    output[y * N + x] = tmp;
}