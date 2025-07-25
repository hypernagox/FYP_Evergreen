cbuffer cbPerObject : register(b0)
{
    float4x4 gWorld;
	float gElapsedTime;
	float gRotationMin;
	float gRotationMax;
	float gRotationLifeExp;
	float gLifeTimeMin;
	float gLifeTimeMax;
	float2 gSizeMin;
	float2 gSizeMax;
	float2 gSizeLifeExp;
	float gAlphaLifeExp;
	float gSpeedMin;
	float gSpeedMax;
	float gSpeedLifeExp;
};

cbuffer cbPerCamera : register(b1)
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
}

Texture2D gMainTex : register(t0);
SamplerState gSampler : register(s0);

const static float VIDMUL = 0.01f;

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