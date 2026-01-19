struct Material {
    uint albedoIndex;
    uint normalIndex;
    uint emissiveIndex;
    float emissiveStrength;
}

[shader("compute")]
[numthreads(1,1,1)]
void main(uint3 threadId : SV_DispatchThreadID,
    uniform Material* materials,
    uniform uint padding,
    uniform uint padding1,
)
{
    uint index = threadId.x;
    materials[index].albedoIndex = padding;
}