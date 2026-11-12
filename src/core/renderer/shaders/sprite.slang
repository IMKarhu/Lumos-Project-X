static const float2 positions[6] = {
    float2(-0.5, -0.5),
    float2( 0.5, -0.5),
    float2( 0.5,  0.5),
    float2(-0.5, -0.5),
    float2( 0.5,  0.5),
    float2(-0.5,  0.5)
};

static const float2 uvs[6] = {
    float2(0.0, 0.0),
    float2(1.0, 0.0),
    float2(1.0, 1.0),
    float2(0.0, 0.0),
    float2(1.0, 1.0),
    float2(0.0, 1.0)
};

struct UniformBuffer {
    float4x4 projview;
};
[[vk::binding(0, 0)]] ConstantBuffer<UniformBuffer> ubo;

struct GpuEntity {
    float4 positionAndsize;
    float4 rotationdata;
    float4 color;
    float4 uvrect;
};
[[vk::binding(0, 1)]] StructuredBuffer<GpuEntity> entities;

struct VertexOutput {
    float4 position : SV_Position;
    float4 color : COLOR0;
    float2 uv : TEXCOORD0;
};

[shader("vertex")]
VertexOutput vertMain(uint vertexID : SV_VertexID, uint instanceID : SV_InstanceID) {
    VertexOutput output;
    GpuEntity entity = entities[instanceID];
    float2 localpos = positions[vertexID] * entity.positionAndsize.zw;
    float sinrotation = sin(entity.rotationdata.x);
    float cosrotation = cos(entity.rotationdata.x);

    float2 rotatedposition = float2(
        localpos.x * cosrotation - localpos.y * sinrotation,
        localpos.x * sinrotation + localpos.y * cosrotation
    );
    float2 worldpos = rotatedposition + entity.positionAndsize.xy;
    output.position = mul(ubo.projview, float4(worldpos, 0.0, 1.0));
    // output.position = float4(positions[vertexID], 0.0, 1.0);
    output.color = entity.color;
    output.uv = entity.uvrect.xy + uvs[vertexID] * entity.uvrect.zw;
    return output;
}

[[vk::binding(0, 2)]] Texture2D<float4> texture;
[[vk::binding(1, 2)]] SamplerState texturesampler;

[shader("fragment")]
float4 fragMain(VertexOutput inVert) : SV_TARGET {
    float4 tex = texture.Sample(texturesampler, inVert.uv);
    return tex * inVert.color;
}
