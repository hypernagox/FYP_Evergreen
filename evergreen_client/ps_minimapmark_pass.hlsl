struct PixelIn
{
	float4 PosH         : SV_POSITION;
	float2 ScreenTex    : TEXCOORD0;
	float4 PosW         : POSITION0;
	float2 Tex          : TEXCOORD1;
};

Texture2D gTexture : register(t0);
SamplerState gSampler : register(s0);

float4 PS(PixelIn pin) : SV_Target
{
    clip(0.5f - length(pin.ScreenTex.xy - 0.5f));
    return gTexture.Sample(gSampler, pin.Tex);
}