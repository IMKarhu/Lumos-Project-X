struct Camera
{
    float4x4 view;
    float4x4 proj;
    float4x4 viewproj;
    float4x4 invviewproj;
    float4 position;
};
