
Texture2D txDiffuse : register(t0);
SamplerState samplerPoint : register(s0);

cbuffer cbChangesEveryFrame : register(b2)
{
    matrix View;
    float4 SunLight;
    float4 CamPos;
}

struct PS_INPUT
{
    float2 Tex : TEX;
    float3 Ref : REFLECTED;
    float Diffuse : DIFFUSE;
    float3 PosWorld : POS_WORLD;
};

float4 PS( PS_INPUT input ) : SV_TARGET
{
    float ambient = 0.1f;
    float3 specular = pow( max(0.0f, dot(input.Ref, normalize(-input.PosWorld))), 20.0f);
    float total_light = saturate( input.Diffuse * 0.5f + ambient + specular * 0.5f);
    return txDiffuse.Sample( samplerPoint, input.Tex ) * total_light;
}