cbuffer cbPerObject : register(b0)
{
    float4x4 gWorld;
};

cbuffer cbPerCamera : register(b1)
{
	float4x4 gView;
    float4x4 gProj;
    float4 gEyePosW;
}

cbuffer cbBones : register(b2)
{
	float4x4 gBones[128];
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
Texture2D gShadowMap : register(t2);
Texture2D gSSAOMap : register(t3);

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
    float4 SSAOPosH     : POSITION1;
    float2 Tex          : TEXCOORD;
    float4 NormalW      : NORMAL;
    float4 TangentW     : TANGENT;
#ifdef GENERATE_SHADOWS
    float4 PosP         : POSITION2;
#endif
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
	float4 posW =  mul(posL, gBones[indices & 0xFF])         * weights.x;
	       posW += mul(posL, gBones[indices >> 8 & 0xFF])    * weights.y;
	       posW += mul(posL, gBones[indices >> 16 & 0xFF])   * weights.z;
	       posW += mul(posL, gBones[indices >> 24 & 0xFF])   * weights.w;
	return posW;
}

#else
#define LocalToObjectPos(vin) float4(vin.PosL, 1.0f)
#define LocalToObjectNormal(vin, normal) float4(normal, 0.0f)

#endif

#define ObjectToWorldPos(pos) mul(pos, gWorld)
#define ObjectToWorldNormal(normal) float4(LocalToWorldNormal(normal.xyz), 0.0f)

#ifdef GENERATE_SHADOWS
#define WorldToClipPos(pos, vin) mul(pos, gLightViewProjClip[vin.InstanceID]);

#else
#define WorldToClipPos(pos, vin) mul(mul(pos, gView), gProj)

#endif

#define ObjectToClipPos(pos) WorldToClipPos(ObjectToWorldPos(pos))

inline float3 LocalToWorldNormal(float3 normalL)
{
	return normalize(mul(normalL, (float3x3)gWorld));
}

#ifdef GENERATE_SHADOWS
#define ConstructPosP(vin, vout) vout.PosP = mul(vout.PosW, gLightViewProj[vin.InstanceID])

#else
#define ConstructPosP(vin, vout)

#endif

#define ConstructVSOutput(vin, vout)                                \
	vout.PosW = ObjectToWorldPos(LocalToObjectPos(vin));            \
	vout.PosH = WorldToClipPos(vout.PosW, vin);                     \
	vout.SSAOPosH = mul(vout.PosH, gTex);                           \
	vout.Tex = vin.Tex;                                             \
	vout.NormalW = ObjectToWorldNormal(LocalToObjectNormal(vin, vin.Normal));       \
    vout.TangentW = ObjectToWorldNormal(LocalToObjectNormal(vin, vin.Tangent));     \
    ConstructPosP(vin, vout);                                       \

#ifdef GENERATE_SHADOWS

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