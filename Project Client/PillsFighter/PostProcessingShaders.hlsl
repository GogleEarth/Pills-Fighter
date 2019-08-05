#ifndef DEFINE_DEFINE
#define DEFINE_DEFINE
#include "Define.hlsl"
#endif

cbuffer cbScreenEffect : register(SCREEN_EFFECT)
{
	float4 gScreenColor;
};

struct VS
{
	float4 position : SV_POSITION;
	float2 uv : TEXCOORD;
};

VS VSPostProcessing(uint nVertexID : SV_VertexID)
{
	VS output;

	output.position = 0.0f;
	output.uv = 0.0f;

	if (nVertexID == 0)
	{
		output.position = float4(-1.0f, +1.0f, 0.0f, 1.0f);
		output.uv = float2(0.0f, 0.0f);
	}
	if (nVertexID == 1)
	{
		output.position = float4(+1.0f, +1.0f, 0.0f, 1.0f);
		output.uv = float2(1.0f, 0.0f);
	}
	if (nVertexID == 2)
	{
		output.position = float4(+1.0f, -1.0f, 0.0f, 1.0f);
		output.uv = float2(1.0f, 1.0f);
	}
	if (nVertexID == 3)
	{
		output.position = float4(-1.0f, +1.0f, 0.0f, 1.0f);
		output.uv = float2(0.0f, 0.0f);
	}
	if (nVertexID == 4)
	{
		output.position = float4(+1.0f, -1.0f, 0.0f, 1.0f);
		output.uv = float2(1.0f, 1.0f);
	}
	if (nVertexID == 5)
	{
		output.position = float4(-1.0f, -1.0f, 0.0f, 1.0f);
		output.uv = float2(0.0f, 1.0f);
	}

	return output;
}

float4 PSPostProcessing(VS input) : SV_Target
{
	float4 cColor = gtxtTexture[0].Sample(gssClamp, input.uv);

	return cColor * gScreenColor;
}

float4 PSPostProcessingCombine(VS input) : SV_Target
{
	float4 cColor = gtxtTexture[0].Sample(gssClamp, input.uv);

	return cColor;
}

//
#define FILTER_SAMPLE 5
static float gfLaplacians[FILTER_SAMPLE] = { -1.0f, -1.0f, 4.0f, -1.0f, -1.0f };
static int2 gnOffsets[FILTER_SAMPLE] = { { 0,-1 },{ -1,0 },{ 0,0 },{ 1,0 },{ 0,1 } };
//static float gfLaplacians[FILTER_SAMPLE] = { -1.0f, -1.0f, -1.0f, -1.0f, 8.0f, -1.0f, -1.0f, -1.0f, -1.0f };
//static int2 gnOffsets[FILTER_SAMPLE] = { { -1,-1 },{ 0,-1 },{ 1,-1 },{ -1,0 },{ 0,0 },{ 1,0 },{ -1,1 },{ 0,1 },{ 1,1 } };

float4 PSPostProcessingByLaplacianEdge(VS input) : SV_Target
{
	float fEdgeness = 0.0f;
	float3 cEdgeness = float3(0.0f, 0.0f, 0.0f);
	int2 pos = int2(input.position.xy);
	if ((uint(pos.x) >= 1) || (uint(pos.y) >= 1) || (uint(pos.x) <= gtxtScreenNormalTexture.Length.x - 2) || (uint(pos.y) <= gtxtScreenNormalTexture.Length.y - 2)) {
		for (int i = 0; i < FILTER_SAMPLE; i++) {
			float3 vNormal = gtxtScreenNormalTexture[int2(pos.xy) + gnOffsets[i]].xyz;
			vNormal = vNormal * 2.0f - 1.0f;
			cEdgeness += gfLaplacians[i] * vNormal;
			fEdgeness = cEdgeness.r * 0.3f + cEdgeness.g * 0.59f + cEdgeness.b * 0.11f;
			cEdgeness = float3(fEdgeness, fEdgeness, fEdgeness);
		}
	}
	float4 outlineColor = 0;
	float4 cColor = gtxtTexture[0].Sample(gssClamp, input.uv);
	float4 sColor = cColor;
	//sColor = (fEdgeness < 0.15f) ? cColor : ((fEdgeness < 0.65f) ? (cColor - cEdgeness) : outlineColor);
	sColor = (fEdgeness < 0.5f) ? cColor : outlineColor;

	return sColor;
}





////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

VS VSTest(uint nVertexID : SV_VertexID)
{
	VS output;

	output.position = 0.0f;
	output.uv = 0.0f;

	if (nVertexID == 0)
	{
		output.position = float4(0.0f, 0.0f, 0.0f, 1.0f);
		output.uv = float2(0.0f, 0.0f);
	}
	if (nVertexID == 1)
	{
		output.position = float4(+1.0f, 0.0f, 0.0f, 1.0f);
		output.uv = float2(1.0f, 0.0f);
	}
	if (nVertexID == 2)
	{
		output.position = float4(+1.0f, -1.0f, 0.0f, 1.0f);
		output.uv = float2(1.0f, 1.0f);
	}
	if (nVertexID == 3)
	{
		output.position = float4(0.0f, 0.0f, 0.0f, 1.0f);
		output.uv = float2(0.0f, 0.0f);
	}
	if (nVertexID == 4)
	{
		output.position = float4(+1.0f, -1.0f, 0.0f, 1.0f);
		output.uv = float2(1.0f, 1.0f);
	}
	if (nVertexID == 5)
	{
		output.position = float4(0.0f, -1.0f, 0.0f, 1.0f);
		output.uv = float2(0.0f, 1.0f);
	}

	return output;
}

float4 PSTest(VS input) : SV_Target
{
	float4 cColor = gtxtTexture[0].Sample(gssClamp, input.uv);

	return cColor;
}