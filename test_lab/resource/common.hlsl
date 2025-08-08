#define MAX_BONES 256

static const float4x4 gTex =
{
        0.5f, 0.0f, 0.0f, 0.0f,
	    0.0f, -0.5f, 0.0f, 0.0f,
	    0.0f, 0.0f, 1.0f, 0.0f,
	    0.5f, 0.5f, 0.0f, 1.0f
};

#ifndef DEFERRED

cbuffer cbPerObject : register(b0)
{
    float4x4 gWorld;
    float4x4 gPrevWorld;
};

cbuffer cbPerCamera : register(b1)
{
    float4x4 gView;
    float4x4 gProj;
    float4x4 gViewProj;
    float4x4 gViewInverse;
    float4x4 gProjInverse;
    float4x4 gViewProjInverse;
	float4x4 gPrevViewProj;
    float4 gEyePosW;
    float2 gRenderTargetSize;
}

struct BoneData { float4x4 m[MAX_BONES]; };
ConstantBuffer<BoneData> gBoneTransforms : register(b2, space0);
ConstantBuffer<BoneData> gPrevBoneTransforms : register(b2, space1);

cbuffer cbPerShadow : register(b3)
{
    float4x4 gLightViewProj[4];
    float4x4 gLightViewProjClip[4];
	float4 gLightPosW[4];
    float4 gShadowDistance;
    float3 gDirLight;
};

cbuffer cbPerFrame : register(b4)
{
    float gTime;
    float gDeltaTime;
    float gMotionBlurFactor;
    float gMotionBlurRadius;
	float4 gFogColor;
	float4 gFogSunColor;
	float gFogDensity;
	float gFogHeightFalloff;
	float gFogDistanceStart;
};

static const float Bayer8x8[64] =
{
    0.0000, 0.7500, 0.1875, 0.9375, 0.0469, 0.7969, 0.2344, 0.9844,
    0.5000, 0.2500, 0.6875, 0.4375, 0.5469, 0.2969, 0.7344, 0.4844,
    0.1250, 0.8750, 0.0625, 0.8125, 0.1719, 0.9219, 0.1094, 0.8594,
    0.6250, 0.3750, 0.5625, 0.3125, 0.6719, 0.4219, 0.6094, 0.3594,
    0.0312, 0.7812, 0.2188, 0.9688, 0.0156, 0.7656, 0.2031, 0.9531,
    0.5312, 0.2812, 0.7188, 0.4688, 0.5156, 0.2656, 0.7031, 0.4531,
    0.1562, 0.9062, 0.0938, 0.8438, 0.1406, 0.8906, 0.0781, 0.8281,
    0.6562, 0.4062, 0.5938, 0.3438, 0.6406, 0.3906, 0.5781, 0.3281
};

Texture2D gMainTex : register(t0);

SamplerState gSampler : register(s0);

struct VertexIn
{
	float3 PosL         : POSITION;
    float2 Tex          : TEXCOORD;
    float3 Normal       : NORMAL;
    float3 Tangent	    : TANGENT;
    float4x4 InstanceTransform : INSTANCETRANSFORM;
#ifdef RIGGED
	uint   BoneIndices  : BONEINDICES;
	float4 BoneWeights  : BONEWEIGHTS;
#endif
};

struct VertexOut
{
	float4 PosH         : SV_POSITION;
    float4 PosW         : POSITION0;
    float2 Tex          : TEXCOORD;
    float4 NormalW      : NORMAL;
    float4 TangentW     : TANGENT;
    float4 PrevPosH     : POSITION2;
};

struct PixelOut
{
	float4 Buffer1 : SV_TARGET0;
    float2 Buffer2 : SV_TARGET1;
    float4 Buffer3 : SV_TARGET2;
};

#ifdef RIGGED
#define LocalToObjectPos(vin) RigTransform(float4(vin.PosL, 1.0f), vin.BoneIndices, vin.BoneWeights)
#define LocalToObjectNormal(vin, normal) RigTransform(float4(normal, 0.0f), vin.BoneIndices, vin.BoneWeights)

