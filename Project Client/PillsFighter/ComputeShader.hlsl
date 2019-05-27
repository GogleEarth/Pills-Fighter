Texture2D<float4> gtxtInput : register(t0);
RWTexture2D<float4> gtxtRWOutput : register(u0);

groupshared float4 gTextureCache[(256 + 2 * 5)];

static float gfWeights[11] = { 0.05f, 0.05f, 0.1f, 0.1f, 0.1f, 0.2f, 0.1f, 0.1f, 0.1f, 0.05f, 0.05f };

[numthreads(256, 1, 1)]
void HorzBlurCS(int3 vGroupThreadID : SV_GroupThreadID, int3 vDispatchThreadID : SV_DispatchThreadID )
{
	if (vGroupThreadID.x < 5)
	{
		int x = max(vDispatchThreadID.x - 5, 0);
		gTextureCache[vGroupThreadID.x] = gtxtInput[int2(x, vDispatchThreadID.y)];
	}
	else if (vGroupThreadID.x >= 256 - 5)
	{
		int x = min(vDispatchThreadID.x + 5, gtxtInput.Length.x - 1);
		gTextureCache[vGroupThreadID.x + 2 * 5] = gtxtInput[int2(x, vDispatchThreadID.y)];
	}
	gTextureCache[vGroupThreadID.x + 5] = gtxtInput[min(vDispatchThreadID.xy, gtxtInput.Length.xy - 1)];

	GroupMemoryBarrierWithGroupSync();

	float4 cBlurredColor = float4(0, 0, 0, 0);
	for (int i = -5; i <= 5; i++)
	{
		int k = vGroupThreadID.x + 5 + i;
		cBlurredColor += gfWeights[i + 5] * gTextureCache[k];
	}

	gtxtRWOutput[vDispatchThreadID.xy] = cBlurredColor;
}

[numthreads(1, 256, 1)]
void VertBlurCS(int3 vGroupThreadID : SV_GroupThreadID, int3 vDispatchThreadID : SV_DispatchThreadID)
{
	if (vGroupThreadID.y < 5)
	{
		int y = max(vDispatchThreadID.y - 5, 0);
		gTextureCache[vGroupThreadID.y] = gtxtInput[int2(vDispatchThreadID.x, y)];
	}
	else if (vGroupThreadID.y >= 256 - 5)
	{
		int y = min(vDispatchThreadID.y + 5, gtxtInput.Length.y - 1);
		gTextureCache[vGroupThreadID.y + 2 * 5] = gtxtInput[int2(vDispatchThreadID.x, y)];
	}
	gTextureCache[vGroupThreadID.y + 5] = gtxtInput[min(vDispatchThreadID.xy, gtxtInput.Length.xy - 1)];

	GroupMemoryBarrierWithGroupSync();

	float4 cBlurredColor = float4(0, 0, 0, 0);
	for (int i = -5; i <= 5; i++)
	{
		int k = vGroupThreadID.y + 5 + i;
		cBlurredColor += gfWeights[i + 5] * gTextureCache[k];
	}

	gtxtRWOutput[vDispatchThreadID.xy] = cBlurredColor;
}