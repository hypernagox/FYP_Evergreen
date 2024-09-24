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
 
static const float2 gTexCoords[6] =
{
	float2(0.0f, 1.0f),
	float2(0.0f, 0.0f),
	float2(1.0f, 0.0f),
	float2(0.0f, 1.0f),
	float2(1.0f, 0.0f),
	float2(1.0f, 1.0f)
};
 
struct VertexOut
{
	float4 PosH : SV_POSITION;
	float3 PosV : POSITION;
	float2 TexC : TEXCOORD;
};

VertexOut VS(uint vid : SV_VertexID)
{
	VertexOut vout;

	vout.TexC = gTexCoords[vid];

	// Quad covering screen in NDC space.
	vout.PosH = float4(2.0f * vout.TexC.x - 1.0f, 1.0f - 2.0f * vout.TexC.y, 0.0f, 1.0f);
 
	// Transform quad corners to view space near plane.
	float4 ph = mul(vout.PosH, gInvProj);
	vout.PosV = ph.xyz / ph.w;

	return vout;
}

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
		occlusion = saturate( (gOcclusionFadeEnd-distZ)/fadeLength );
	}
	
	return occlusion;	
}

float NdcDepthToViewDepth(float z_ndc)
{
	float viewZ = gProj[3][2] / (z_ndc - gProj[2][2]);
	return viewZ;
}
 
float4 PS(VertexOut pin) : SV_Target
{
	// Extract random vector and map from [0,1] --> [-1, +1].
	float3 randVec = 2.0f * rand3(float3(pin.TexC, 0.0f));

	float3 n = normalize(gNormalMap.SampleLevel(gsamPointClamp, pin.TexC, 0.0f).xyz);
	float3 u = normalize(randVec - n * dot(randVec, n));
	float3 v = cross(n, u);
	float3x3 tbn = float3x3(u, v, n);

	float pz = gDepthMap.SampleLevel(gsamDepthMap, pin.TexC, 0.0f).r;
	pz = NdcDepthToViewDepth(pz);

	float3 p = (pz / pin.PosV.z) * pin.PosV;

	float occlusionSum = 0.0f;
	for (int i = 0; i < KERNEL_SIZE; ++i)
	{
		float3 offset = mul(gOffsetVectors[i].xyz, tbn);
        float3 sample = p + gOcclusionRadius * offset;

		float4 projQ = mul(float4(sample, 1.0f), gProjTex);
		projQ /= projQ.w;

		float rz = gDepthMap.SampleLevel(gsamDepthMap, projQ.xy, 0.0f).r;
		rz = NdcDepthToViewDepth(rz);

		float3 r = (rz / sample.z) * sample;
		
		float distZ = p.z - r.z;
		float dp = max(dot(n, normalize(r - p)), 0.0f);

		float occlusion = dp * OcclusionFunction(distZ);
		occlusionSum += occlusion;
	}
	
	occlusionSum /= KERNEL_SIZE;
	float access = 1.0f - occlusionSum;

	return saturate(pow(access, 6.0f));
}