inline float4 RigTransform(float4 posL, uint indices, float4 weights)
{
	float4 posW =  mul(posL, gBoneTransforms.m[indices & 0xFF])         * weights.x;
	       posW += mul(posL, gBoneTransforms.m[indices >> 8 & 0xFF])    * weights.y;
	       posW += mul(posL, gBoneTransforms.m[indices >> 16 & 0xFF])   * weights.z;
	       posW += mul(posL, gBoneTransforms.m[indices >> 24 & 0xFF])   * weights.w;
	return posW;
}

inline float4 PrevRigTransform(float4 posL, uint indices, float4 weights)
{
	float4 posW =  mul(posL, gPrevBoneTransforms.m[indices & 0xFF])         * weights.x;
	       posW += mul(posL, gPrevBoneTransforms.m[indices >> 8 & 0xFF])    * weights.y;
	       posW += mul(posL, gPrevBoneTransforms.m[indices >> 16 & 0xFF])   * weights.z;
	       posW += mul(posL, gPrevBoneTransforms.m[indices >> 24 & 0xFF])   * weights.w;
	return posW;
}

#else
#define LocalToObjectPos(vin) float4(vin.PosL, 1.0f)
#define LocalToObjectNormal(vin, normal) float4(normal, 0.0f)

#endif

#define ObjectToWorldPos(pos) mul(pos, gWorld)
#define ObjectToWorldNormal(normal) float4(LocalToWorldNormal(normal.xyz), 0.0f)

#define WorldToClipPos(pos, vin) mul(mul(pos, gView), gProj)
#define ObjectToClipPos(pos) WorldToClipPos(ObjectToWorldPos(pos))

inline float3 LocalToWorldNormal(float3 normalL)
{
	return normalize(mul(normalL, (float3x3)gWorld));
}

#ifdef GENERATE_SHADOWS
#define ConstructSSAOPosH(vin, vout) vout.SSAOPosH = float4(0.0f, 0.0f, 0.0f, 1.0f)
#define ConstructPrevPosH(vin, vout) vout.PrevPosH = float4(0.0f, 0.0f, 0.0f, 1.0f)

#else
#ifdef RIGGED
#define ConstructPrevPosH(vin, vout) vout.PrevPosH = mul(mul(PrevRigTransform(float4(vin.PosL, 1.0f), vin.BoneIndices, vin.BoneWeights), gPrevWorld), gPrevViewProj)
#else
#define ConstructPrevPosH(vin, vout) vout.PrevPosH = mul(mul(float4(vin.PosL, 1.0f), gPrevWorld), gPrevViewProj)
#endif

#endif

#define ConstructVSOutput(vin, vout)                                                \
	vout.PosW = ObjectToWorldPos(LocalToObjectPos(vin));                            \
	vout.PosH = WorldToClipPos(vout.PosW, vin);                                     \
	vout.Tex = vin.Tex;                                                             \
	vout.NormalW = ObjectToWorldNormal(LocalToObjectNormal(vin, vin.Normal));       \
    vout.TangentW = ObjectToWorldNormal(LocalToObjectNormal(vin, vin.Tangent));     \
    ConstructPrevPosH(vin, vout);                                                   \

#if defined(GENERATE_SHADOWS) && !defined(USE_CUSTOM_SHADOWPS)

void ShadowPS(VertexOut pin)
{
    float a = gMainTex.Sample(gSampler, pin.Tex).a;
    clip(a - 0.1f);
}

#endif

float SpecularValue(float posW, float3 normalW)
{
	float3 R = reflect(gDirLight, normalW);
	float spec = pow(max(0.0f, dot(R, normalize(gEyePosW.xyz - posW))), 8.0f);
	return spec;
}

