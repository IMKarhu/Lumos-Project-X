static const float2 quadPositions[6] = {
    float2(-0.5, -0.5),
    float2( 0.5, -0.5),
    float2( 0.5,  0.5),
    float2( 0.5,  0.5),
    float2(-0.5,  0.5),
    float2(-0.5, -0.5)
};
static const float2 quadUvs[6] = {
    float2(0, 1),
    float2(1, 1),
    float2(1, 0),

    float2(1, 0),
    float2(0, 0),
    float2(0, 1)
};

struct Sprite {
    float2 position;
    float2 padding;
    float2 size;
    float4 uvrect;
    float rotation;
};

struct SpriteBuffer
{
    Sprite sprite;
};
[[vk::binding(1, 1)]]StructuredBuffer<Sprite> sprites;

struct UniformBuffer {
    float4x4 projview;
};
[[vk::binding(0, 0)]] ConstantBuffer<UniformBuffer> ubo;

struct VertexOutput {
    float4 position : SV_Position;
    float2 texcoord;
};

struct Push
{
    float4x4 model;
};

[[vk::push_constant]] Push pushconstant;

[shader("vertex")]
VertexOutput vertMain(uint vertexID : SV_VertexID) {
    VertexOutput output;
    uint spriteindex = vertexID / 6;
    uint quadvertex = vertexID % 6;
    Sprite sprite = sprites[spriteindex];
    float2 localposition = quadPositions[quadvertex] * sprite.size;
    float c = cos(sprite.rotation);
    float s = sin(sprite.rotation);
    float2 rotatedposition = float2(c * localposition.x - s * localposition.y,
                                    s * localposition.x + c * localposition.y);
    float2 worldposition = rotatedposition + sprite.position;
    output.position = mul(ubo.projview, float4(worldposition, 0.0, 1.0));
    float2 uvmin = sprite.uvrect.xy;
    float2 uvmax = sprite.uvrect.zw;
    output.texcoord = lerp(uvmin, uvmax, quadUvs[quadvertex]);
    return output;
}

[[vk::binding(0, 1)]] Sampler2D texture;

[shader("fragment")]
float4 fragMain(VertexOutput inVert) : SV_TARGET {
    float2 texcoord = inVert.texcoord;
    return texture.Sample(texcoord);
}
