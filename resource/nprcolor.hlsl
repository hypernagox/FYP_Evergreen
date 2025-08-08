#include "common.hlsl"

#ifdef DEFERRED

float4 PSDeferred(VertexOut pin) : SV_Target
{
	float3 normalV = ReconstructNormal(gBuffer2.Sample(gsamPointClamp, pin.TexC).xy);
	float3 normalW = normalize(mul(normalV, transpose((float3x3)gView)));

	float depth = gBufferDSV.Sample(gsamPointClamp, pin.TexC).r;
	// Compute world space position from depth value.
	float4 PosNDC = float4(2.0f * pin.TexC.x - 1.0f, 1.0f - 2.0f * pin.TexC.y, depth, 1.0f);
    float4 PosW = mul(PosNDC, gViewProjInverse);
	PosW /= PosW.w;

	float4 gBuffer1Color = gBuffer1.Sample(gsamPointClamp, pin.TexC);
	gBuffer1Color.rgb = pow(gBuffer1Color.rgb, gamma);

	float diffuseValue = min(ShadowValue(PosW, normalW, 1e-3f), DiffuseLight(pin));
	diffuseValue = ceil(diffuseValue - 0.1f);
	float3 fColor = (AmbientLight(pin) * 4.0f + diffuseValue) * gBuffer1Color.rgb;

	// Apply rim light.
	float4 limColor = float4(0.2f, 0.2f, 0.2f, 1.0f);
	float rimFactor = saturate(1.0f + normalV.z);
	rimFactor = round(rimFactor);
	fColor += limColor.rgb * rimFactor * diffuseValue;

	return float4(fColor, 1.0f);
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
    
    clip(texColor.a - 0.1f);
     
    pOut.Buffer1 = texColor;
    pOut.Buffer2 = PackNormal(normal);
    pOut.Buffer3.rg = PackMotion(posH, pin.PrevPosH);
    return pOut;
}

#endif