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

cbuffer cbPerShadow : register(b2)
{
    float4x4 gLightViewProj[4];
    float4x4 gLightViewProjClip[4];
    float4 gShadowDistance;
    float3 gDirLight;
};

cbuffer cbPerFrame : register(b3)
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
Texture2D gShadowMap : register(t1);
Texture2D gSSAOMap : register(t2);

SamplerState gSampler : register(s0);
SamplerComparisonState gSamplerShadow : register(s1);

float ShadowValue(float4 posW, float3 normalW, float distanceH)
{
    float4 shadowPosH = (float4)0.0f;
    if (distanceH < gShadowDistance[0]) {
        shadowPosH = mul(mul(posW, gLightViewProjClip[0]), gTex);
	}
	else if (distanceH < gShadowDistance[1]) {
        shadowPosH = mul(mul(posW, gLightViewProjClip[1]), gTex);
	}
	else if (distanceH < gShadowDistance[2]) {
        shadowPosH = mul(mul(posW, gLightViewProjClip[2]), gTex);
	}
	else if (distanceH < gShadowDistance[3]) {
        shadowPosH = mul(mul(posW, gLightViewProjClip[3]), gTex);
    }
    else {
        return 1.0f;
    }
    
    // Complete projection by doing division by w.
    shadowPosH.xyz /= shadowPosH.w;
    
    float percentLit = 0.0f;
    if (max(shadowPosH.x, shadowPosH.y) < 1.0f && min(shadowPosH.x, shadowPosH.y) > 0.0f)
    {
        // Depth in NDC space.
        float depth = shadowPosH.z;

        uint width, height, numMips;
        gShadowMap.GetDimensions(0, width, height, numMips);

        // Texel size.
        float dx = 1.0f / (float) width;
        float tanLight = abs(length(cross(normalW, -gDirLight)) / dot(normalW, -gDirLight));
        float bias = 0.0003f * tanLight;

        const float2 offsets[9] =
        {
            float2(-dx, -dx), float2(0.0f, -dx), float2(dx, -dx),
            float2(-dx, 0.0f), float2(0.0f, 0.0f), float2(dx, 0.0f),
            float2(-dx, +dx), float2(0.0f, +dx), float2(dx, +dx)
        };

        [unroll]
        for (int i = 0; i < 9; ++i)
        {
            percentLit += gShadowMap.SampleCmpLevelZero(gSamplerShadow,
            shadowPosH.xy + offsets[i], depth - bias).r;
        }
    }
    else
    {
        percentLit = 9.0f;
    }
    
    return percentLit / 9.0f;
}