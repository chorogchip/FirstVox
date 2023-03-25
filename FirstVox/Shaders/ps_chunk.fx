
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
    float3 PosCamSpace : POS_CAM_SPACE;
};

float4 PS( PS_INPUT input ) : SV_TARGET
{
    float ambient = 0.1f;
    float specular = 0.0f;
    if (input.Diffuse > 0.0f)
    {
        specular = pow(max(0.0f, dot(input.Ref, normalize(-input.PosCamSpace))), 32);
    }

    float total_light = input.Diffuse * 0.5f + ambient;
    float4 sample_color = txDiffuse.Sample( samplerPoint, input.Tex );
    float4 litColor = sample_color * total_light + float4(specular, specular, specular, 1.0f) * 0.3f;

    float distFromEye = length( input.PosCamSpace );
    float fogStart = 512.0f;
    float fogRange = 256.0f;
    float4 fogColor = float4(0.3f, 0.5f, 0.8f, 1.0f);
    float fogLerp = saturate( (distFromEye - fogStart) / fogRange );

    litColor = lerp( litColor, fogColor, fogLerp );
    litColor.a = 1.0f;

    return litColor;
}
