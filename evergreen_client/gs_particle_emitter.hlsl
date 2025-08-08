#include "inc_sphere_particle_emitter.hlsl"

struct GeometryIn
{
	float4 PosW         : SV_POSITION;
	float3 Normal       : NORMAL0;
	float  Life			: TEXCOORD0;
	uint   Vid          : TEXCOORD1;
};

struct GeometryOut
{
	float4 PosH         : SV_POSITION;
	float4 PosW         : POSITION0;
	float2 Tex          : TEXCOORD1;
    float  Alpha        : TEXCOORD2;
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
    uint flags = asuint(gWorld[3][3]);
    float elapsedTime = gTime - gEmitTime;
    if (elapsedTime > input[0].Life && !(flags & 1)) {
       return;
    }

	float currentLife = fmod(elapsedTime, input[0].Life);
	float lifeFactor = currentLife / input[0].Life;

    float2 size = lerp(gSizeMin, gSizeMax, rand3(input[0].Vid * VIDMUL - gEmitTime).y + 0.5f);
    if (gSizeLifeExp.x < 0.0f) {
       size.x *= pow(1.0f - lifeFactor, -gSizeLifeExp.x);
    }
    else {
       size.x *= pow(lifeFactor, gSizeLifeExp.x);
    }
    if (gSizeLifeExp.y < 0.0f) {
	   size.y *= pow(1.0f - lifeFactor, -gSizeLifeExp.y);
	}
	else {
	   size.y *= pow(lifeFactor, gSizeLifeExp.y);
	}
    float alpha = 1.0f;
    if (gAlphaLifeExp.x < 0.0f) {
		alpha *= pow(1.0f - lifeFactor, -gAlphaLifeExp.x);
	}
	else {
		alpha *= pow(lifeFactor, gAlphaLifeExp.x);
	}
    float rotation = lerp(gRotationMin, gRotationMax, rand3(input[0].Vid * VIDMUL - gEmitTime).z + 0.5f);
    if (gRotationLifeExp < 0.0f) {
		rotation *= pow(1.0f - lifeFactor, -gRotationLifeExp);
	}
	else {
		rotation *= pow(lifeFactor, gRotationLifeExp);
	}

    // View 행렬의 열벡터를 사용하여 cameraRight / cameraUp 추출
    float3 right = float3(1.0f, 0.0f, 0.0f);
    float3 up = float3(0.0f, 0.0f, 1.0f);

    if (!(flags & 4))
    {
        right = normalize(float3(gView[0][0], gView[1][0], gView[2][0]));
        up    = normalize(float3(gView[0][1], gView[1][1], gView[2][1]));
    }
    
    if (flags & 2)
    {
        float2 vNormal = float2(dot(input[0].Normal, right), dot(input[0].Normal, up));
        rotation += atan2(vNormal.y, vNormal.x);
    }

    float3x3 rotationMatrix = float3x3(
		cos(rotation), -sin(rotation), 0.0f,
		sin(rotation),  cos(rotation), 0.0f,
		0.0f,          0.0f,          1.0f
	);

    float3 offsets[4] = {
        float3(-size.x, size.y, 0.0f),   // Top Left
		float3(size.x, size.y, 0.0f),    // Top Right
		float3(-size.x, -size.y, 0.0f),  // Bottom Left
		float3(size.x, -size.y, 0.0f)    // Bottom Right
	};

    // 회전 적용
	for (int i = 0; i < 4; ++i)
	{
		offsets[i] = mul(rotationMatrix, offsets[i]);
        offsets[i] = right * offsets[i].x + up * offsets[i].y;
	}

    float2 uvs[4] = {
        float2(0, 0),
        float2(1, 0),
        float2(0, 1),
        float2(1, 1)
    };

    int indices[6] = { 0, 1, 2, 3, 2, 1 };

    for (int ind = 0; ind < 6; ++ind)
    {
        int idx = indices[ind];
        GeometryOut output;
        float3 worldPos = input[0].PosW.xyz + offsets[idx];

        float4 viewPos = mul(float4(worldPos, 1.0f), gView);
        output.PosW = float4(worldPos, 1.0f);
        output.PosH = mul(viewPos, gProj);
        output.Tex = uvs[idx];
        output.Alpha = alpha;
        triStream.Append(output);
    }
}