float3 NormalSampleToWorldSpace(float3 normalSample, float3 normalW, float3 tangentW)
{
    float3 normalT = normalize(normalSample * 2.0f - 1.0f);

    float3 N = normalW;
    float3 T = normalize(tangentW - dot(tangentW, N) * N);
    float3 B = cross(N, T);

    float3x3 TBN = float3x3(T, B, N);
    return mul(normalT, TBN);
}

float2 PackNormal(float3 n)
{
	return normalize(n.xy) * sqrt(n.z * 0.5f + 0.5f);
}

float2 PackMotion(float4 posH, float4 prevPosH)
{
	float2 motion = (posH.xy / posH.w) - (prevPosH.xy / prevPosH.w);
	motion *= gMotionBlurFactor * 0.5f * gRenderTargetSize / gMotionBlurRadius;
	motion /= max(length(motion), 1.0f);
	return motion;
}

float GetDitherThreshold(float2 fragCoord)
{
    int x = (int)fmod(fragCoord.x, 8.0);
    int y = (int)fmod(fragCoord.y, 8.0);
    int index = y * 8 + x;
    return Bayer8x8[index];
}

#endif

#ifdef DEFERRED

cbuffer cbPerCamera : register(b0)
{
	float4x4 gView;
	float4x4 gProj;
	float4x4 gViewProj;
	float4x4 gViewInverse;
	float4x4 gProjInverse;
	float4x4 gViewProjInverse;
	float4x4 gPrevViewProj;
	float4 gEyePosW;
}

cbuffer cbPerShadow : register(b1)
{
	float4x4 gLightViewProj[4];
	float4x4 gLightViewProjClip[4];
	float4 gLightPosW[4];
	float4 gShadowDistance;
	float3 gDirLight;
};

cbuffer cbPerFrame : register(b2)
{
    float gTime;
    float gDeltaTime;
    float gMotionBlurFactor;
    float gMotionBlurRadius;
	float4 gFogColor;
	float4 gFogSunColor;
	float gFogDensity;
	float gFogHeightFalloff;
	float gFogDistanceStart;
};

// Nonnumeric values cannot be added to a cbuffer.
Texture2D gBuffer1    : register(t0);
Texture2D gBuffer2    : register(t1);
Texture2D gBuffer3    : register(t2);
Texture2D gShadowMap  : register(t3);
Texture2D gSSAOMap	  : register(t4);
Texture2D gBufferDSV  : register(t5);

SamplerState gsamPointClamp : register(s0);
SamplerState gsamLinearClamp : register(s1);
SamplerState gsamDepthMap : register(s2);
SamplerState gsamLinearWrap : register(s3);
SamplerComparisonState gSamplerShadow : register(s4);

struct VertexOut
{
	float4 PosH : SV_POSITION;
	float4 PosV : POSITION;
	float2 TexC : TEXCOORD;
	uint InstanceID : SV_InstanceID;
};

float ShadowValue(float4 posW, float3 normalW, int level, float bias = 0.0f)
{
	float4 shadowPosH = mul(mul(posW, gLightViewProjClip[level]), gTex);

	// Complete projection by doing division by w.
	shadowPosH.xy /= shadowPosH.w;
    
	float percentLit = 0.0f;
	if (max(shadowPosH.x, shadowPosH.y) < 1.0f && min(shadowPosH.x, shadowPosH.y) > 0.0f)
	{
		// Depth in NDC space.
		float depth = shadowPosH.z - bias;

		uint width, height, numMips;
		gShadowMap.GetDimensions(0, width, height, numMips);

		// Texel size.
		float dx = 1.0f / (float)width;
		const float2 offsets[9] = {
			float2(-dx, -dx), float2(0.0f, -dx), float2(dx, -dx),
			float2(-dx, 0.0f), float2(0.0f, 0.0f), float2(dx, 0.0f),
			float2(-dx, +dx), float2(0.0f, +dx), float2(dx, +dx)
		};

		[unroll]
		for (int i = 0; i < 9; ++i)
			percentLit += gShadowMap.SampleCmpLevelZero(gSamplerShadow, shadowPosH.xy + offsets[i], depth).r;
	}
	else
	{
		percentLit = 9.0f;
	}
    
	return percentLit / 9.0f;
}

