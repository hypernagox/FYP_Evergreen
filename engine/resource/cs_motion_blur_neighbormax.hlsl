Texture2D gSource : register(t0);
RWTexture2D<float2> gDestination : register(u0);

[numthreads(16, 16, 1)]
void CS(int3 id : SV_DispatchThreadID)
{
	float2 maxSample = 0.0f;
	[unroll]
	for (int y = -1; y <= 1; ++y)
	{
		[unroll]
		for (int x = -1; x <= 1; ++x)
		{
			float2 sample = gSource.Load(int3(id.xy + int2(x, y), 0));
			if (dot(sample, sample) > dot(maxSample, maxSample))
			{
				maxSample = sample;
			}
		}
	}
	gDestination[id.xy] = maxSample;
}