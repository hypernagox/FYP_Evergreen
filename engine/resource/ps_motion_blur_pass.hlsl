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
	float2 gRenderTargetSize;
};

Texture2D gSource : register(t0);
Texture2D gMotion : register(t1);
Texture2D gDepth : register(t2);
Texture2D gNeighborMax : register(t3);
SamplerState gSamPoint : register(s0);
		
static const uint gSampleCount = 16;

static const float Bayer8x8[64] =
{
    0.0000, 0.7500, 0.1875, 0.9375, 0.0469, 0.7969, 0.2344, 0.9844,
    0.5000, 0.2500, 0.6875, 0.4375, 0.5469, 0.2969, 0.7344, 0.4844,
    0.1250, 0.8750, 0.0625, 0.8125, 0.1719, 0.9219, 0.1094, 0.8594,
    0.6250, 0.3750, 0.5625, 0.3125, 0.6719, 0.4219, 0.6094, 0.3594,
    0.0312, 0.7812, 0.2188, 0.9688, 0.0156, 0.7656, 0.2031, 0.9531,
    0.5312, 0.2812, 0.7188, 0.4688, 0.5156, 0.2656, 0.7031, 0.4531,
    0.1562, 0.9062, 0.0938, 0.8438, 0.1406, 0.8906, 0.0781, 0.8281,
    0.6562, 0.4062, 0.5938, 0.3438, 0.6406, 0.3906, 0.5781, 0.3281
};

float GetDitherThreshold(float2 fragCoord)
{
    int x = (int)fmod(fragCoord.x, 8.0);
    int y = (int)fmod(fragCoord.y, 8.0);
    int index = y * 8 + x;
    return Bayer8x8[index];
}

float NdcDepthToViewDepth(float z_ndc)
{
	float viewZ = gProj[3][2] / (z_ndc - gProj[2][2]);
	return viewZ;
}

// Depth comapare function returns two weights: background and foreground that sum one
float2 DepthComp(float centerDepth, float sampleDepth, float depthScale)
{
    return saturate(0.5f + float2(depthScale, -depthScale) * (sampleDepth - centerDepth));
}

// Spread compare function similar to depth compare
float2 SpreadComp(float offsetLen, float centerSpreadLen, float sampleSpreadLen)
{
	return saturate(float2(centerSpreadLen, sampleSpreadLen) - max(offsetLen - 1.0f, 0.0f));
}

// DepthComp and SpreadComp are used together as follows to get the weight for a sample
float SampleWeight(float centerDepth, float sampleDepth, float offsetLen, float centerSpreadLen, float sampleSpreadLen, float depthScale, float2 weights)
{
    float2 depthComp = DepthComp(centerDepth, sampleDepth, depthScale);
	float2 spreadComp = SpreadComp(offsetLen, centerSpreadLen, sampleSpreadLen);
	return dot(depthComp, spreadComp * weights);
}

float4 PS(VertexOut pin) : SV_Target
{
	float2 rcpro = rcp(gRenderTargetSize);
	float2 vc = gMotion.Sample(gSamPoint, pin.TexC).xy * float2(MAX_BLUR_RADIUS, -MAX_BLUR_RADIUS);
	float2 vn = gNeighborMax.Sample(gSamPoint, pin.TexC).xy * float2(MAX_BLUR_RADIUS, -MAX_BLUR_RADIUS);

	if (length(vn) < 0.5f)
	{
		return gSource.Sample(gSamPoint, pin.TexC);
	}

	float2 wn = normalize(vn);
	float2 wp = float2(-wn.y, wn.x);
	if (dot(wp, vc) < 0)
	{
		wp = -wp;
	}
	float lvx = length(vc);
	float2 wc = normalize(lerp(wp, normalize(vc), saturate((lvx - 0.5f) / 1.5f)));

	float4 sampleSum = 0.0f;
	float depthSrc = NdcDepthToViewDepth(gDepth.Sample(gSamPoint, pin.TexC).r);
	float bias = GetDitherThreshold(pin.PosH.xy);

	[unroll]
	for (uint i = 0; i < gSampleCount; ++i)
	{
		float t = lerp(-1.0f, 1.0f, (i + bias) / gSampleCount);
		float2 d = vn;
		float2 texDest = pin.TexC + d * rcpro * t;
		float depthDst = NdcDepthToViewDepth(gDepth.Sample(gSamPoint, texDest).r);

		float2 vs = gMotion.Sample(gSamPoint, texDest).xy * float2(MAX_BLUR_RADIUS, -MAX_BLUR_RADIUS);
		float wa = dot(wc, normalize(d));
		float wb = abs(dot(normalize(vs), normalize(d)));

		float ld = abs(length(d) * t);
		float lvy = length(vs);
		const float stepScale = 0.5f;
        float y = SampleWeight(depthSrc, depthDst, ld * stepScale, lvx * stepScale, lvy * stepScale, 1e+3f, 1.0f);

		sampleSum += float4(gSource.Sample(gSamPoint, texDest).rgb, 1.0f) * y;
	}

	sampleSum.rgba *= 1.0f / gSampleCount;
	float3 final = sampleSum.rgb + gSource.Sample(gSamPoint, pin.TexC).rgb * (1.0f - sampleSum.a);
	return float4(final, 1.0f);
}