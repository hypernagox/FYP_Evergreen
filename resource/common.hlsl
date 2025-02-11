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

cbuffer cbBones : register(b2)
{
    uint gFrameStride;
    uint gSubmeshIndex;
    uint2 gFrameIndex;
    float2 gFrameFrac;
    uint2 gTransitionFrameIndex;
    float2 gTransitionFrameFrac;
    float2 gTransitionFactor;
};

cbuffer cbPerShadow : register(b3)
{
    float4x4 gLightViewProj[4];
    float4x4 gLightViewProjClip[4];
    float4 gShadowDistance;
    float4 gShadowBias;
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

Texture2D gMainTex : register(t0);
Texture2D gNormalMap : register(t1);
StructuredBuffer<float4x4> gBoneMatrices : register(t2);
Texture2D gShadowMap : register(t3);
Texture2D gSSAOMap : register(t4);

SamplerState gSampler : register(s0);

struct VertexIn
{
	float3 PosL         : POSITION;
    float2 Tex          : TEXCOORD;
    float3 Normal       : NORMAL;
    float3 Tangent	    : TANGENT;
#ifdef RIGGED
	uint   BoneIndices  : BONEINDICES;
	float4 BoneWeights  : BONEWEIGHTS;
#endif
#ifdef GENERATE_SHADOWS
	uint   InstanceID   : SV_InstanceID;
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
#ifdef GENERATE_SHADOWS
    float4 PosP         : POSITION3;
#endif
};

struct PixelOut
{
	float4 Buffer1 : SV_TARGET0;
    float2 Buffer2 : SV_TARGET1;
    float2 Buffer3 : SV_TARGET2;
};

#ifdef RIGGED

#define LocalToObjectPos(vin) RigTransform(float4(vin.PosL, 1.0f), 0, vin.BoneIndices, vin.BoneWeights)
#define LocalToObjectNormal(vin, normal) RigTransform(float4(normal, 0.0f), 0, vin.BoneIndices, vin.BoneWeights)

inline float4x4 BoneTransform(uint i, uint boneIndex)
{
    float4x4 boneOffset = gBoneMatrices[gSubmeshIndex * gFrameStride + boneIndex];
    float4x4 currFrame = lerp(gBoneMatrices[gFrameIndex[i] * gFrameStride + boneIndex], gBoneMatrices[(gFrameIndex[i] + 1) * gFrameStride + boneIndex], gFrameFrac[i]);
    float4x4 prevFrame = lerp(gBoneMatrices[gTransitionFrameIndex[i] * gFrameStride + boneIndex], gBoneMatrices[(gTransitionFrameIndex[i] + 1) * gFrameStride + boneIndex], gTransitionFrameFrac[i]);
    return mul(boneOffset, lerp(prevFrame, currFrame, gTransitionFactor[i]));
}

inline float4 RigTransform(float4 posL, uint i, uint indices, float4 weights)
{
	float4 posW =  mul(posL, BoneTransform(i, indices & 0xFF))         * weights.x;
	       posW += mul(posL, BoneTransform(i, indices >> 8 & 0xFF))    * weights.y;
	       posW += mul(posL, BoneTransform(i, indices >> 16 & 0xFF))   * weights.z;
	       posW += mul(posL, BoneTransform(i, indices >> 24 & 0xFF))   * weights.w;
	return posW;
}

#else
#define LocalToObjectPos(vin) float4(vin.PosL, 1.0f)
#define LocalToObjectNormal(vin, normal) float4(normal, 0.0f)

#endif

#define ObjectToWorldPos(pos) mul(pos, gWorld)
#define ObjectToWorldNormal(normal) float4(LocalToWorldNormal(normal.xyz), 0.0f)

#ifdef GENERATE_SHADOWS
#define WorldToClipPos(pos, vin) mul(pos, gLightViewProjClip[vin.InstanceID % 4]);

#else
#define WorldToClipPos(pos, vin) mul(mul(pos, gView), gProj)

#endif

#define ObjectToClipPos(pos) WorldToClipPos(ObjectToWorldPos(pos))

inline float3 LocalToWorldNormal(float3 normalL)
{
	return normalize(mul(normalL, (float3x3)gWorld));
}

#ifdef GENERATE_SHADOWS
#define ConstructPosP(vin, vout) vout.PosP = mul(vout.PosW, gLightViewProj[vin.InstanceID % 4])
#define ConstructSSAOPosH(vin, vout) vout.SSAOPosH = float4(0.0f, 0.0f, 0.0f, 1.0f)
#define ConstructPrevPosH(vin, vout) vout.PrevPosH = float4(0.0f, 0.0f, 0.0f, 1.0f)

#else
#define ConstructPosP(vin, vout)
#ifdef RIGGED
#define ConstructPrevPosH(vin, vout) vout.PrevPosH = mul(mul(RigTransform(float4(vin.PosL, 1.0f), 1, vin.BoneIndices, vin.BoneWeights), gPrevWorld), gPrevViewProj)
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
    ConstructPosP(vin, vout);                                                       \
    ConstructPrevPosH(vin, vout);                                                   \

#if defined(GENERATE_SHADOWS) && !defined(USE_CUSTOM_SHADOWPS)

void ShadowPS(VertexOut pin)
{
    float a = gMainTex.Sample(gSampler, pin.Tex).a;
    clip(a - 0.1f);
    clip(1.0f - max(abs(pin.PosP.x), abs(pin.PosP.y)));
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
    float3 N = normalW;
    float3 T = normalize(tangentW - dot(tangentW, N) * N);
    float3 B = cross(N, T);

    float3x3 TBN = float3x3(T, B, N);
    return mul(normalSample * 2.0f - 1.0f, TBN);
}

float2 PackNormal(float3 n)
{
	return normalize(n.xy) * sqrt(n.z * 0.5f + 0.5f);
}