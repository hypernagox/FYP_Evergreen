struct VertexOut
{
	float4 PosH         : SV_POSITION;
    float4 PosW         : POSITION0;
    float2 Tex          : TEXCOORD;
    float4 NormalW      : NORMAL;
    float4 TangentW     : TANGENT;
    float4 PrevPosH     : POSITION2;
};

float4 PS(VertexOut pin) : SV_Target
{
    return float4(pin.PosW.yyy / 100.0f, 1.0f);
}