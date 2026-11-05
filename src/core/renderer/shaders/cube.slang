struct VSInput {
    float2 position;
    float3 color;
};

struct UniformBuffer {
    float4x4 projview;
};
[[vk::binding(0, 0)]] ConstantBuffer<UniformBuffer> ubo;

struct Push
{
    float4x4 model;
};

[[vk::push_constant]] Push pushconstant;

struct VertexOutput {
    float4 position : SV_Position;
    float3 color;
};

[shader("vertex")]
VertexOutput vertMain(VSInput input)
{
    VertexOutput output;
    output.position = float4(input.position, 0.0, 1.0);
    output.color = input.color;
    return output;
}

[shader("fragment")]
float4 fragMain(VertexOutput invert)
{
    return float4(invert.color, 1.0);
}
