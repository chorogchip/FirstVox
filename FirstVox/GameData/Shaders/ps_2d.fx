

Texture2D txDiffuse : register(t0);
SamplerState samplerPoint : register(s0);

struct PS_INPUT
{
    float4 pos : SV_POSITION;
    float2 uv : UV;
};

struct PS_OUTPUT
{
    float4 color;
};

PS_OUTPUT PS( PS_INPUT input ) : SV_TARGET
{
    PS_OUTPUT output = (PS_OUTPUT) 0;
    output.color = txDiffuse.Sample(samplerPoint, input.uv);
    clip(output.color.a == 0.0f ? -1 : 1);
    return output;
}
