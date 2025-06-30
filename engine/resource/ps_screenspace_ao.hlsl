#include "vs_drawscreen.hlsl"

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
 
// Nonnumeric values cannot be added to a cbuffer.
Texture2D gNormalMap    : register(t0);
Texture2D gDepthMap     : register(t1);

SamplerState gsamPointClamp : register(s0);
SamplerState gsamLinearClamp : register(s1);
SamplerState gsamDepthMap : register(s2);
SamplerState gsamLinearWrap : register(s3);

// discontinuous pseudorandom uniformly distributed in [-0.5, +0.5]^3
float3 rand3(float3 c) {
	float j = 4096.0f * sin(dot(c, float3(17.0f, 59.4f, 15.0f)));
	float3 r;
	r.z = frac(512.0f * j);
	j *= 0.125f;
	r.x = frac(512.0f * j);
	j *= 0.125f;
	r.y = frac(512.0f * j);
	return r - 0.5f;
}	

// Determines how much the sample point q occludes the point p as a function
// of distZ.
float OcclusionFunction(float distZ)
{
	float occlusion = 0.0f;
	if(distZ > gSurfaceEpsilon)
	{
		float fadeLength = gOcclusionFadeEnd - gOcclusionFadeStart;
		occlusion = saturate((gOcclusionFadeEnd - distZ) / fadeLength);
	}
	
	return occlusion;	
}

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
 
float4 PS(VertexOut pin) : SV_Target
{
	// Extract random vector and map from [0,1] --> [-1, +1].
	float3 randVec = 2.0f * rand3(float3(pin.TexC, 0.0f));

	float3 n = ReconstructNormal(gNormalMap.Sample(gsamPointClamp, pin.TexC).xy);
	float pz = gDepthMap.Sample(gsamDepthMap, pin.TexC).r;
	pz = NdcDepthToViewDepth(pz);
	
	// Transform quad corners to view space near plane.
	float4 ph = mul(pin.PosV, gInvProj);
	float3 PosV = ph.xyz / ph.w;

	float3 p = (pz / PosV.z) * PosV;

	float occlusionSum = 0.0f;
	for (int i = 0; i < KERNEL_SIZE; ++i)
	{
		float3 offset = reflect(gOffsetVectors[i].xyz, randVec);
		float flip = sign(dot(offset, n));

        float3 q = p + gOcclusionRadius * offset * flip;
		float4 projQ = mul(float4(q, 1.0f), gProjTex);
		projQ /= projQ.w;

		float rz = gDepthMap.Sample(gsamDepthMap, projQ.xy).r;
		rz = NdcDepthToViewDepth(rz);

		float3 r = (rz / q.z) * q;
		
		float distZ = p.z - r.z;
		float dp = max(dot(n, normalize(r - p)), 0.0f);

		float occlusion = dp * OcclusionFunction(distZ);
		occlusionSum += occlusion;
	}
	
	occlusionSum /= KERNEL_SIZE;
	float access = 1.0f - occlusionSum;

	return saturate(pow(access, 6.0f));
}