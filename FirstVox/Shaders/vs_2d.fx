

struct VS_INPUT
{
    float posX : POSITIONX;
    float posY : POSITIONY;
    uint U16V16 : UV;
};

struct VS_OUTPUT
{
    float4 pos : SV_POSITION;
    float2 uv : UV;
};


VS_OUTPUT VS(VS_INPUT input)
{
    VS_OUTPUT output = (VS_OUTPUT) 0;
    output.uv = float2(
        (float) (input.U16V16 & 0xffff) * (1.0f / 16.0f),
        (float) (input.U16V16 >> 16) * (1.0f / 16.0f));
    output.pos = float4(
        input.posX,
        input.posY,
        0.5f, 1.0f);
    
    return output;
}