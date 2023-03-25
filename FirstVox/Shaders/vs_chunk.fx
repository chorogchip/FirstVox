

cbuffer cbChangeOnResize : register(b1)
{
    matrix Projection;
}

cbuffer cbChangesEveryFrame : register(b2)
{
    matrix View;
    float4 SunLight;
    float4 CamPos;
}
cbuffer cbChangesByChunk : register(b3)
{
    matrix World;
}
struct VS_INPUT
{
    uint Pos : POSITION_NORMAL;
    uint Tex : UV;
};
/*
struct VS_INPUT
{
    float3 Pos : POSITION;
    float2 Tex : TEX;
    float3 Nor : NORMAL;
};*/
struct VS_OUTPUT
{
    float2 Tex : TEX;
    float3 Ref : REFLECTED;
    float Diffuse : DIFFUSE;
    float3 PosCamSpace : POS_CAM_SPACE;
    float4 Pos : SV_POSITION;
};

VS_OUTPUT VS( VS_INPUT input )
{
    VS_OUTPUT output = (VS_OUTPUT)0;
    output.Pos = float4(
        (float)((input.Pos & 0xff000000) >> 24) - 0.5f,
        (float)((input.Pos & 0x00ffc000) >> 14) - 0.5f,
        (float)((input.Pos & 0x00003fc0) >> 6) - 0.5f,
        1.0f );

    output.Pos = mul( output.Pos, World );
    output.PosCamSpace = output.Pos - CamPos;
    output.Pos = mul( output.Pos, View );
    
    // earth curvature shader
    /*
    float r = 6371000.0f + output.Pos.y;
    float dist = sqrt(output.Pos.x * output.Pos.x + output.Pos.z * output.Pos.z);
    float theta = dist / r;
    output.Pos.y -= r * (1.0f - cos( theta ));
    float new_d = sin( theta ) / theta;
    output.Pos.x *= new_d;
    output.Pos.z *= new_d;
    /**/
    float3 Normal = float3( 0.0f, 1.0f, 0.0f);
    switch ( input.Pos & 0x0000003f )
    {
    case 0: Normal = float3( 0.0f, 1.0f, 0.0f); break;
    case 1: Normal = float3( 0.0f,-1.0f, 0.0f); break;
    case 2: Normal = float3( 0.0f, 0.0f, 1.0f); break;
    case 3: Normal = float3( 0.0f, 0.0f,-1.0f); break;
    case 4: Normal = float3( 1.0f, 0.0f, 0.0f); break;
    case 5: Normal = float3(-1.0f, 0.0f, 0.0f); break;
    }

    output.Pos = mul( output.Pos, Projection );
    output.Tex = float2(
        (float)((input.Tex & 0x0000ffff)) * (1.0f / 16.0f),
        (float)((input.Tex & 0xffff0000) >> 16) * (1.0f / 16.0f));
    output.Ref = (2.0f * dot( Normal, SunLight) * Normal) - SunLight;
    output.Diffuse = max( 0, dot( Normal, SunLight ) );
    return output;
}