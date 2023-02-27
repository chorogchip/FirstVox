

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
    float3 Pos : POSITION;
    float2 Tex : TEX;
    float3 Nor : NORMAL;
};
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
    output.Pos = float4( input.Pos, 1.0f );
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

    output.Pos = mul( output.Pos, Projection );
    output.Tex = input.Tex;
    output.Ref = (2.0f * dot( input.Nor, SunLight) * input.Nor) - SunLight;
    output.Diffuse = max( 0, dot( input.Nor, SunLight ) );
    return output;
}