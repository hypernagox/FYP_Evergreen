#define THREAD_SIZE 32

cbuffer cbSsao : register(b0)
{
	float4x4 gProj;
	float4x4 gInvProj;
	float4x4 gProjTex;
	float4   gOffsetVectors[KERNEL_SIZE];

	// Coordinates given in view space.
	float    gOcclusionRadius;
	float    gOcclusionFadeStart;
	float    gOcclusionFadeEnd;
	float    gSurfaceEpsilon;
};

cbuffer cbBlurState : register(b1)
{
	bool gOrientation;
};

cbuffer cbBlur : register(b2)
{
	float4 gBlurWeights[BLUR_SAMPLE / 4];
};

Texture2D gSrcTex : register(t0);
Texture2D gNormalMap : register(t1);
Texture2D gDepthMap : register(t2);
RWTexture2D<float> gDstTex : register(u0);

SamplerState gsamPointClamp : register(s0);
SamplerState gsamLinearClamp : register(s1);
SamplerState gsamDepthMap : register(s2);
SamplerState gsamLinearWrap : register(s3);

float NdcDepthToViewDepth(float z_ndc)
{
	float viewZ = gProj[3][2] / (z_ndc - gProj[2][2]);
	return viewZ;
}

float3 ReconstructNormal(float2 np)
{
	float3 n;
	n.z = dot(np, np) * 2.0f - 1.0f;
	n.xy = normalize(np) * sqrt(1.0f - n.z * n.z);
	return n;
}

// 20 Bytes per kernel. Expected to take up 1'280 Bytes of shared memory.
groupshared float gColorCache[THREAD_SIZE + BLUR_SAMPLE * 2];
groupshared float3 gNormalCache[THREAD_SIZE + BLUR_SAMPLE * 2];
groupshared float gDepthCache[THREAD_SIZE + BLUR_SAMPLE * 2];
		
[numthreads(THREAD_SIZE, 1, 1)]
void CS(int3 id : SV_DISPATCHTHREADID, int3 tid : SV_GROUPTHREADID)
{
    uint width, height;
	gDstTex.GetDimensions(width, height);

	int3 dstID = gOrientation ? id.yxz : id.xyz;
	int3 offsetID = int3(1 - gOrientation, gOrientation, 0);
	int cacheID = tid.x + BLUR_SAMPLE;
	
	// Store the cache value.
	gColorCache[cacheID] = gSrcTex.Load(dstID).r;
	gNormalCache[cacheID] = ReconstructNormal(gNormalMap.Load(dstID).rg);
	gDepthCache[cacheID] = NdcDepthToViewDepth(gDepthMap.Load(dstID).r);

	if (tid.x < BLUR_SAMPLE)
	{
	    int3 dstpID = max(dstID - offsetID * BLUR_SAMPLE, int3(0, 0, 0));
		gColorCache[cacheID - BLUR_SAMPLE] = gSrcTex.Load(dstpID).r;
		gNormalCache[cacheID - BLUR_SAMPLE] = ReconstructNormal(gNormalMap.Load(dstpID).rg);
		gDepthCache[cacheID - BLUR_SAMPLE] = NdcDepthToViewDepth(gDepthMap.Load(dstpID).r);
	}
	if (tid.x >= THREAD_SIZE - BLUR_SAMPLE)
	{
	    int3 dstpID = min(dstID + offsetID * BLUR_SAMPLE, int3(width - 1, height - 1, 0));
		gColorCache[cacheID + BLUR_SAMPLE] = gSrcTex.Load(dstpID).r;
		gNormalCache[cacheID + BLUR_SAMPLE] = ReconstructNormal(gNormalMap.Load(dstpID).rg);
		gDepthCache[cacheID + BLUR_SAMPLE] = NdcDepthToViewDepth(gDepthMap.Load(dstpID).r);
	}

	float colorSum = gColorCache[cacheID] * gBlurWeights[0][0];
	float weightSum = gBlurWeights[0][0];

	float3 n = gNormalCache[cacheID];
	float p = gDepthCache[cacheID];

	// Ensure all threads have written to the cache before proceeding.
	GroupMemoryBarrierWithGroupSync();
			
	[unroll]
	for (int i = 1 - BLUR_SAMPLE; i < BLUR_SAMPLE; ++i)
	{
		if (i == 0)
			continue;

		float3 np = gNormalCache[cacheID + i];
		float pp = gDepthCache[cacheID + i];

        if (dot(n, np) > 0.8f && abs(p - pp) < 0.2f)
		{
			int wi = abs(i);
            float weight = gBlurWeights[wi / 4][wi % 4];
			colorSum += gColorCache[cacheID + i] * weight;
			weightSum += weight;
		}
	}
	gDstTex[dstID.xy] = colorSum / weightSum;
}