#include "common.hlsl"

#ifdef DEFERRED

float4 PSDeferred(VertexOut pin) : SV_Target
{
	return PSDeferredDefault(pin);
}

#else

Texture2D gNormalTex : register(t1);

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

    float4 PosL = mul(float4(vin.PosL, 1.0f), vin.InstanceTransform);
    float4 PosW = ObjectToWorldPos(PosL);

    float windOffset = simplex3d_fractal(float3(PosW.xy * 0.1f + gTime * -0.1f, gTime * 0.01f)) * 2.0f;
    PosW.xz += windOffset * (1.0f - vin.Tex.y);

    vout.PosW = PosW;
	vout.PosH = WorldToClipPos(vout.PosW, vin);                                     
	vout.Tex = vin.Tex;                                                             
	vout.NormalW = ObjectToWorldNormal(mul(vin.Normal, (float3x3)vin.InstanceTransform));       
    vout.TangentW = ObjectToWorldNormal(mul(vin.Tangent, (float3x3)vin.InstanceTransform));
    vout.PrevPosH = mul(PosW, gPrevViewProj);

    return vout;
}

PixelOut PS(VertexOut pin)
{
	PixelOut pOut;

    float3 normalW = normalize(pin.NormalW);

    // Normal mapping
    float4 normalMapSample = gNormalTex.Sample(gSampler, pin.Tex);
    float3 normal = NormalSampleToWorldSpace(normalMapSample.rgb, normalW, pin.TangentW.xyz);
    normal = mul(normal, (float3x3)gView);

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

#endif