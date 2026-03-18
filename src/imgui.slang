import canta;

struct VSOutput {
    float4 position : SV_Position;
    float2 uv : TEXCOORD;
    float4 colour: COLOUR;
};

[shader("vertex")]
VSOutput vertex(
    float2 pos : POSITION,
    float2 uv : TEXCOORD,
    float4 colour,
    uniform float2 scale,
    uniform float2 translate,
) {
    VSOutput out;
    out.position = float4(pos * scale + translate, 0, 1);
    out.uv = uv;
    out.colour = colour;
    return out;
}

[shader("fragment")]
float4 fragment(
    in VSOutput input,
    uniform float2 scale,
    uniform float2 translate,
    uniform canta.Sampler sampler,
    uniform canta.Image2D<float4> image,
) {
    float4 colour = image.get().Sample(sampler.get(), input.uv);
    return input.colour * colour;
}