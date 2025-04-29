#include "common.hlsl"

Texture2D gSrcSplatmap0 : register(t0);
Texture2D gSrcSplatmap1 : register(t1);
Texture2D gSrcTex0 : register(t2);
Texture2D gSrcTex1 : register(t3);
Texture2D gSrcTex2 : register(t4);
Texture2D gSrcTex3 : register(t5);
Texture2D gSrcTex4 : register(t6);
Texture2D gSrcTex5 : register(t7);
Texture2D gSrcTex6 : register(t8);

VertexOut VS(VertexIn vin)
{
	VertexOut vout;
    ConstructVSOutput(vin, vout);

    return vout;
}

PixelOut PS(VertexOut pin)
{
	PixelOut pOut;
    float3 normal = normalize(mul(pin.NormalW.xyz, (float3x3)gView));
	float2 uvMapped = pin.Tex * 128.0f;

	float4 texSplat0 = gSrcSplatmap0.Sample(gSampler, pin.Tex);
	float4 texSplat1 = gSrcSplatmap1.Sample(gSampler, pin.Tex);

	float4 texColor = 0;
	texColor += texSplat0.x * gSrcTex0.Sample(gSampler, uvMapped);
	texColor += texSplat0.y * gSrcTex1.Sample(gSampler, uvMapped);
	texColor += texSplat0.z * gSrcTex2.Sample(gSampler, uvMapped);
	texColor += texSplat0.w * gSrcTex3.Sample(gSampler, uvMapped);
	texColor += texSplat1.x * gSrcTex4.Sample(gSampler, uvMapped);
	texColor += texSplat1.y * gSrcTex5.Sample(gSampler, uvMapped);
	texColor += texSplat1.z * gSrcTex6.Sample(gSampler, uvMapped);

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