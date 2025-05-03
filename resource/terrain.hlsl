#define USE_CUSTOM_SHADOWPS
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

Texture2D gSrcNorm0 : register(t9);
Texture2D gSrcNorm1 : register(t10);
Texture2D gSrcNorm2 : register(t11);
Texture2D gSrcNorm3 : register(t12);
Texture2D gSrcNorm4 : register(t13);
Texture2D gSrcNorm5 : register(t14);
Texture2D gSrcNorm6 : register(t15);

VertexOut VS(VertexIn vin)
{
	VertexOut vout;
    ConstructVSOutput(vin, vout);

    return vout;
}

PixelOut PS(VertexOut pin)
{
	PixelOut pOut;
    float3 normalW = normalize(pin.NormalW.xyz);
	float2 uvMapped = pin.Tex * 128.0f;

	float4 texSplat0 = gSrcSplatmap0.Sample(gSampler, pin.Tex);
	float4 texSplat1 = gSrcSplatmap1.Sample(gSampler, pin.Tex);

	float4 aSample0 = gSrcTex0.Sample(gSampler, uvMapped);
	float4 aSample1 = gSrcTex1.Sample(gSampler, uvMapped);
	float4 aSample2 = gSrcTex2.Sample(gSampler, uvMapped);
	float4 aSample3 = gSrcTex3.Sample(gSampler, uvMapped);
	float4 aSample4 = gSrcTex4.Sample(gSampler, uvMapped);
	float4 aSample5 = gSrcTex5.Sample(gSampler, uvMapped);
	float4 aSample6 = gSrcTex6.Sample(gSampler, uvMapped);

	float height0 = aSample0.a * texSplat0.x;
	float height1 = aSample1.a * texSplat0.y;
	float height2 = aSample2.a * texSplat0.z;
	float height3 = aSample3.a * texSplat0.w;
	float height4 = aSample4.a * texSplat1.x;
	float height5 = aSample5.a * texSplat1.y;
	float height6 = aSample6.a * texSplat1.z;

	float depthMax = height0;
	depthMax = max(depthMax, height1);
	depthMax = max(depthMax, height2);
	depthMax = max(depthMax, height3);
	depthMax = max(depthMax, height4);
	depthMax = max(depthMax, height5);
	depthMax = max(depthMax, height6);
	depthMax -= 0.01f;

	float fraction0 = max(height0 - depthMax, 0);
	float fraction1 = max(height1 - depthMax, 0);
	float fraction2 = max(height2 - depthMax, 0);
	float fraction3 = max(height3 - depthMax, 0);
	float fraction4 = max(height4 - depthMax, 0);
	float fraction5 = max(height5 - depthMax, 0);
	float fraction6 = max(height6 - depthMax, 0);
	float fractionSum = fraction0 + fraction1 + fraction2 + fraction3 + fraction4 + fraction5 + fraction6;

	float mix0 = fraction0 / fractionSum;
	float mix1 = fraction1 / fractionSum;
	float mix2 = fraction2 / fractionSum;
	float mix3 = fraction3 / fractionSum;
	float mix4 = fraction4 / fractionSum;
	float mix5 = fraction5 / fractionSum;
	float mix6 = fraction6 / fractionSum;
	
	float4 texColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
	texColor.rgb += aSample0.rgb * mix0;
	texColor.rgb += aSample1.rgb * mix1;
	texColor.rgb += aSample2.rgb * mix2;
	texColor.rgb += aSample3.rgb * mix3;
	texColor.rgb += aSample4.rgb * mix4;
	texColor.rgb += aSample5.rgb * mix5;
	texColor.rgb += aSample6.rgb * mix6;

	float4 texNormal = 0;
	texNormal += gSrcNorm0.Sample(gSampler, uvMapped) * mix0;
	texNormal += gSrcNorm1.Sample(gSampler, uvMapped) * mix1;
	texNormal += gSrcNorm2.Sample(gSampler, uvMapped) * mix2;
	texNormal += gSrcNorm3.Sample(gSampler, uvMapped) * mix3;
	texNormal += gSrcNorm4.Sample(gSampler, uvMapped) * mix4;
	texNormal += gSrcNorm5.Sample(gSampler, uvMapped) * mix5;
	texNormal += gSrcNorm6.Sample(gSampler, uvMapped) * mix6;

	// Normal mapping
    float3 normal = NormalSampleToWorldSpace(texNormal.rgb, normalW, pin.TangentW.xyz);
    normal = mul(normal, (float3x3)gView);

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

void ShadowPS(VertexOut pin)
{
}
