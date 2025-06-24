#include "common.hlsl"

#ifdef DEFERRED

float4 PSDeferred(VertexOut pin) : SV_Target
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
	float4 skyColor = float4(0.378f, 0.457f, 0.563f, 1.0f);
	float shadeValue = min(saturate(dot(normalW, -gDirLight) + 0.5f), ShadowValue(PosW, normalW, 1e-3f));
	shadeValue = round(shadeValue);
	float AOFactor = gSSAOMap.Sample(gsamPointClamp, pin.TexC).r;
	gBuffer1Color.rgb = gBuffer1Color.rgb * lerp(skyColor, 1.0f.xxxx, shadeValue) * AOFactor;

	// Apply rim light.
	float4 limColor = float4(0.2f, 0.2f, 0.2f, 1.0f);
	float rimFactor = saturate(1.0f + normalV.z);
	rimFactor = round(rimFactor);
	gBuffer1Color.rgb += limColor.rgb * rimFactor * shadeValue;

	return float4(gBuffer1Color.rgb, 1.0f);
}

#else

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
    float4 texColor = gMainTex.Sample(gSampler, pin.Tex);
    float4 posH = mul(pin.PosW, gViewProj);
    posH /= posH.w;
    pin.PrevPosH /= pin.PrevPosH.w;
    float4 posDelta = posH - pin.PrevPosH;
    
    clip(texColor.a - 0.1f);
     
    pOut.Buffer1 = texColor;
    pOut.Buffer2 = PackNormal(normal);
    pOut.Buffer3 = posDelta.xy * gMotionBlurFactor * 0.5f * gRenderTargetSize / gMotionBlurRadius;
	pOut.Buffer3 /= max(length(pOut.Buffer3), 1.0f);
    return pOut;
}

#endif