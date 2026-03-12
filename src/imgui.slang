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

struct Push {
    [[vk::offset(16)]] canta.Sampler sampler;
    [[vk::offset(20)]] canta.Image2D<float4> image;
}

[[vk::push_constant]]
Push push;

[shader("fragment")]
float4 fragment(
    in VSOutput input,
) {
    float4 colour = push.image.get().Sample(push.sampler.get(), input.uv);
    return input.colour * colour;
}