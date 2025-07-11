#include "vs_drawscreen.hlsl"

cbuffer cbPass : register(b0)
{
	float gExposure;
	float gBloomStrength;
};

Texture2D gDestination : register(t0);
Texture2D gSourceBloom : register(t1);

SamplerState gsamPointClamp : register(s0);
SamplerState gsamLinearClamp : register(s1);

float3 ACESToneMapping(float3 v)
{ 
    v *= 0.6f;
    float a = 2.51f;
    float b = 0.03f;
    float c = 2.43f;
    float d = 0.59f;
    float e = 0.14f;
    return clamp((v * (a * v + b)) / (v * (c * v + d) + e), 0.0f, 1.0f);
}

float Luminance(float3 v)
{
	return dot(v, float3(0.2126f, 0.7152f, 0.0722f));
}

float4 PS(VertexOut pin) : SV_Target
{
    const float gamma = 2.2f;

    float4 colDest = gDestination.Sample(gsamPointClamp, pin.TexC);
	float4 colBloom = gSourceBloom.Sample(gsamPointClamp, pin.TexC);

	// Apply bloom effect
	float3 colFinal = colDest.rgb + colBloom.rgb * gBloomStrength;

	// Ensure the final color is mapped to LDR
	colFinal = ACESToneMapping(colFinal * gExposure);

	colFinal = pow(colFinal, 1.0f / gamma);
	colFinal = lerp(colFinal, Luminance(colFinal), 1.0f - 1.1f);

	return float4(colFinal, 1.0f);
}