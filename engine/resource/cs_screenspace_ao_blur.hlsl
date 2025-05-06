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
		
[numthreads(128, 1, 1)]
void CS(int3 id : SV_DispatchThreadID)
{
    uint width, height, srcWidth, srcHeight;
	gSrcTex.GetDimensions(srcWidth, srcHeight);
	gDstTex.GetDimensions(width, height);

	int2 dstID = id.xy;
	float2 texOffset = float2(1.0f, 0.0f) / float(srcWidth);
	if (gOrientation)
	{
		dstID = id.yx;
		texOffset = float2(0.0f, 1.0f) / float(srcHeight);
	}

	float2 uv = (float2(dstID) + 0.5f) / float2(width, height);
	float4 color = gSrcTex.SampleLevel(gsamPointClamp, uv, 0) * gBlurWeights[0][0];
	float weightSum = gBlurWeights[0][0];

	float3 n = ReconstructNormal(gNormalMap.SampleLevel(gsamPointClamp, uv, 0).xy);
	float p = NdcDepthToViewDepth(gDepthMap.SampleLevel(gsamPointClamp, uv, 0).x);
			
	[unroll]
	for (int i = 1 - BLUR_SAMPLE; i < BLUR_SAMPLE; ++i)
	{
		if (i == 0)
			continue;

		float2 offset = texOffset * float(i);
		float4 sample = gSrcTex.SampleLevel(gsamPointClamp, uv + offset, 0);

		float3 np = ReconstructNormal(gNormalMap.SampleLevel(gsamPointClamp, uv + offset, 0).xy);
		float pp = NdcDepthToViewDepth(gDepthMap.SampleLevel(gsamPointClamp, uv + offset, 0).x);

        if (dot(n, np) > 0.8f && abs(p - pp) < 0.2f)
		{
			int wi = abs(i);
            float weight = gBlurWeights[wi / 4][wi % 4];
			color += sample * weight;
			weightSum += weight;
		}
	}
	gDstTex[dstID.xy] = color / weightSum;
}