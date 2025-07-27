#include "inc_sphere_particle_emitter.hlsl"

struct PixelIn
{
	float4 PosH         : SV_POSITION;
	float4 PosW         : POSITION0;
	float2 Tex          : TEXCOORD1;
	float  Alpha        : TEXCOORD2;
};

float4 PS(PixelIn pin) : SV_Target
{
    return float4(gWorld[0][3], gWorld[1][3], gWorld[2][3], gMainTex.Sample(gSampler, pin.Tex).r * pin.Alpha);
}