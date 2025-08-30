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

    float3 albedo     = pow(gbuffer1.rgb, gamma);
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
    float alpha = max(0.001f, smoothness * smoothness);
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
    float3 F = F0 + (1.0f - F0) * pow(1.0f - saturate(dot(N, V)), 5.0f);

    // Specular Term
    float3 specular = (D * G * F) / (4.0f * max(NdotV, 0.001f) * max(NdotL, 0.001f));

    // Diffuse Term (Lambertian, modulated by metalness)
    float3 kd = (1.0f - F) * (1.0f - metallic);
    float3 diffuse = kd * albedo / PI;

    // Final Shading
    float3 radiance = 2.0f.xxx; // Assumed directional light color
    float3 color = (diffuse + specular) * radiance * NdotL * ShadowValue(PosW, normalWS);

    // Ambient Term (you may replace with IBL or ambient light color)
	float3 skyColor = pow(float3(0.2274f, 0.2274f, 0.2274f), gamma);
    float3 ambientDiffuse = skyColor * albedo;
    float3 ambientSpecular = skyColor * F * (1.0f - smoothness); // Roughness 기반으로 감소
    color += ambientSpecular;
    color = ApplyFog(color, PosW);

    return float4(color, 1.0f);
}

#else

Texture2D gFlowTex : register(t1);
Texture2D gDerivTex : register(t2);

VertexOut VS(VertexIn vin)
{
	VertexOut vout;
    ConstructVSOutput(vin, vout);

    return vout;
}

float3 FlowUVW (float2 uv, float2 flowVector, float2 jump, float flowOffset, float tiling, float time, bool flowB)
{
	float phaseOffset = flowB ? 0.5 : 0;
	float progress = frac(time + phaseOffset);
	float3 uvw;
	uvw.xy = uv - flowVector * (progress + flowOffset);
	uvw.xy *= tiling;
	uvw.xy += phaseOffset;
	uvw.xy += (time - progress) * jump;
	uvw.z = 1 - abs(1 - 2 * progress);
	return uvw;
}

float3 UnpackDerivativeHeight (float4 textureData)
{
	float3 dh = textureData.agb;
	dh.xy = dh.xy * 2 - 1;
	return dh;
}

static float _FlowStrength = 0.02f;
static float _HeightScale = 1.0f;
static float _HeightScaleModulated = 9.0f;
static float _Speed = 0.2f;
static float _UJump = 0.202f;
static float _VJump = 0.202f;
static float _Tiling = 32.0f;
static float _FlowOffset = 0.0f;
static float4 _Color = float4(0.3568f, 0.6549f, 0.8235f, 1.0f);

PixelOut PS(VertexOut pin)
{
	PixelOut pOut;

	float3 flow = float3(gFlowTex.Sample(gSampler, pin.Tex).rgb);
	flow.xy = flow.xy * 2 - 1;
	flow *= _FlowStrength;
	float noise = gFlowTex.Sample(gSampler, pin.Tex).a;
	float time = gTime * _Speed + noise;
	float2 jump = float2(_UJump, _VJump);

	float3 uvwA = FlowUVW(pin.Tex, flow.xy, jump, _FlowOffset, _Tiling, time, false);
	float3 uvwB = FlowUVW(pin.Tex, flow.xy, jump, _FlowOffset, _Tiling, time, true);

	float finalHeightScale = flow.z * _HeightScaleModulated + _HeightScale;

	float3 dhA = UnpackDerivativeHeight(gDerivTex.Sample(gSampler, uvwA.xy)) * (uvwA.z * finalHeightScale);
	float3 dhB = UnpackDerivativeHeight(gDerivTex.Sample(gSampler, uvwB.xy)) * (uvwB.z * finalHeightScale);

	float4 texA = gMainTex.Sample(gSampler, uvwA.xy) * uvwA.z;
	float4 texB = gMainTex.Sample(gSampler, uvwB.xy) * uvwB.z;
	
    float4 posH = mul(pin.PosW, gViewProj);
     
    pOut.Buffer1 = (texA + texB) * _Color;
    pOut.Buffer2 = PackNormal(mul(normalize(float3(-(dhA.x + dhB.x), 1.0f, -(dhA.y + dhB.y))), (float3x3)gView));
    
    pOut.Buffer3.rg = PackMotion(posH, pin.PrevPosH);
	pOut.Buffer3.zw = float2(0.0f, 0.7f);

    return pOut;
}

#endif