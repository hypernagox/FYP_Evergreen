#include "common.hlsl"

VertexOut VS(VertexIn vin)
{
    VertexOut vout;
    ConstructVSOutput(vin, vout);

    return vout;
}

PixelOut PS(VertexOut pin)
{
    PixelOut pOut;
    float3 normal = normalize(mul(pin.NormalW.xyz, (float3x3) gView));
    float4 texColor = gMainTex.Sample(gSampler, pin.Tex);
     
    pOut.Buffer1 = texColor;
    pOut.Buffer2.xyz = normal;
    pOut.Buffer3 = pin.PosW;
    return pOut;
}