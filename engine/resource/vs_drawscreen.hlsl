struct VertexOut
{
	float4 PosH : SV_POSITION;
	float4 PosV : POSITION;
	float2 TexC : TEXCOORD;
	uint InstanceID : SV_InstanceID;
};
 
static const float2 gTexCoords[6] =
{
	float2(0.0f, 1.0f),
	float2(0.0f, 0.0f),
	float2(1.0f, 0.0f),
	float2(0.0f, 1.0f),
	float2(1.0f, 0.0f),
	float2(1.0f, 1.0f)
};

VertexOut VS(uint vid : SV_VertexID, uint iid : SV_InstanceID)
{
	VertexOut vout;

	vout.TexC = gTexCoords[vid];

	// Quad covering screen in NDC space.
	vout.PosH = float4(2.0f * vout.TexC.x - 1.0f, 1.0f - 2.0f * vout.TexC.y, 0.0f, 1.0f);
	vout.PosV = vout.PosH;
	vout.InstanceID = iid;

	return vout;
}