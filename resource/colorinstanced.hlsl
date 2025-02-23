#include "common.hlsl"

VertexOut VS(VertexIn vin)
{
    VertexOut vout;

    float4 PosL = mul(float4(vin.PosL, 1.0f), vin.InstanceTransform);

    vout.PosW = ObjectToWorldPos(PosL);
	vout.PosH = WorldToClipPos(vout.PosW, vin);                                     
	vout.Tex = vin.Tex;                                                             
	vout.NormalW = ObjectToWorldNormal(mul(vin.Normal, (float3x3)vin.InstanceTransform));       
    vout.TangentW = ObjectToWorldNormal(mul(vin.Tangent, (float3x3)vin.InstanceTransform));     
    ConstructPosP(vin, vout);                                                       
    vout.PrevPosH = mul(mul(PosL, gPrevWorld), gPrevViewProj);

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