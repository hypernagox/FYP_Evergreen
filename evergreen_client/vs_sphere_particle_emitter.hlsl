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

	vout.Life = lerp(gLifeTimeMin, gLifeTimeMax, rand3(vid * VIDMUL - gEmitTime).x + 0.5f);
	vout.Vid = vid;

	float currentLife = fmod(gTime - gEmitTime, vout.Life);
	float lifeFactor = currentLife / vout.Life;

	// Calculate position based on the sphere emitter parameters
	vout.Normal = normalize(rand3(vid * VIDMUL));
	float speed = lerp(gSpeedMin, gSpeedMax, rand3(vid * VIDMUL - gEmitTime).y + 0.5f);

	// Transform position to homogeneous clip space
	float4 posL = float4(0.0f, 0.0f, 0.0f, 1.0f);
	if (gSpeedLifeExp < 0.0f) {
		posL.xyz += vout.Normal * speed * vout.Life * pow(1.0f - lifeFactor, -gSpeedLifeExp);
	}
	else {
	    posL.xyz += vout.Normal * speed * vout.Life * pow(lifeFactor, gSpeedLifeExp);
	}

	vout.PosW = mul(posL, gWorld);
	vout.PosW.w = 1.0f;

	return vout;
}