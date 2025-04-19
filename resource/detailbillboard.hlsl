#define USE_CUSTOM_SHADOWPS
#include "common.hlsl"

struct GeometryOut {
	float4 PosH     : SV_POSITION;
	float4 PosW     : POSITION0;
    float4 PrevPosH : POSITION1;
	float2 Tex      : TEXCOORD0;
	float3 NormalW  : NORMAL;
};

VertexOut VS(VertexIn vin)
{
	VertexOut vout;
    ConstructVSOutput(vin, vout);

    return vout;
}

[maxvertexcount(4)]
void GS(point VertexOut input[1], uint primitiveID : SV_PRIMITIVEID, inout TriangleStream<GeometryOut> outStream)
{
    const float viewDepth = length(input[0].PosW - gEyePosW);
    const float scale = saturate(8.0f - viewDepth * 0.125f) * 2.0f;

    float3 up = normalize(float3(gView[0].y, gView[1].y, gView[2].y) + float3(0.0f, 1.0f, 0.0f));
    float3 look = -normalize(float3(gView[0].z, gView[1].z, gView[2].z));
    float3 right = normalize(cross(up, look));
    float3x3 uvw = float3x3(right, up, look);

    float3 offsets[4];
    offsets[0] = float3(+0.5f, 0.0f, 0.0f);
    offsets[1] = float3(+0.5f, 1.0f, 0.0f);
    offsets[2] = float3(-0.5f, 0.0f, 0.0f);
    offsets[3] = float3(-0.5f, 1.0f, 0.0f);
    
    input[0].PosW.xyz *= 2.0f;

    GeometryOut output;
    for (uint i = 0; i < 4; ++i)
    {
        output.PosW = input[0].PosW + float4(mul(offsets[i], uvw) * scale, 1.0f);
        output.PosH = mul(output.PosW, gViewProj);
        output.PrevPosH = mul(output.PosW, gPrevViewProj);
        output.NormalW = -gDirLight;
        output.Tex = input[0].Tex + float2(i / 2, 1 - i % 2) * 0.5f;
        outStream.Append(output);
    }
}

PixelOut PS(GeometryOut pin)
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

void ShadowPS(GeometryOut pin)
{
    float a = gMainTex.Sample(gSampler, pin.Tex).a;
    clip(a - 0.1f);
}