
Texture2D txDiffuse : register(t0);
SamplerState samplerPoint : register(s0);

struct PS_INPUT
{
    float2 Color : COLOR;
};

float4 PS( PS_INPUT input ) : SV_TARGET
{
    return txDiffuse.Sample( samplerPoint, input.Color );
}