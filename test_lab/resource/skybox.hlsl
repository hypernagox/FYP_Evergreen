#include "common.hlsl"

static const float2 gTexCoords[6] =
{
	float2(0.0f, 1.0f),
	float2(0.0f, 0.0f),
	float2(1.0f, 0.0f),
	float2(0.0f, 1.0f),
	float2(1.0f, 0.0f),
	float2(1.0f, 1.0f)
};

float2 DirectionToEnvironmentUV(float3 dir)
{
	const float PI = 3.14159265359f;
	float t = atan2(dir.z, dir.x);
	float p = asin(dir.y);
	return float2(t / PI * 0.5f + 0.5f, -p / PI + 0.5f);
}

VertexOut VS(uint vid : SV_VertexID)
{
    VertexIn vin = (VertexIn)0;
	VertexOut vout;
    ConstructVSOutput(vin, vout);
	vout.Tex = gTexCoords[vid];
	// Quad covering screen in NDC space.
	vout.PosH = float4(2.0f * vout.Tex.x - 1.0f, 1.0f - 2.0f * vout.Tex.y, 1.0f - 1e-4f, 1.0f);
	vout.NormalW = float4(-gDirLight, 0.0f);
    return vout;
}

PixelOut PS(VertexOut pin)
{
	PixelOut pOut;
	float4 PosNDC = float4(2.0f * pin.Tex.x - 1.0f, 1.0f - 2.0f * pin.Tex.y, 1.0f, 1.0f);
    float4 PosW = mul(PosNDC, gViewProjInverse);
	float4 PrevPosH = mul(PosW, gPrevViewProj);
	PosW /= PosW.w;
	PrevPosH /= PrevPosH.w;
	
    float3 normal = normalize(mul(pin.NormalW.xyz, (float3x3)gView));
    float3 delta = normalize(PosW.xyz - gEyePosW.xyz);
	float2 uv = DirectionToEnvironmentUV(delta);
    float4 texColor = gMainTex.Sample(gSampler, uv);
    float4 posDelta = PosNDC - PrevPosH;

    pOut.Buffer1 = texColor;
    pOut.Buffer2 = PackNormal(normal);
    pOut.Buffer3 = posDelta.xy * gMotionBlurFactor * 0.5f * gRenderTargetSize / gMotionBlurRadius;
	pOut.Buffer3 /= max(length(pOut.Buffer3), 1.0f);
    return pOut;
}