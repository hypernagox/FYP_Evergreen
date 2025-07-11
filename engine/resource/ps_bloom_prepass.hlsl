#include "vs_drawscreen.hlsl"

cbuffer cbPass : register(b0)
{
	float gThreshold;
};

Texture2D gSource : register(t0);

SamplerState gsamPointClamp : register(s0);
SamplerState gsamLinearClamp : register(s1);

float Luminance(float3 v)
{
	return dot(v, float3(0.2126f, 0.7152f, 0.0722f));
}

float4 PS(VertexOut pin) : SV_Target
{
	float4 color = gSource.Sample(gsamPointClamp, pin.TexC);
	float luminance = Luminance(color.rgb);
	color.rgb *= max(luminance - gThreshold, 0.0f) / max(luminance, gThreshold);

	return color;
}