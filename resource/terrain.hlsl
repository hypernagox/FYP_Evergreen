#define USE_CUSTOM_SHADOWPS
#include "common.hlsl"

VertexOut VS(VertexIn vin)
{
	VertexOut vout;
    ConstructVSOutput(vin, vout);

    return vout;
}

struct HullOut
{
	float4 PosW : POSITION;
	float2 Tex : TEXCOORD0;
	float4 NormalW : NORMAL;
};

struct HullConstantOut
{
	float edges[4] : SV_TessFactor;
	float inside[2] : SV_InsideTessFactor;
};

HullConstantOut HSConstant(InputPatch<VertexOut, 16> vin)
{
	HullConstantOut output;
	const float c = max(min(32 / vin[0].PosH.w, 4), 1);
	output.edges[0] = c;
	output.edges[1] = c;
	output.edges[2] = c;
	output.edges[3] = c;
	output.inside[0] = c;
	output.inside[1] = c;
	return output;
}

[domain("quad")]
[partitioning("integer")]
[outputtopology("triangle_ccw")]
[outputcontrolpoints(16)]
[patchconstantfunc("HSConstant")]
HullOut HS(InputPatch<VertexOut, 16> vin, uint index : SV_OutputControlPointID)
{
	HullOut output;
	output.PosW = vin[index].PosW;
	output.Tex = vin[index].Tex;
	output.NormalW = vin[index].NormalW;
	return output;
}

struct DomainOut
{
	float4 PosH		: SV_POSITION;
	float4 PosW		: POSITION0;
	float4 PrevPosH	: POSITION1;
	float2 Tex		: TEXCOORD0;
	float4 NormalW	: NORMAL;
};

float4 CatmullRom(float4 p0, float4 p1, float4 p2, float4 p3, float t)
{
    float t1 = t;
	float t2 = t * t;
	float t3 = t * t * t;
	float d0 = t3 * -1.0f + t2 * +2.0f + t1 * -1.0f + 0.0f;
	float d1 = t3 * +3.0f + t2 * -5.0f + t1 * +0.0f + 2.0f;
	float d2 = t3 * -3.0f + t2 * +4.0f + t1 * +1.0f + 0.0f;
	float d3 = t3 * +1.0f + t2 * -1.0f;
	return (p0 * d0 + p1 * d1 + p2 * d2 + p3 * d3) * 0.5f;
}

#define BILINEAR(comp) lerp(lerp(hin[5].comp, hin[6].comp, u), lerp(hin[9].comp, hin[10].comp, u), v)

[domain("quad")]
DomainOut DS(HullConstantOut hcin, OutputPatch<HullOut, 16> hin, float2 uv : SV_DomainLocation)
{
	DomainOut output;

	float u = uv.x;
	float v = uv.y;

	float4 p1 = CatmullRom(hin[0].PosW, hin[1].PosW, hin[2].PosW, hin[3].PosW, u);
	float4 p2 = CatmullRom(hin[4].PosW, hin[5].PosW, hin[6].PosW, hin[7].PosW, u);
	float4 p3 = CatmullRom(hin[8].PosW, hin[9].PosW, hin[10].PosW, hin[11].PosW, u);
	float4 p4 = CatmullRom(hin[12].PosW, hin[13].PosW, hin[14].PosW, hin[15].PosW, u);
	output.PosW = CatmullRom(p1, p2, p3, p4, v);
	output.PosH = mul(output.PosW, gViewProj);
	output.PrevPosH = mul(output.PosW, gPrevViewProj);
	output.Tex = BILINEAR(Tex);
	output.NormalW = BILINEAR(NormalW);
	return output;
}

#ifdef GENERATE_SHADOWS
void ShadowPS(DomainOut pin)
{
}

#else
PixelOut PS(DomainOut pin)
{
	PixelOut pOut;
    float3 normal = normalize(mul(pin.NormalW.xyz, (float3x3)gView));
    float4 texColor = gMainTex.Sample(gSampler, pin.Tex * 16.0f);
    float4 posH = mul(pin.PosW, gViewProj);
    posH /= posH.w;
    pin.PrevPosH /= pin.PrevPosH.w;
    float4 posDelta = posH - pin.PrevPosH;
     
    pOut.Buffer1 = texColor;
    pOut.Buffer2 = PackNormal(normal);
    pOut.Buffer3 = posDelta.xy * gMotionBlurFactor * 0.5f * gRenderTargetSize / gMotionBlurRadius;
	pOut.Buffer3 /= max(length(pOut.Buffer3), 1.0f);
    return pOut;
}
#endif