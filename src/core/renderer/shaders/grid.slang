#include "camera.slang"
struct VertexOutput {
    float4 position : SV_Position;
    float3 nearpoint : NEAR_POINT;
    float3 farpoint : FAR_POINT;
};

struct Grid
{
    float4 minorcolor;
    float4 majorcolor;
    float4 cell; //x = size, y = major every, z = fade start, w = fade end
};
[[vk::binding(0, 0)]] ConstantBuffer<Camera> m_camera;
[[vk::push_constant]] ConstantBuffer<Grid> m_grid;

float3 unproject(float2 xy, float z)
{
    float4 p = mul(m_camera.invviewproj, float4(xy, z, 1.0));
    return p.xyz / p.w;
}

[shader("vertex")]
VertexOutput vertMain(uint vertexID : SV_VertexID)
{
    VertexOutput output;
    float2 uv = float2(float((vertexID << 1) & 2), float(vertexID & 2));
    float2 ndc = uv * 2.0 - 1.0;

    output.nearpoint = unproject(ndc, 0.0);
    output.farpoint = unproject(ndc, 1.0);
    output.position = float4(ndc, 0.0, 1.0);
    return output;
}

//fragment stuff
struct FragOutput
{
    float4 color : SV_Target0;
    float depth : SV_Depth;
};

float gridline(float2 p, float cell)
{
    float2 coord = p / cell;
    float2 fw = fwidth(coord);
    float2 distancetoline = abs(frac(coord - 0.5) - 0.5) / fw;
    return 1.0 - min(min(distancetoline.x, distancetoline.y), 1.0);
}

[shader("fragment")]
FragOutput fragMain(VertexOutput invert)
{
    float3 nearpoint = invert.nearpoint;
    float3 farpoint = invert.farpoint;

    float t = -nearpoint.y / (farpoint.y - nearpoint.y);
    float3 p = nearpoint + t * (farpoint - nearpoint);

    float minorline = gridline(p.xz, m_grid.cell.x);
    float majorline = gridline(p.xz, m_grid.cell.x * m_grid.cell.y);
    float axisX = 1.0 - min(abs(p.z) / fwidth(p.z), 1.0);
    float axisZ = 1.0 - min(abs(p.x) / fwidth(p.x), 1.0);

    if (!(t > 0.0)) {
        discard;
    }
    float3 rgb = lerp(m_grid.minorcolor.rgb, m_grid.majorcolor.rgb, majorline);
    float a = max(minorline * m_grid.minorcolor.a, majorline * m_grid.majorcolor.a);
    rgb = lerp(rgb, float3(0.90, 0.25, 0.25), axisX);
    rgb = lerp(rgb, float3(0.25, 0.45, 0.95), axisZ);
    a = max(a, max(axisX, axisZ));

    float dist = length(p - m_camera.position.xyz);
    a *= 1.0 - smoothstep(m_grid.cell.z, m_grid.cell.w, dist);

    if (a < 0.002) {
        discard;
    }
    float4 clip = mul(m_camera.viewproj, float4(p, 1.0));

    FragOutput output;
    output.color = float4(rgb, a);
    output.depth = clip.z / clip.w;
    return output;
}
