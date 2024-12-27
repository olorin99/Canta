// hello-world.slang
//StructuredBuffer<float> buffer0;
//StructuredBuffer<float> buffer1;
//RWStructuredBuffer<float> result;

struct Material {
    uint albedoIndex;
    uint normalIndex;
    uint emissiveIndex;
    float emissiveStrength;
}

[shader("compute")]
[numthreads(1,1,1)]
void main(uint3 threadId : SV_DispatchThreadID,
    [vk::push_constant] uniform Material* materials,
    [vk::push_constant] uniform uint padding,
    [vk::push_constant] uniform uint padding1,
)
{
    uint index = threadId.x;
    materials[index].albedoIndex = padding;
}