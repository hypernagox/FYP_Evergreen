#include "vs_drawscreen.hlsl"

cbuffer cbPass : register(b0)
{
	float gSamRadius;
};

Texture2D gSource : register(t0);

SamplerState gsamPointClamp : register(s0);
SamplerState gsamLinearClamp : register(s1);

static const float2 gOffsets[9] = {
	float2(-1, -1), float2(0, -1),	float2(1, -1),
	float2(-1, 0),	float2(0, 0),	float2(1, 0),
	float2(-1, 1),	float2(0, 1),	float2(1, 1)
};

static const float gWeights[9] = {
	0.0625f,	0.125f,		0.0625f,
	0.125f,		0.25f,		0.125f,
	0.0625f,	0.125f,		0.0625f
};

float4 PS(VertexOut pin) : SV_Target
{
	uint sx, sy;
	gSource.GetDimensions(sx, sy);
	float2 rcpro = rcp(float2(sx, sy));

	float4 color = float4(0, 0, 0, 0);
	[unroll]
	for (int i = 0; i < 9; ++i)
	{
		float2 offset = gOffsets[i] * rcpro * gSamRadius;
		float4 sample = gSource.Sample(gsamLinearClamp, pin.TexC + offset);
		color += sample * gWeights[i];
	}
	return color;
}