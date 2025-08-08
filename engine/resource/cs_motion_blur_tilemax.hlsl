Texture2D gSource : register(t0);
RWTexture2D<float2> gDestination : register(u0);

[numthreads(16, 16, 1)]
void CS(int3 id : SV_DispatchThreadID)
{
	float2 maxSample = 0.0f;
	for (int y = 0; y < MAX_BLUR_RADIUS; ++y)
	{
		for (int x = 0; x < MAX_BLUR_RADIUS; ++x)
		{
			float2 sample = gSource.Load(int3(id.xy * MAX_BLUR_RADIUS + int2(x, y), 0)).xy;
			if (dot(sample, sample) > dot(maxSample, maxSample))
			{
				maxSample = sample;
			}
		}
	}
	gDestination[id.xy] = maxSample;
}