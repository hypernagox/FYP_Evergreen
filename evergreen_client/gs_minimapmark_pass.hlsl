cbuffer cbPerObject : register(b0)
{
    float4x4 gWorld;
	float4x4 gView;
	float4x4 gProj;
};

struct GeometryIn
{
	float4 PosW         : SV_POSITION;
};

struct GeometryOut
{
	float4 PosH         : SV_POSITION;
	float2 ScreenTex    : TEXCOORD0;
	float4 PosW         : POSITION0;
	float2 Tex          : TEXCOORD1;
};

static const float4x4 gTex =
{
        0.5f, 0.0f, 0.0f, 0.0f,
	    0.0f, -0.5f, 0.0f, 0.0f,
	    0.0f, 0.0f, 1.0f, 0.0f,
	    0.5f, 0.5f, 0.0f, 1.0f
};

[maxvertexcount(6)]
void GS(point GeometryIn input[1], inout TriangleStream<GeometryOut> triStream)
{
    float size = 16.0f;

    // View 행렬의 열벡터를 사용하여 cameraRight / cameraUp 추출
    float3 right = normalize(float3(gView[0][0], gView[1][0], gView[2][0]));
    float3 up    = normalize(float3(gView[0][1], gView[1][1], gView[2][1]));

    // 사각형 오프셋 계산
    float3 offsets[4] = {
        -right * size + up * size,   // Top Left
         right * size + up * size,   // Top Right
        -right * size - up * size,   // Bottom Left
         right * size - up * size    // Bottom Right
    };

    float2 uvs[4] = {
        float2(0, 0),
        float2(1, 0),
        float2(0, 1),
        float2(1, 1)
    };

    int indices[6] = { 0, 1, 2, 3, 2, 1 };

    for (int i = 0; i < 6; ++i)
    {
        int idx = indices[i];
        GeometryOut output;
        float3 worldPos = input[0].PosW.xyz + offsets[idx];

        float4 viewPos = mul(float4(worldPos, 1.0), gView);
        output.PosW = float4(worldPos, 1.0);
        output.PosH = mul(viewPos, gProj);
        output.Tex = uvs[idx];
	    output.ScreenTex = mul(output.PosH, gTex).xy / output.PosH.w;
        triStream.Append(output);
    }
}