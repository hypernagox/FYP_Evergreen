#include "common.hlsl"

#ifdef DEFERRED

float4 PSDeferred(VertexOut pin) : SV_Target
{
    float PI = 3.14159265358979323846f;

    float4 gbuffer1 = gBuffer1.Sample(gsamPointClamp, pin.TexC);
    float2 gbuffer2 = gBuffer2.Sample(gsamPointClamp, pin.TexC);
    float4 gbuffer3 = gBuffer3.Sample(gsamPointClamp, pin.TexC);

	float depth = gBufferDSV.Sample(gsamPointClamp, pin.TexC).r;
	// Compute world space position from depth value.
	float4 PosNDC = float4(2.0f * pin.TexC.x - 1.0f, 1.0f - 2.0f * pin.TexC.y, depth, 1.0f);
    float4 PosW = mul(PosNDC, gViewProjInverse);
	PosW /= PosW.w;

    float3 albedo     = gbuffer1.rgb;
	float3 normalV    = ReconstructNormal(gbuffer2.xy);
    float3 normalWS   = normalize(mul(normalV, (float3x3)gViewInverse));
    float  smoothness = gbuffer3.a;
    float  metallic   = gbuffer3.b;

    float3 N = normalWS;
    float3 L = -gDirLight;
    float3 V = normalize(gEyePosW.xyz - PosW.xyz);
    float3 H = normalize(L + V); // Half vector

    float3 F0 = lerp(float3(0.04f, 0.04f, 0.04f), albedo, metallic);

    // Normal Distribution Function (GGX/Trowbridge-Reitz)
    float NdotH = max(dot(N, H), 0.0f);
    float alpha = max(0.001f, smoothness);
    float alphaSqr = alpha * alpha;
    float denom = (NdotH * NdotH) * (alphaSqr - 1.0f) + 1.0f;
    float D = alphaSqr / (PI * denom * denom);

    // Geometry Function (Schlick-GGX)
    float NdotV = max(dot(N, V), 0.0f);
    float NdotL = max(dot(N, L), 0.0f);
    float k = (alpha + 1.0f) * (alpha + 1.0f) / 8.0f;

    float G_V = NdotV / (NdotV * (1.0f - k) + k);
    float G_L = NdotL / (NdotL * (1.0f - k) + k);
    float G = G_V * G_L;

    // Fresnel Equation (Schlick's Approximation)
    float3 F = F0 + (1.0f - F0) * pow(1.0f - saturate(dot(H, V)), 5.0f);

    // Specular Term
    float3 specular = (D * G * F) / (4.0f * max(NdotV, 0.001f) * max(NdotL, 0.001f));

    // Diffuse Term (Lambertian, modulated by metalness)
    float3 kd = (1.0f - F) * (1.0f - metallic);
    float3 diffuse = kd * albedo / PI;

    // Final Shading
    float3 radiance = 2.0f.xxx; // Assumed directional light color
    float3 color = (diffuse + specular) * radiance * NdotL * ShadowValue(PosW, normalWS);

    // Ambient Term (you may replace with IBL or ambient light color)
    float3 ambient = AmbientLight(pin) * albedo;
    color += ambient;

    return float4(color, 1.0f);
}

#else

Texture2D gNormalTex : register(t1);
Texture2D gMetallicTex : register(t2);

VertexOut VS(VertexIn vin)
{
	VertexOut vout;
    ConstructVSOutput(vin, vout);

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
    
    clip(texColor.a - 0.1f);
     
    pOut.Buffer1 = texColor;
    pOut.Buffer2 = PackNormal(normal);
    pOut.Buffer3.rg = PackMotion(posH, pin.PrevPosH);
    pOut.Buffer3.ba = gMetallicTex.Sample(gSampler, pin.Tex).ra;

    return pOut;
}

#endif