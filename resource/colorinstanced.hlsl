#include "common.hlsl"

Texture2D gNormalTex : register(t1);

VertexOut VS(VertexIn vin)
{
    VertexOut vout;

    float4 PosL = mul(float4(vin.PosL, 1.0f), vin.InstanceTransform);

    vout.PosW = ObjectToWorldPos(PosL);
	vout.PosH = WorldToClipPos(vout.PosW, vin);                                     
	vout.Tex = vin.Tex;                                                             
	vout.NormalW = ObjectToWorldNormal(mul(vin.Normal, (float3x3)vin.InstanceTransform));       
    vout.TangentW = ObjectToWorldNormal(mul(vin.Tangent, (float3x3)vin.InstanceTransform));
    vout.PrevPosH = mul(mul(PosL, gPrevWorld), gPrevViewProj);

    return vout;
}

PixelOut PS(VertexOut pin)
{
	PixelOut pOut;

    float3 normalW = normalize(pin.NormalW);

    // Normal mapping
    float4 normalMapSample = gNormalTex.Sample(gSampler, pin.Tex);
    float3 normal = NormalSampleToWorldSpace(normalMapSample.rgb, normalW, pin.TangentW.xyz);
    normal = mul(normal, (float3x3)gView);

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