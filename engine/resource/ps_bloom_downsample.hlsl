#include "vs_drawscreen.hlsl"

Texture2D gSource : register(t0);

SamplerState gsamPointClamp : register(s0);
SamplerState gsamLinearClamp : register(s1);

static const float2 gOffsets[13] = {
	float2(-2, -2), float2(0, -2),	float2(2, -2),
	float2(-1, -1), float2(1, -1),
	float2(-2, 0),	float2(0, 0),	float2(2, 0),
	float2(-1, 1),	float2(1, 1),
	float2(-2, 2),	float2(0, 2),	float2(2, 2)
};

static const float gWeights[13] = {
	0.03125f,	0.0625f,	0.03125f,
	0.125f,		0.125f,
	0.0625f,	0.125f,		0.0625f,
	0.125f,		0.125f,
	0.03125f,	0.0625f,	0.03125f
};

float4 PS(VertexOut pin) : SV_Target
{
	uint sx, sy;
	gSource.GetDimensions(sx, sy);
	float2 rcpro = rcp(float2(sx, sy));

	float4 color = float4(0, 0, 0, 0);
	[unroll]
	for (int i = 0; i < 13; ++i)
	{
		float2 offset = gOffsets[i] * rcpro;
		float4 sample = gSource.Sample(gsamLinearClamp, pin.TexC + offset);
		color += sample * gWeights[i];
	}
	return color;
}