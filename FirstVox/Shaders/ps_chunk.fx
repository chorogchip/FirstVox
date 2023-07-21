

Texture2D txDiffuse : register(t0);
SamplerState samplerPoint : register(s0);

cbuffer cbChangesEveryFramePS : register(b2)
{
    float4 skyColor;
}

struct PS_INPUT
{
    //float4 color : LIGHT_COLORS;  // RGB(GL+SUN), AO
    //float3 color : TEXCOORD;
    float2 UV : UV;
    float fog : FOG;
    float AO : AMBIENT;
    float4 pos : SV_POSITION;
};

struct PS_OUTPUT
{
    float4 color;
};

PS_OUTPUT PS( PS_INPUT input ) : SV_TARGET
{
    PS_OUTPUT output = (PS_OUTPUT)0;
    float4 tex_color = txDiffuse.Sample(samplerPoint, input.UV);
    clip(tex_color.a == 0.0f ? -1 : 1);
    float3 output_color = tex_color.rgb * input.AO * (1.0f - input.fog) + input.fog * skyColor.xyz;
    output.color = float4(output_color, 1.0f);
    return output;

    // prev phong model
    /*
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
    */
}
