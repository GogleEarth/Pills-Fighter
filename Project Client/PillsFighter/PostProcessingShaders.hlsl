#ifndef DEFINE_DEFINE
#define DEFINE_DEFINE
#include "Define.hlsl"
#endif

cbuffer cbMotionBlurInfo : register(b12)
{
	float4x4 gmtxPrevViewProjection;
	float4x4 gmtxInverseViewProjection;
	int gnWidth;
	int gnHeight;
	int gnSamples;
}

float4 VSPostProcessing(uint nVertexID : SV_VertexID) : SV_POSITION
{
	if (nVertexID == 0) return(float4(-1.0f, +1.0f, 0.0f, 1.0f));
	if (nVertexID == 1) return(float4(+1.0f, +1.0f, 0.0f, 1.0f));
	if (nVertexID == 2) return(float4(+1.0f, -1.0f, 0.0f, 1.0f));
	if (nVertexID == 3) return(float4(-1.0f, +1.0f, 0.0f, 1.0f));
	if (nVertexID == 4) return(float4(+1.0f, -1.0f, 0.0f, 1.0f));
	if (nVertexID == 5) return(float4(-1.0f, -1.0f, 0.0f, 1.0f));

	return(float4(0, 0, 0, 0));
}

float4 PSPostProcessing(float4 position : SV_POSITION) : SV_Target
{
	float3 cColor = gtxtTexture[0][int2(position.xy)].rgb;

	return(float4(cColor, 1.0f));
}