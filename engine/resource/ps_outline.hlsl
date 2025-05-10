#include "vs_drawscreen.hlsl"

Texture2D gSource : register(t0);
Texture2D<uint2> gStencil : register(t1);

SamplerState gsamPointClamp : register(s0);

static const float2 offsets[8] =
{
	float2(-1, -1),
	float2( 0, -1),
	float2( 1, -1),
	float2(-1,  0),
	float2( 1,  0),
	float2(-1,  1),
	float2( 0,  1),
	float2( 1,  1)
};

static const float4 outlineColor = float4(1.0f, 1.0f, 1.0f, 1.0f);

float4 PS(VertexOut pin) : SV_Target
{
	float2 uv = pin.TexC;
	float4 color = gSource.Sample(gsamPointClamp, uv);
	uint sSrc = gStencil.Load(int3(pin.PosH.xy, 0)).g;
	uint sDst = 0;
	[unroll]
	for (int i = 0; i < 8; i++)
	{
		sDst |= gStencil.Load(int3(pin.PosH.xy + offsets[i], 0)).g;
	}
	return (sSrc & 0x80) < (sDst & 0x80) ? outlineColor : color;
}