struct PixelIn
{
	float4 PosH         : SV_POSITION;
	float4 PosW         : POSITION0;
	float2 Tex          : TEXCOORD;
};

Texture2D gTexture : register(t0);
SamplerState gSampler : register(s0);

float4 PS(PixelIn pin) : SV_Target
{
    return gTexture.Sample(gSampler, pin.Tex);
}