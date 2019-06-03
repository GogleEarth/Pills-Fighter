#ifndef DEFINE_DEFINE
#define DEFINE_DEFINE
#include "Define.hlsl"
#endif

struct VS_TERRAIN_INPUT
{
	float3 position : POSITION;
	float4 color : COLOR;
	//float3 normal : NORMAL;
	float2 uv0 : TEXCOORD0;
	float2 uv1 : TEXCOORD1;
};

struct VS_TERRAIN_OUTPUT
{
	float4 position : SV_POSITION;
	float4 color : COLOR;
	//float3 normal : NORMAL;
	float2 uv0 : TEXCOORD0;
	float2 uv1 : TEXCOORD1;
};

VS_TERRAIN_OUTPUT VSTerrain(VS_TERRAIN_INPUT input)
{
	VS_TERRAIN_OUTPUT output;

	output.position = mul(mul(mul(float4(input.position, 1.0f), gmtxGameObject), gmtxView), gmtxProjection);
	output.color = input.color;
	output.uv0 = input.uv0;
	output.uv1 = input.uv1;

	return(output);
}

float4 PSTerrain(VS_TERRAIN_OUTPUT input) : SV_TARGET
{
	float4 cTileColor1 = gtxtTileTexture[1].Sample(gssWrap, input.uv1);
	float4 cTileColor2 = gtxtTileTexture[2].Sample(gssWrap, input.uv1);
	float4 cTileColor3 = gtxtTileTexture[3].Sample(gssWrap, input.uv1);
	float4 cTileColor4 = gtxtTileTexture[4].Sample(gssWrap, input.uv1);

	float4 cColor;
	float4 road = float4(1, 1, 1, 1);
	//float4 block = float4(127, 127, 127, 1);
	float4 bush = float4(0, 0, 0, 1);
	//float4 trail = float4(176, 176, 176, 1);

	float4 tile = gtxtTileTexture[0].Sample(gssWrap, input.uv0);

	if (all(tile == road)) { cColor = cTileColor1; }
	else if (tile.x > 0.001 && tile.x < 0.6) { cColor = cTileColor2; }
	else if (all(tile == bush)) { cColor = cTileColor3; }
	else if (tile.x > 0.6 && tile.x < 0.999) { cColor = cTileColor4; }
	else { cColor = float4(1, 1, 1, 1); }

	return(cColor);
}
