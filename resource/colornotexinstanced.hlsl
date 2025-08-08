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
    float3 normal = normalize(mul(pin.NormalW.xyz, (float3x3)gView));
    float4 texColor = 1.0f;
    float4 posH = mul(pin.PosW, gViewProj);
     
    pOut.Buffer1 = texColor;
    pOut.Buffer2 = PackNormal(normal);
    pOut.Buffer3.rg = PackMotion(posH, pin.PrevPosH);
    return pOut;
}

#endif