#ifndef DEFINE_DEFINE
#define DEFINE_DEFINE
#include "Define.hlsl"
#endif

#ifndef DEFINE_LIGHT
#define DEFINE_LIGHT
#include "Light.hlsl"
#endif

struct VS_TERRAIN_INPUT
{
	float3 position : POSITION;
	float3 normal : NORMAL;
	float4 color : COLOR;
	float2 uv0 : TEXCOORD0;
	float2 uv1 : TEXCOORD1;
};

struct VS_TERRAIN_OUTPUT
{
	float3 positionW : POSITION;
	float4 position : SV_POSITION;
	float3 normalW : NORMAL;
	float4 color : COLOR;
	float2 uv0 : TEXCOORD0;
	float2 uv1 : TEXCOORD1;
	float4 shadowPosH : SHADOWPOS;
};

VS_TERRAIN_OUTPUT VSTerrain(VS_TERRAIN_INPUT input)
{
	VS_TERRAIN_OUTPUT output;

	output.positionW = (float3)mul(float4(input.position, 1.0f), gmtxGameObject);
	output.position = mul(float4(output.positionW, 1.0f), gmtxViewProjection);
	output.normalW = mul(input.normal, (float3x3)gmtxGameObject);
	output.color = input.color;
	output.uv0 = input.uv0;
	output.uv1 = input.uv1;
	output.shadowPosH = mul(float4(output.positionW, 1.0f), gmtxShadowTransform);

	return(output);
}

float4 PSTerrain(VS_TERRAIN_OUTPUT input) : SV_TARGET
{
	float4 cColor;
	float4 road = float4(1, 1, 1, 1);
	//float4 block = float4(127, 127, 127, 1);
	float4 bush = float4(0, 0, 0, 1);
	//float4 trail = float4(176, 176, 176, 1);

	float4 tile = gtxtTileTexture[0].Sample(gssWrap, input.uv0);

	if (all(tile == road)) 
	{ 
		cColor = gtxtTileTexture[1].Sample(gssWrap, input.uv1);
	}
	else if (tile.x > 0.001 && tile.x < 0.6) 
	{ 
		cColor = gtxtTileTexture[2].Sample(gssWrap, input.uv1);
	}
	else if (all(tile == bush)) 
	{ 
		cColor = gtxtTileTexture[3].Sample(gssWrap, input.uv1);
	}
	else if (tile.x > 0.6 && tile.x < 0.999)
	{ 
		cColor = gtxtTileTexture[4].Sample(gssWrap, input.uv1);
	}
	else 
	{ 
		cColor = float4(1, 1, 1, 1);
	}

	float3 normalW = normalize(input.normalW);
	float4 cIllumination = float4(1.0f, 1.0f, 1.0f, 1.0f);

	float3 shadowFactor = float3(1.0f, 1.0f, 1.0f);
	shadowFactor = CalcShadowFactor(input.shadowPosH);

	cIllumination = Lighting(input.positionW, normalW, gMaterial, 0.2f, shadowFactor);

	return(cColor * cIllumination);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

struct VS_TERRAIN_SHADOW_INPUT
{
	float3 position : POSITION;
};

struct VS_TERRAIN_SHADOW_OUTPUT
{
	float4 position : SV_POSITION;
};

VS_TERRAIN_SHADOW_OUTPUT VSTerrainShadow(VS_TERRAIN_SHADOW_INPUT input)
{
	VS_TERRAIN_SHADOW_OUTPUT output;

	output.position = mul(mul(float4(input.position, 1.0f), gmtxGameObject), gmtxLightViewProjection);

	return(output);
}