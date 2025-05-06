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

// discontinuous pseudorandom uniformly distributed in [-0.5, +0.5]^3
float3 rand3(float3 c) {
	float j = 4096.0f * sin(dot(c, float3(17.0f, 59.4f, 15.0f)));
	float3 r;
	r.z = frac(512.0f * j);
	j *= 0.125f;
	r.x = frac(512.0f * j);
	j *= 0.125f;
	r.y = frac(512.0f * j);
	return r - 0.5f;
}

float NdcDepthToViewDepth(float z_ndc)
{
	float viewZ = gProj[3][2] / (z_ndc - gProj[2][2]);
	return viewZ;
}

float Cone(float x, float r)
{
	return saturate(1.0f - abs(x) / r);
}

float Cylinder(float x, float r)
{
	return 1.0f - smoothstep(r * 0.95f, r * 1.05f, abs(x));
}

float SoftDepthComp(float lhs, float rhs)
{
	const float ext = 1e-3f;
	return saturate(1.0f - (lhs - rhs) / ext);
}

float4 PS(VertexOut pin) : SV_Target
{
	uint dx, dy;
	gSource.GetDimensions(dx, dy);
	float2 rcpro = rcp(float2(dx, dy));

	float2 vn = gNeighborMax.Sample(gSamPoint, pin.TexC).xy * MAX_BLUR_RADIUS;
	vn.y = -vn.y;

	if (length(vn) < 0.5f)
	{
		return gSource.Sample(gSamPoint, pin.TexC);
	}

	float2 vx = gMotion.Sample(gSamPoint, pin.TexC).xy * MAX_BLUR_RADIUS;
	vx.y = -vx.y;

	float weight = 1.0f / max(length(vx), 1.0f);
	float4 color = gSource.Sample(gSamPoint, pin.TexC) * weight;

	float depthSrc = NdcDepthToViewDepth(gDepth.Sample(gSamPoint, pin.TexC).r);
	float bias = rand3(float3(pin.TexC, 0.0f)).x;

	[unroll]
	for (uint i = 0; i < gSampleCount; ++i)
	{
		if (i == (gSampleCount - 1) / 2)
		{
			continue;
		}

		float t = lerp(-1.0f, 1.0f, (i + bias + 1.0f) / (gSampleCount + 1.0f));
		float2 texDest = pin.TexC + vn * rcpro * t;
		float depthDst = NdcDepthToViewDepth(gDepth.Sample(gSamPoint, texDest).r);

		float d = length(vn) * t;
		float2 vy = gMotion.Sample(gSamPoint, texDest) * MAX_BLUR_RADIUS;
		vy.y = -vy.y;
        float y =	SoftDepthComp(depthDst, depthSrc) * Cone(d, length(vy)) +
					SoftDepthComp(depthSrc, depthDst) * Cone(d, length(vx)) +
					Cylinder(d, length(vy)) * Cylinder(d, length(vx)) * 2.0f;

		weight += y;
		color += gSource.Sample(gSamPoint, texDest) * y; 
	}

	return color / weight;
}