float ShadowValue(float4 posW, float3 normalW, float bias = 0.0f)
{
	float distanceW0 = length(posW.xyz - gLightPosW[0].xyz);
	float distanceW1 = length(posW.xyz - gLightPosW[1].xyz);
	float distanceW2 = length(posW.xyz - gLightPosW[2].xyz);
	float distanceW3 = length(posW.xyz - gLightPosW[3].xyz);
	if (distanceW0 < gShadowDistance[0])
		return lerp(ShadowValue(posW, normalW, 0, bias), ShadowValue(posW, normalW, 1, bias), saturate((distanceW0 - gShadowDistance[0]) / (0.1f * gShadowDistance[0]) + 1.0f));
	else if (distanceW1 < gShadowDistance[1])
		return lerp(ShadowValue(posW, normalW, 1, bias), ShadowValue(posW, normalW, 2, bias), saturate((distanceW1 - gShadowDistance[1]) / (0.1f * (gShadowDistance[1] - gShadowDistance[0])) + 1.0f));
	else if (distanceW2 < gShadowDistance[2])
		return lerp(ShadowValue(posW, normalW, 2, bias), ShadowValue(posW, normalW, 3, bias), saturate((distanceW2 - gShadowDistance[2]) / (0.1f * (gShadowDistance[2] - gShadowDistance[1])) + 1.0f));
	else if (distanceW3 < gShadowDistance[3])
		return lerp(ShadowValue(posW, normalW, 3, bias), 1.0f, saturate((distanceW3 - gShadowDistance[3]) / (0.1f * (gShadowDistance[3] - gShadowDistance[2])) + 1.0f));
	else
		return 1.0f;
}

static const float gamma = 2.2f;

float3 ReconstructNormal(float2 np)
{
	float3 n;
	n.z = dot(np, np) * 2.0f - 1.0f;
	n.xy = normalize(np) * sqrt(1.0f - n.z * n.z);
	return n;
}

float3 AmbientLight(VertexOut pin)
{
	// Sky color. #133771
	float3 skyColor = pow(float3(0.357f, 0.404f, 0.467f), gamma);
	float aoFactor = gSSAOMap.Sample(gsamPointClamp, pin.TexC).r;

	return skyColor.rgb * aoFactor;
}

float3 DiffuseLight(VertexOut pin)
{
	float3 normalV = ReconstructNormal(gBuffer2.Sample(gsamPointClamp, pin.TexC).xy);
	float3 normalW = normalize(mul(normalV, transpose((float3x3)gView)));

    float3 lightColor = 1.0f;
	float lambertian = max(dot(normalW, -gDirLight), 0.0);
	float lightPower = 2.0f;
	return lightColor * lambertian * lightPower;
}

float3 ApplyFog(float3 col, float3 worldPos)
{
    float distance = max(0.0f, length(worldPos - gEyePosW.xyz) - gFogDistanceStart);
	float3 direction = normalize(worldPos - gEyePosW.xyz);
    float sunAmount = max(dot(direction, -gDirLight), 0.0f);

	float fogAmount = saturate((gFogHeightFalloff * gFogDensity) * exp(-(gEyePosW.y + gFogDistanceStart * direction.y) / gFogDensity) * (1.0f - exp(-distance * direction.y / gFogDensity)) / direction.y);
    float3 fogColor = lerp(gFogColor.rgb, gFogSunColor.rgb, pow(sunAmount, 2.0f));
	
    return lerp(col, fogColor, fogAmount);
}

float4 PSDeferredDefault(VertexOut pin) : SV_Target
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

	float3 fColor = (AmbientLight(pin) + min(ShadowValue(PosW, normalW), DiffuseLight(pin))) * gBuffer1Color.rgb;
	fColor = ApplyFog(fColor, PosW.xyz);
	return float4(fColor, 1.0f);
}

#endif