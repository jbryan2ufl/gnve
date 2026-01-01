struct VSInput {
    float3 inPosition;
    float2 inTexCoord;
};

struct UniformBuffer {
    float4x4 model;
    float4x4 view;
    float4x4 proj;
};

[[vk::binding(0, 0)]]
ConstantBuffer<UniformBuffer> ubo;

struct VSOutput {
    float4 pos : SV_Position;
    float2 fragTexCoord;
};

[shader("vertex")]
VSOutput vertMain(VSInput input)
{
    VSOutput output;
    output.pos = mul(ubo.proj, mul(ubo.view, mul(ubo.model, float4(input.inPosition, 1.0))));
    output.fragTexCoord = input.inTexCoord;
    return output;
}

[[vk::binding(1, 0)]]
Sampler2D textures[];

struct Push {
    uint texIndex;
}

[[vk::push_constant]]
ConstantBuffer<Push> push;

[shader("fragment")]
// float4 fragMain(VSOutput vertIn) : SV_TARGET { return float4(1.0f, 1.0f, 1.0f, 1.0f); }
float4 fragMain(VSOutput vertIn) : SV_TARGET { return textures[int(push.texIndex)].Sample(vertIn.fragTexCoord); }
