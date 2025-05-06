#include "vs_drawscreen.hlsl"

cbuffer cbPerCamera : register(b0)
{
	float4x4 gView;
	float4x4 gProj;
	float4x4 gViewProj;
	float4x4 gViewInverse;
	float4x4 gProjInverse;
	float4x4 gViewProjInverse;
	float4x4 gPrevViewProj;
	float4 gEyePosW;
}

cbuffer cbPerShadow : register(b1)
{
	float4x4 gLightViewProj[4];
	float4x4 gLightViewProjClip[4];
	float4 gLightPosW[4];
	float4 gShadowDistance;
	float4 gShadowBias;
	float3 gDirLight;
};

// Nonnumeric values cannot be added to a cbuffer.
Texture2D gBuffer1    : register(t0);
Texture2D gBuffer2    : register(t1);
Texture2D gBuffer3    : register(t2);
Texture2D gShadowMap  : register(t3);
Texture2D gSSAOMap	  : register(t4);
Texture2D gBufferDSV  : register(t5);

SamplerState gsamPointClamp : register(s0);
SamplerState gsamLinearClamp : register(s1);
SamplerState gsamDepthMap : register(s2);
SamplerState gsamLinearWrap : register(s3);
SamplerComparisonState gSamplerShadow : register(s4);

static const float4x4 gTex = {
	0.5f, 0.0f, 0.0f, 0.0f,
	0.0f, -0.5f, 0.0f, 0.0f,
	0.0f, 0.0f, 1.0f, 0.0f,
	0.5f, 0.5f, 0.0f, 1.0f
};

float ShadowValue(float4 posW, float3 normalW, int level)
{
	float4 shadowPosH = mul(mul(posW, gLightViewProjClip[level]), gTex);

	// Complete projection by doing division by w.
	shadowPosH.xy /= shadowPosH.w;
    
	float percentLit = 0.0f;
	if (max(shadowPosH.x, shadowPosH.y) < 1.0f && min(shadowPosH.x, shadowPosH.y) > 0.0f)
	{
		// Depth in NDC space.
		float depth = shadowPosH.z;

		uint width, height, numMips;
		gShadowMap.GetDimensions(0, width, height, numMips);

		// Texel size.
		float dx = 1.0f / (float)width;
		float tanLight = abs(length(cross(normalW, -gDirLight)) / dot(normalW, -gDirLight));
		float bias = 1e-4f;

		const float2 offsets[9] = {
			float2(-dx, -dx), float2(0.0f, -dx), float2(dx, -dx),
			float2(-dx, 0.0f), float2(0.0f, 0.0f), float2(dx, 0.0f),
			float2(-dx, +dx), float2(0.0f, +dx), float2(dx, +dx)
		};

		[unroll]
		for (int i = 0; i < 9; ++i)
			percentLit += gShadowMap.SampleCmpLevelZero(gSamplerShadow, shadowPosH.xy + offsets[i], depth - bias).r;
	}
	else
	{
		percentLit = 9.0f;
	}
    
	return percentLit / 9.0f;
}

float ShadowValue(float4 posW, float3 normalW)
{
	float distanceW0 = length(posW.xyz - gLightPosW[0].xyz);
	float distanceW1 = length(posW.xyz - gLightPosW[1].xyz);
	float distanceW2 = length(posW.xyz - gLightPosW[2].xyz);
	float distanceW3 = length(posW.xyz - gLightPosW[3].xyz);
	if (distanceW0 < gShadowDistance[0])
		return lerp(ShadowValue(posW, normalW, 0), ShadowValue(posW, normalW, 1), saturate((distanceW0 - gShadowDistance[0]) / (0.1f * gShadowDistance[0]) + 1.0f));
	else if (distanceW1 < gShadowDistance[1])
		return lerp(ShadowValue(posW, normalW, 1), ShadowValue(posW, normalW, 2), saturate((distanceW1 - gShadowDistance[1]) / (0.1f * (gShadowDistance[1] - gShadowDistance[0])) + 1.0f));
	else if (distanceW2 < gShadowDistance[2])
		return lerp(ShadowValue(posW, normalW, 2), ShadowValue(posW, normalW, 3), saturate((distanceW2 - gShadowDistance[2]) / (0.1f * (gShadowDistance[2] - gShadowDistance[1])) + 1.0f));
	else if (distanceW3 < gShadowDistance[3])
		return lerp(ShadowValue(posW, normalW, 3), 1.0f, saturate((distanceW3 - gShadowDistance[3]) / (0.1f * (gShadowDistance[3] - gShadowDistance[2])) + 1.0f));
	else
		return 1.0f;
}

float3 ReconstructNormal(float2 np)
{
	float3 n;
	n.z = dot(np, np) * 2.0f - 1.0f;
	n.xy = normalize(np) * sqrt(1.0f - n.z * n.z);
	return n;
}
 
float4 PS(VertexOut pin) : SV_Target
{
	float depth = gBufferDSV.Sample(gsamPointClamp, pin.TexC).r;
	// Compute world space position from depth value.
	float4 PosNDC = float4(2.0f * pin.TexC.x - 1.0f, 1.0f - 2.0f * pin.TexC.y, depth, 1.0f);
    float4 PosW = mul(PosNDC, gViewProjInverse);
	PosW /= PosW.w;

	float4 gBuffer1Color = gBuffer1.Sample(gsamPointClamp, pin.TexC);
	float3 normalV = ReconstructNormal(gBuffer2.Sample(gsamPointClamp, pin.TexC).xy);
	float3 normalW = normalize(mul(normalV, transpose((float3x3)gView)));

	// Sky color. #142743
	float4 skyColor = float4(0.178f, 0.257f, 0.363f, 1.0f);
	float diffuse = saturate(dot(normalW, -gDirLight) * 1.5f);
	float shadowValue = ShadowValue(PosW, normalW);
	float AOFactor = gSSAOMap.Sample(gsamPointClamp, pin.TexC).r;
	gBuffer1Color.rgb = gBuffer1Color.rgb * lerp(skyColor, 1.0f.xxxx, min(shadowValue, diffuse)) * AOFactor;
	return float4(gBuffer1Color.rgb, 1.0f);
}