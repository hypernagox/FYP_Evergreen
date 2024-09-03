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
    float4 PosW         : POSITION0;
    float4 SSAOPosH     : POSITION1;
    float2 Tex          : TEXCOORD;
    float4 NormalW      : NORMAL;
};

VertexOut VS(VertexIn vin)
{
	VertexOut vout;
	
	// Transform to homogeneous clip space.
    vout.PosW = mul(float4(vin.PosL, 1.0f), gWorld);
	vout.PosH = mul(mul(vout.PosW, gView), gProj);
    vout.SSAOPosH = mul(vout.PosH, gTex);
	
	// Just pass vertex color into the pixel shader.
    vout.Tex = vin.Tex;
    vout.NormalW = mul(float4(vin.Normal, 0.0f), gWorld);
    
    return vout;
}

float4 PS(VertexOut pin) : SV_Target
{
    float3 normal = normalize(pin.NormalW.xyz);
    float4 texColor = gMainTex.Sample(gSampler, pin.Tex);
    
    clip(texColor.a - 0.1f);
    
    // Finish texture projection and sample SSAO map.
    pin.SSAOPosH /= pin.SSAOPosH.w;

    float ambient = 0.25f + gSSAOMap.Sample(gSampler, pin.SSAOPosH.xy, 0.0f).r * 0.75f;
    float diffuse = pow(saturate(dot(normal, -gDirLight) * 1.1f - 0.1f), 0.3f);
    float percentLit = ShadowValue(pin.PosW, normal, distance(pin.PosW, gEyePosW));
    
    texColor.rgb *= ambient * (min(diffuse, percentLit) * 0.8f + 0.2f);
    return texColor;
}