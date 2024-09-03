#include "common.hlsl"

struct VertexIn
{
	float3 PosL         : POSITION;
    float2 Tex          : TEXCOORD;
    float3 Normal       : NORMAL;
    float3 Tangent      : TANGENT;
};

struct VertexOut
{
	float4 PosH         : SV_POSITION;
    float4 PosW         : POSITION;
    float2 Tex          : TEXCOORD;
};

VertexOut VS(VertexIn vin)
{
	VertexOut vout;
	
	// Transform to homogeneous clip space.
    vout.PosW = mul(float4(vin.PosL, 1.0f), gWorld);
	vout.PosH = mul(mul(vout.PosW, gView), gProj);
    vout.Tex = vin.Tex;
    
    return vout;
}

float4 PS(VertexOut pin) : SV_Target
{
    return gMainTex.Sample(gSampler, pin.Tex);
}