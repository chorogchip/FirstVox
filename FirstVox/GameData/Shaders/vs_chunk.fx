/*
cbuffer cbChangeOnResize : register(b1)
{
    matrix Projection;
}*/

cbuffer cbChangesEveryFrameVS : register(b0)
{
    matrix ViewProj;
}
/*
cbuffer cbChangesByChunk : register(b3)
{
    matrix World;
}
*/
cbuffer cbChangesByChunk : register(b1)
{
    float4 ChunkPos;
    float4 FogValues;
}

struct VS_INPUT
{
    uint pos : POSITION;
    uint light : LIGHT;
    uint UV : TEXCOORD;
};

struct VS_OUTPUT
{
    //float4 color : LIGHT_COLORS;  // RGB(GL+SUN), AO
    float2 UV : UV;
    float fog : FOG;
    float AO : AMBIENT;
    float4 pos : SV_POSITION;
};

VS_OUTPUT VS( VS_INPUT input )
{

    VS_OUTPUT output = (VS_OUTPUT)0;

    float3 opos = float3(
        (float)((input.pos & 0xff000000) >> 24) - 0.5f,
        (float)((input.pos & 0x00ffff00) >> 8) - 0.5f,
        (float)((input.pos & 0x000000ff)) - 0.5f);

   // uint sun_light = (input.light & 0xf0) >> 4;
    uint AO = input.light & 0x3;
    /*
    output.color = float4(
        (float)((input.light & 0xff000000) >> 24) * (1.0f / 256.0f),
        (float)((input.light & 0x00ff0000) >> 16) * (1.0f / 256.0f),
        (float)((input.light & 0x0000ff00) >> 8) * (1.0f / 256.0f),
        (float)AO * (1.0f / 4.0f) + 0.25f );
    */
    output.AO = (float) AO * (1.0f / 4.0f) + 0.25f;
        
    output.UV = float2(
        ((float) (input.UV & 0x000000ff)) * (16.0f / 256.0f),
        ((float) (input.UV >> 8) * (16.0f / 256.0f)));
        
    /*
    output.color = float3(
        (float)((input.UV & 0xff000000) >> 24) * (1.0f / 256.0f),  
        (float)((input.UV & 0x00ff0000) >> 16) * (1.0f / 256.0f),  
        (float)((input.UV & 0x0000ff00) >> 8) * (1.0f / 256.0f)
    );*/

    float xinv = 32.0f - opos.x;
    float zinv = 32.0f - opos.z;
    float fog1 = FogValues[1] * opos.x + FogValues[0] * xinv;
    float fog2 = FogValues[3] * opos.x + FogValues[2] * xinv;
    output.fog = fog2 * opos.z + fog1 * zinv;
    
    output.pos = mul(float4(opos + ChunkPos.xyz, 1.0f), ViewProj);

    return output;

    /*
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
    */
}