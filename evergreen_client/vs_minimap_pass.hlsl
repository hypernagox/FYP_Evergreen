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
    float4 PosW         : POSITION0;
    float2 Tex          : TEXCOORD;
    float4 NormalW      : NORMAL;
    float4 TangentW     : TANGENT;
    float4 PrevPosH     : POSITION2;
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

	// Pass through texture coordinates
	vout.Tex = vin.Tex;

	// Transform normal and tangent to world space
	vout.NormalW = mul(float4(vin.Normal, 0.0f), gWorld);
	vout.TangentW = mul(float4(vin.Tangent, 0.0f), gWorld);

	// Store previous position (for motion vectors)
	vout.PrevPosH = vout.PosH; // Assuming this is set elsewhere in the pipeline

	return vout;
}