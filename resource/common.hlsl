#define MAX_BONES 256

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
};

static const float4x4 gTex =
{
        0.5f, 0.0f, 0.0f, 0.0f,
	    0.0f, -0.5f, 0.0f, 0.0f,
	    0.0f, 0.0f, 1.0f, 0.0f,
	    0.5f, 0.5f, 0.0f, 1.0f
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
    float2 Buffer3 : SV_TARGET2;
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

float GetDitherThreshold(float2 fragCoord)
{
    int x = (int)fmod(fragCoord.x, 8.0);
    int y = (int)fmod(fragCoord.y, 8.0);
    int index = y * 8 + x;
    return Bayer8x8[index];
}