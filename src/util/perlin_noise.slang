import canta;

uint hash(uint3 key, uint seed) {
    return canta.random.rand(uint4(key, seed));
}

// from https://www.shadertoy.com/view/slB3z3

float3 gradient(uint h) {
    const float3 gradients[12] = float3[12](
        float3(1, 1, 0), float3(-1, 1, 0), float3(1, -1, 0), float3(-1, -1, 0),
        float3(1, 0, 1), float3(-1, 0, 1), float3(1, 0, -1), float3(-1, 0, -1),
        float3(0, 1, 1), float3(0, -1, 1), float3(0, 1, -1), float3(0, -1, -1)
    );
    return gradients[int(h % 12u)];
}

float interpolate(float value1, float value2, float value3, float value4, float value5, float value6, float value7, float value8, float3 t) {
    return lerp(
        lerp(lerp(value1, value2, t.x), lerp(value3, value4, t.x), t.y),
        lerp(lerp(value5, value6, t.x), lerp(value7, value8, t.x), t.y),
        t.z
    );
}

float3 fade(float3 t) {
    // 6t^5 - 15t^4 + 10t^3
	return t * t * t * (t * (t * 6.0 - 15.0) + 10.0);
}

float perlinNoise(float3 position, uint seed) {
    float3 floorPosition = floor(position);
    float3 fractPosition = position - floorPosition;
    uint3 cellCoordinates = uint3(int3(floorPosition));
    float value1 = dot(gradient(hash(cellCoordinates, seed)), fractPosition);
    float value2 = dot(gradient(hash(cellCoordinates + uint3(1, 0, 0), seed)), fractPosition - float3(1, 0, 0));
    float value3 = dot(gradient(hash(cellCoordinates + uint3(0, 1, 0), seed)), fractPosition - float3(0, 1, 0));
    float value4 = dot(gradient(hash(cellCoordinates + uint3(1, 1, 0), seed)), fractPosition - float3(1, 1, 0));
    float value5 = dot(gradient(hash(cellCoordinates + uint3(0, 0, 1), seed)), fractPosition - float3(0, 0, 1));
    float value6 = dot(gradient(hash(cellCoordinates + uint3(1, 0, 1), seed)), fractPosition - float3(1, 0, 1));
    float value7 = dot(gradient(hash(cellCoordinates + uint3(0, 1, 1), seed)), fractPosition - float3(0, 1, 1));
    float value8 = dot(gradient(hash(cellCoordinates + uint3(1, 1, 1), seed)), fractPosition - float3(1, 1, 1));
    return interpolate(value1, value2, value3, value4, value5, value6, value7, value8, fade(fractPosition));
}

float perlinNoise(float3 position, int octaveCount, float persistence, float lacunarity, uint seed) {
    float value = 0.0;
    float amplitude = 1.0;
    for (int i = 0; i < octaveCount; i++) {
        uint s = hash(uint(i), seed);
        value += perlinNoise(position, s) * amplitude;
        amplitude *= persistence;
        position *= lacunarity;
    }
    return value;
}


[shader("compute")]
[numthreads(8, 8, 1)]
void main(
    uint3 threadId : SV_DispatchThreadID,
    uniform canta.RWImage2D<float4> image,
    uniform const float time,
    uniform const uint octaves,
    uniform const float persistence,
    uniform const float lacunarity,
    uniform const uint seed,
) {
    if (any(threadId.xy >= image.size()))
        return;

    let coord = (float2(threadId.xy) / image.size());

    let n = perlinNoise(float3(coord, time), octaves, persistence, lacunarity, seed);
    let colour = float3((n + 1) * 0.5);

    image[threadId.xy] = float4(colour, 1.0);
}