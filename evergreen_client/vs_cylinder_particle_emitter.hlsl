#include "inc_sphere_particle_emitter.hlsl"

struct VertexOut
{
    float4 PosW         : SV_POSITION;
	float3 Normal       : NORMAL0;
	float  Life			: TEXCOORD0;
	uint   Vid          : TEXCOORD1;
};

VertexOut VS(uint vid : SV_VERTEXID)
{
	VertexOut vout;

	vout.Life = lerp(gLifeTimeMin, gLifeTimeMax, rand3(vid * VIDMUL + 0.1f).x + 0.5f);
	vout.Vid = vid;

	float currentLife = fmod(gElapsedTime, vout.Life);
	float lifeFactor = currentLife / vout.Life;

	// Calculate position based on the sphere emitter parameters
	vout.Normal = float3(0.0f, 0.0f, 1.0f);
	float speed = lerp(gSpeedMin, gSpeedMax, rand3(vid * VIDMUL).y + 0.5f);

	// Transform position to homogeneous clip space
	float4 posL = float4(0.0f, 0.0f, 0.0f, 1.0f);
	posL.xy = normalize(rand3(vid * VIDMUL)).xy;
	if (gSpeedLifeExp < 0.0f) {
		posL.xyz += vout.Normal * speed * vout.Life * pow(1.0f - lifeFactor, -gSpeedLifeExp);
	}
	else {
	    posL.xyz += vout.Normal * speed * vout.Life * pow(lifeFactor, gSpeedLifeExp);
	}

	vout.PosW = mul(posL, gWorld);
	vout.PosW.w = 1.0f;
	vout.Normal = normalize(mul(vout.Normal, (float3x3)gWorld));

	return vout;
}