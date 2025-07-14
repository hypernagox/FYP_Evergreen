#include "common.hlsl"

#ifdef DEFERRED

float4 PSDeferred(VertexOut pin) : SV_Target
{
	return PSDeferredDefault(pin);
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
    pOut.Buffer3.rg = posDelta.xy * gMotionBlurFactor * 0.5f * gRenderTargetSize / gMotionBlurRadius;
	pOut.Buffer3.rg /= max(length(pOut.Buffer3.rg), 1.0f);
    return pOut;
}

#endif