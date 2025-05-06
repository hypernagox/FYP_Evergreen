#include "vs_drawscreen.hlsl"

Texture2D gBuffer1    : register(t0);
Texture2D gBuffer2    : register(t1);
Texture2D gBuffer3    : register(t2);
Texture2D gShadowMap  : register(t3);
Texture2D gSSAOMap	  : register(t4);
Texture2D gBufferDSV  : register(t5);
Texture2D gEnvironmentMap : register(t6);

SamplerState gsamPointClamp : register(s0);

static const float4x4 gTex = {
	0.5f, 0.0f, 0.0f, 0.0f,
	0.0f, -0.5f, 0.0f, 0.0f,
	0.0f, 0.0f, 1.0f, 0.0f,
	0.5f, 0.5f, 0.0f, 1.0f
};

float4 PS(VertexOut pin) : SV_Target
{
	float4 col = 1.0f;
	switch (pin.InstanceID)
	{
	case 0:
		col = gBuffer1.Sample(gsamPointClamp, pin.TexC);
		break;
	case 1:
		col = float4(gBuffer2.Sample(gsamPointClamp, pin.TexC).rg, 0.0f, 1.0f);
		break;
	case 2:
		{
			float2 src = gBuffer3.Sample(gsamPointClamp, pin.TexC).rg;
            col = float4(saturate(src.x * -0.5f + 0.5f), saturate(src.y * 0.5f + 0.5f), saturate(max(src.y * -0.5f + 0.5f, 0.5f)), 1.0f);
		}
		break;
	case 3:
		col = float4(gShadowMap.Sample(gsamPointClamp, pin.TexC).rrr, 1.0f);
		break;
	}
	return col;
}