Texture2D<float4> gtxtInputA : register(t0);
Texture2D<float4> gtxtInputB : register(t1);
Texture2D<float4> gtxtInputC : register(t2);
Texture2D<float> gtxtDepth : register(t3);
Texture2D<float4> gtxtMask : register(t4);
RWTexture2D<float4> gtxtRWOutput : register(u0);

groupshared float4 gTextureCache[(256 + 2 * 5)];

static float gfWeights[11] = { 0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 0.2f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f };

[numthreads(256, 1, 1)]
void HorzBlurCS(int3 vGroupThreadID : SV_GroupThreadID, int3 vDispatchThreadID : SV_DispatchThreadID)
{
	if (vGroupThreadID.x < 5)
	{
		int x = max(vDispatchThreadID.x - 5, 0);
		gTextureCache[vGroupThreadID.x] = gtxtInputA[int2(x, vDispatchThreadID.y)];
	}
	else if (vGroupThreadID.x >= 256 - 5)
	{
		int x = min(vDispatchThreadID.x + 5, gtxtInputA.Length.x - 1);
		gTextureCache[vGroupThreadID.x + 2 * 5] = gtxtInputA[int2(x, vDispatchThreadID.y)];
	}
	gTextureCache[vGroupThreadID.x + 5] = gtxtInputA[min(vDispatchThreadID.xy, gtxtInputA.Length.xy - 1)];

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
		gTextureCache[vGroupThreadID.y] = gtxtInputA[int2(vDispatchThreadID.x, y)];
	}
	else if (vGroupThreadID.y >= 256 - 5)
	{
		int y = min(vDispatchThreadID.y + 5, gtxtInputA.Length.y - 1);
		gTextureCache[vGroupThreadID.y + 2 * 5] = gtxtInputA[int2(vDispatchThreadID.x, y)];
	}
	gTextureCache[vGroupThreadID.y + 5] = gtxtInputA[min(vDispatchThreadID.xy, gtxtInputA.Length.xy - 1)];

	GroupMemoryBarrierWithGroupSync();

	float4 cBlurredColor = float4(0, 0, 0, 0);
	for (int i = -5; i <= 5; i++)
	{
		int k = vGroupThreadID.y + 5 + i;
		cBlurredColor += gfWeights[i + 5] * gTextureCache[k];
	}

	gtxtRWOutput[vDispatchThreadID.xy] = cBlurredColor;
}

[numthreads(8, 8, 1)]
void Add2CS(int3 vDispatchThreadID : SV_DispatchThreadID)
{
	gtxtRWOutput[vDispatchThreadID.xy] = gtxtInputA[vDispatchThreadID.xy] + gtxtInputB[vDispatchThreadID.xy];
}

[numthreads(8, 8, 1)]
void Add3CS(int3 vDispatchThreadID : SV_DispatchThreadID)
{
	gtxtRWOutput[vDispatchThreadID.xy] = gtxtInputA[vDispatchThreadID.xy] + gtxtInputB[vDispatchThreadID.xy] + gtxtInputC[vDispatchThreadID.xy];
}

[numthreads(8, 8, 1)]
void BrightFilterCS(int3 vDispatchThreadID : SV_DispatchThreadID)
{
	float brightness = dot(gtxtInputA[vDispatchThreadID.xy].rgb, float3(0.2126f, 0.7152f, 0.0722));
	if (brightness > 0.95f)
		gtxtRWOutput[vDispatchThreadID.xy] = gtxtInputA[vDispatchThreadID.xy];
}

cbuffer cbMotionBlurInfo : register(b0)
{
	matrix gmtxPrevViewProjection;
	matrix gmtxInverseViewProjection;
	int gnWidth;
	int gnHeight;
	float fFPS;
}

#define T 0.016667f
#define SAMPLES 16

[numthreads(8, 8, 1)]
void MotionBlurCS(int3 vDispatchThreadID : SV_DispatchThreadID)
{
	float zOverW = gtxtDepth[vDispatchThreadID.xy];
	float mask = gtxtMask[vDispatchThreadID.xy].r;

	if (mask > 0.5f)
	{
		gtxtRWOutput[vDispatchThreadID.xy] = gtxtInputA[vDispatchThreadID.xy];
		return;
	}

	float2 oriTex;
	oriTex.x = float(vDispatchThreadID.x) / float(gnWidth);
	oriTex.y = float(vDispatchThreadID.y) / float(gnHeight);

	float4 H = float4(oriTex.x * 2.0f - 1.0f, (1.0f - oriTex.y) * 2.0f - 1.0f, zOverW, 1.0f);

	float4 D = mul(gmtxInverseViewProjection, H);

	float4 worldPos = D / D.w;

	float4 currPos = H;

	float4 prevPos = mul(gmtxPrevViewProjection, worldPos);

	prevPos /= prevPos.w;

	float2 velocity = (currPos - prevPos).xy * 1.5f;

	float dt = T / fFPS;

	velocity *= dt;

	float4 color = float4(0.0f, 0.0f, 0.0f, 1.0f);

	float2 du = velocity / SAMPLES;

	float2 texcoord = oriTex + du;

	int nSamples = 0;
	for (int i = 1; i < SAMPLES; ++i, texcoord -= du)
	{
		if (texcoord.x >= 1.0f) break;
		if (texcoord.y >= 1.0f) break;
		if (texcoord.x <= 0.0f) break;
		if (texcoord.y <= 0.0f) break;

		int2 nIndex = int2(texcoord.x * gnWidth, texcoord.y * gnHeight);

		if (gtxtDepth[nIndex].r - zOverW > 0.1f) continue;
		mask = gtxtMask[nIndex].r;
		if (mask > 0.5f) continue;

		float4 currColor = gtxtInputA[nIndex];

		color += currColor;
		nSamples++;
	}

	gtxtRWOutput[vDispatchThreadID.xy] = color / nSamples;
}