cbuffer cbPerObject : register(b0)
{
    float4x4 gWorld;
	float4x4 gView;
	float4x4 gProj;
};

struct VertexIn
{
	float3 PosL         : POSITION;
    float2 Tex          : TEXCOORD;
    float3 Normal       : NORMAL;
    float3 Tangent	    : TANGENT;
};

struct VertexOut
{
	float4 PosH         : SV_POSITION;
	float2 ScreenTex    : TEXCOORD0;
    float4 PosW         : POSITION0;
    float2 Tex          : TEXCOORD1;
    float4 NormalW      : NORMAL;
    float4 TangentW     : TANGENT;
};

static const float4x4 gTex =
{
        0.5f, 0.0f, 0.0f, 0.0f,
	    0.0f, -0.5f, 0.0f, 0.0f,
	    0.0f, 0.0f, 1.0f, 0.0f,
	    0.5f, 0.5f, 0.0f, 1.0f
};

VertexOut VS(VertexIn vin)
{
	VertexOut vout;

	// Transform position to homogeneous clip space
	vout.PosH = mul(float4(vin.PosL, 1.0f), gWorld);

	// Store world position
	vout.PosW = vout.PosH;

	vout.PosH = mul(vout.PosH, gView);
	vout.PosH = mul(vout.PosH, gProj);
	vout.ScreenTex = mul(vout.PosH, gTex).xy / vout.PosH.w;

	// Pass through texture coordinates
	vout.Tex = vin.Tex;

	// Transform normal and tangent to world space
	vout.NormalW = mul(float4(vin.Normal, 0.0f), gWorld);
	vout.TangentW = mul(float4(vin.Tangent, 0.0f), gWorld);

	return vout;
}