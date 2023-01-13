
cbuffer cbNeverChanges : register(b0)
{
}

cbuffer cbChangeOnResize : register(b1)
{
    matrix Projection;
}

cbuffer cbChangesEveryFrame : register(b2)
{
    matrix World;
    matrix View;
}
struct VS_INPUT
{
    float3 Pos : POSITION;
    float2 Color : COLOR;
};
struct VS_OUTPUT
{
    float2 Color : COLOR;
    float4 Pos : SV_POSITION;
};

VS_OUTPUT VS( VS_INPUT input )
{
    VS_OUTPUT output = (VS_OUTPUT)0;
    output.Pos = float4( input.Pos, 1.0f );
    output.Pos = mul( output.Pos, World );
    output.Pos = mul( output.Pos, View );
    output.Pos = mul( output.Pos, Projection );
    output.Color = input.Color;
    return output;
}