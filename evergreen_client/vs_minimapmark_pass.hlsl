cbuffer cbPerObject : register(b0)
{
    float4x4 gWorld;
	float4x4 gView;
	float4x4 gProj;
};

struct VertexOut
{
    float4 PosW         : SV_POSITION;
};

VertexOut VS()
{
	VertexOut vout;

	// Transform position to homogeneous clip space
	vout.PosW = gWorld[3];

	return vout;
}