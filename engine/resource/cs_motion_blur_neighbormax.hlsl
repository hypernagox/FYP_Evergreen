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
			int2 offset = int2(x, -y);
			float2 sample = gSource.Load(int3(id.xy + offset, 0)).xy;

			// Guertin et al. 2013: Only consider diagonal neighbors if their velocity is pointing towards the center tile.
			// This prevents over-estimation of the motion vector.
			if ((x != 0 && y != 0) && dot(sample, float2(-x, -y)) <= 0.0f)
			{
				continue; // Skip this sample
			}

			if (dot(sample, sample) > dot(maxSample, maxSample))
			{
				maxSample = sample;
			}
		}
	}
	gDestination[id.xy] = maxSample;
}