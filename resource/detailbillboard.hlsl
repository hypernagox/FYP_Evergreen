#define USE_CUSTOM_SHADOWPS
#include "common.hlsl"

struct GeometryOut {
	float4 PosH     : SV_POSITION;
	float4 PosW     : POSITION0;
    float4 PrevPosH : POSITION1;
	float2 Tex      : TEXCOORD0;
	float3 NormalW  : NORMAL;
};

// Random function
float3 random3(float3 c) {
    float j = 4096.0 * sin(dot(c, float3(17.0, 59.4, 15.0)));
    float3 r;
    r.z = frac(512.0 * j);
    j *= 0.125;
    r.x = frac(512.0 * j);
    j *= 0.125;
    r.y = frac(512.0 * j);
    return r - 0.5;
}

// Skew constants
static const float F3 = 0.3333333;
static const float G3 = 0.1666667;

// 3D Simplex noise function
float simplex3d(float3 p) {
    float3 s = floor(p + dot(p, float3(F3, F3, F3)));
    float3 x = p - s + dot(s, float3(G3, G3, G3));

    float3 e = step(float3(0.0, 0.0, 0.0), x - x.yzx);
    float3 i1 = e * (1.0 - e.zxy);
    float3 i2 = 1.0 - e.zxy * (1.0 - e);

    float3 x1 = x - i1 + G3;
    float3 x2 = x - i2 + 2.0 * G3;
    float3 x3 = x - 1.0 + 3.0 * G3;

    float4 w;
    float4 d;

    w.x = dot(x, x);
    w.y = dot(x1, x1);
    w.z = dot(x2, x2);
    w.w = dot(x3, x3);

    w = max(0.6 - w, 0.0);

    d.x = dot(random3(s), x);
    d.y = dot(random3(s + i1), x1);
    d.z = dot(random3(s + i2), x2);
    d.w = dot(random3(s + 1.0), x3);

    w *= w;
    w *= w;
    d *= w;

    return dot(d, float4(52.0, 52.0, 52.0, 52.0));
}

// Rotation matrices
static const float3x3 rot1 = float3x3(
    -0.37, 0.36, 0.85,
    -0.14, -0.93, 0.34,
    0.92, 0.01, 0.4
);

static const float3x3 rot2 = float3x3(
    -0.55, -0.39, 0.74,
    0.33, -0.91, -0.24,
    0.77, 0.12, 0.63
);

static const float3x3 rot3 = float3x3(
    -0.71, 0.52, -0.47,
    -0.08, -0.72, -0.68,
    -0.7, -0.45, 0.56
);

// Fractal noise using rotated octaves
float simplex3d_fractal(float3 m) {
    return   0.5333333 * simplex3d(mul(m, rot1))
           + 0.2666667 * simplex3d(mul(m * 2.0, rot2))
           + 0.1333333 * simplex3d(mul(m * 4.0, rot3))
           + 0.0666667 * simplex3d(m * 8.0);
}

VertexOut VS(VertexIn vin)
{
	VertexOut vout;
    ConstructVSOutput(vin, vout);

    return vout;
}

[maxvertexcount(4)]
void GS(point VertexOut input[1], uint primitiveID : SV_PRIMITIVEID, inout TriangleStream<GeometryOut> outStream)
{
    const float viewDepth = length(input[0].PosW - gEyePosW);
    const float scale = saturate(8.0f - viewDepth * 0.125f) * 2.0f;

    float3 up = normalize(float3(gView[0].y, gView[1].y, gView[2].y) + float3(0.0f, 1.0f, 0.0f));
    float3 look = -normalize(float3(gView[0].z, gView[1].z, gView[2].z));
    float3 right = normalize(cross(up, look));
    float3x3 uvw = float3x3(right, up, look);

    float windOffset = simplex3d_fractal(float3(input[0].PosW.xy * 0.1f + gTime * -0.1f, gTime * 0.01f));
    float3 offsets[4];
    offsets[0] = float3(+0.5f, 0.0f, 0.0f);
    offsets[1] = float3(+0.5f, 1.0f, 0.0f);
    offsets[2] = float3(-0.5f, 0.0f, 0.0f);
    offsets[3] = float3(-0.5f, 1.0f, 0.0f);
    
    input[0].PosW.xyz *= 2.0f;

    GeometryOut output;
    for (uint i = 0; i < 4; ++i)
    {
        float3 worldOffset = mul(offsets[i], uvw) * scale + float3(windOffset, 0.0f, windOffset) * (i % 2);
        output.PosW = input[0].PosW + float4(worldOffset, 1.0f);
        output.PosH = mul(output.PosW, gViewProj);
        output.PrevPosH = mul(output.PosW, gPrevViewProj);
        output.NormalW = -gDirLight;
        output.Tex = input[0].Tex + float2(i / 2, 1 - i % 2) * 0.5f;
        outStream.Append(output);
    }
}

PixelOut PS(GeometryOut pin)
{
	PixelOut pOut;
    float3 normal = normalize(mul(pin.NormalW.xyz, (float3x3)gView));
    float4 texColor = gMainTex.Sample(gSampler, pin.Tex);
    float4 posH = mul(pin.PosW, gViewProj);
    posH /= posH.w;
    pin.PrevPosH /= pin.PrevPosH.w;
    float4 posDelta = posH - pin.PrevPosH;
    
    clip(texColor.a - 0.1f);
     
    pOut.Buffer1 = texColor;
    pOut.Buffer2 = PackNormal(normal);
    pOut.Buffer3 = posDelta.xy * gMotionBlurFactor * 0.5f * gRenderTargetSize / gMotionBlurRadius;
	pOut.Buffer3 /= max(length(pOut.Buffer3), 1.0f);
    return pOut;
}

void ShadowPS(GeometryOut pin)
{
    float a = gMainTex.Sample(gSampler, pin.Tex).a;
    clip(a - 0.1f);
}