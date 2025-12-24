import canta;

[shader("compute")]
[numthreads(32, 32, 1)]
void main(
    uint3 threadId: SV_DispatchThreadID,
    uniform float* lhs,
    uniform float* rhs,
    uniform float* output
) {
    const uint idx = threadId.x;

    output[idx] = 0;
}