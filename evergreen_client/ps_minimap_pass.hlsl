struct VertexOut
{
	float4 PosH         : SV_POSITION;
	float2 ScreenTex    : TEXCOORD0;
    float4 PosW         : POSITION0;
    float2 Tex          : TEXCOORD1;
    float4 NormalW      : NORMAL;
    float4 TangentW     : TANGENT;
};

float4 PS(VertexOut pin) : SV_Target
{
    clip(0.5f - length(pin.ScreenTex.xy - 0.5f));
    return float4(pin.PosW.yyy / 128.0f, 1.0f);
}