//struct CB_GAMEOBJECT_INFO
//{
//	matrix		mtxWorld;
//	uint		nMaterial;
//};
//
//struct CB_CAMERA_INFO
//{
//	matrix		mtxView;
//	matrix		mtxProjection;
//	float3		vCameraPosition;
//};
//
//ConstantBuffer<CB_GAMEOBJECT_INFO> gcbGameObjectInfo : register(b0);
//ConstantBuffer<CB_CAMERA_INFO> gcbCameraInfo : register(b1);

cbuffer cbGameObjectInfo : register(b0)
{
	matrix		gmtxGameObject : packoffset(c0);
	uint		gnMaterial : packoffset(c4);
};

cbuffer cbCameraInfo : register(b1)
{
	matrix		gmtxView : packoffset(c0);
	matrix		gmtxProjection : packoffset(c4);
	float3		gvCameraPosition : packoffset(c8);
};

#include "Light.hlsl"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
Texture2D gtxtTexture : register(t1);
SamplerState gSamplerState : register(s0);

struct VS_TEXTURED_INPUT
{
	float3 position : POSITION;
	float3 normal : NORMAL;
	float2 uv : TEXCOORD;
};

struct VS_TEXTURED_OUTPUT
{
	float4 position : SV_POSITION;
	float3 positionW : POSITION;
	float3 normalW : NORMAL;
	float2 uv : TEXCOORD;
};

VS_TEXTURED_OUTPUT VSTextured(VS_TEXTURED_INPUT input)
{
	VS_TEXTURED_OUTPUT output;

	//output.normalW = mul(input.normal, (float3x3)gcbGameObjectInfo.mtxWorld);
	//output.positionW = (float3)mul(float4(input.position, 1.0f), gcbGameObjectInfo.mtxWorld);
	//output.position = mul(mul(float4(output.positionW, 1.0f), gcbCameraInfo.mtxView), gcbCameraInfo.mtxProjection);	
	output.normalW = mul(input.normal, (float3x3)gmtxGameObject);
	output.positionW = (float3)mul(float4(input.position, 1.0f), gmtxGameObject);
	output.position = mul(mul(float4(output.positionW, 1.0f), gmtxView), gmtxProjection);
	output.uv = input.uv;

	return(output);
}

float4 PSTextured(VS_TEXTURED_OUTPUT input) : SV_TARGET
{
	float4 cColor = gtxtTexture.Sample(gSamplerState, input.uv);

	input.normalW = normalize(input.normalW);
	float4 cIllumination = Lighting(input.positionW, input.normalW, gnMaterial);

	return(cColor * cIllumination);
	//return(cColor);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
struct VS_DIFFUSED_INPUT
{
	float3 position : POSITION;
	float4 color : COLOR;
};

struct VS_DIFFUSED_OUTPUT
{
	float4 position : SV_POSITION;
	float4 color : COLOR;
};

VS_DIFFUSED_OUTPUT VSDiffused(VS_DIFFUSED_INPUT input)
{
	VS_DIFFUSED_OUTPUT output;

	//output.position = mul(mul(mul(float4(input.position, 1.0f), gcbGameObjectInfo.mtxWorld), gcbCameraInfo.mtxView), gcbCameraInfo.mtxProjection);
	output.position = mul(mul(mul(float4(input.position, 1.0f), gmtxGameObject), gmtxView), gmtxProjection);
	output.color = input.color;

	return(output);
}

float4 PSDiffused(VS_DIFFUSED_OUTPUT input) : SV_TARGET
{
	return(input.color);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

struct INSTANCEDGAMEOBJECTINFO
{
	matrix mtxGameObject;
	uint   nMaterial;
};

StructuredBuffer<INSTANCEDGAMEOBJECTINFO> gGameObjectInfo : register(t0);

struct VS_INSTANCED_TEXTURED_OUTPUT
{
	float4 position : SV_POSITION;
	float3 positionW : POSITION;
	float3 normalW : NORMAL;
	float2 uv : TEXCOORD;
	uint instance : INSTANCE;
};

VS_INSTANCED_TEXTURED_OUTPUT VSInstancingTextured(VS_TEXTURED_INPUT input, uint nInstanceID : SV_InstanceID)
{
	VS_INSTANCED_TEXTURED_OUTPUT output;

	output.normalW = mul(input.normal, (float3x3)gGameObjectInfo[nInstanceID].mtxGameObject);
	output.positionW = (float3)mul(float4(input.position, 1.0f), gGameObjectInfo[nInstanceID].mtxGameObject);
	//output.position = mul(mul(float4(output.positionW, 1.0f), gcbCameraInfo.mtxView), gcbCameraInfo.mtxProjection);
	output.position = mul(mul(float4(output.positionW, 1.0f), gmtxView), gmtxProjection);
	output.uv = input.uv;
	output.instance = nInstanceID;

	return(output);
}

float4 PSInstancingTextured(VS_INSTANCED_TEXTURED_OUTPUT input) : SV_TARGET
{
	float4 cColor = gtxtTexture.Sample(gSamplerState, input.uv);

	input.normalW = normalize(input.normalW);
	float4 cIllumination = Lighting(input.positionW, input.normalW, gGameObjectInfo[input.instance].nMaterial);

	return(cColor* cIllumination);
	//return(cColor);
}

////////////

VS_DIFFUSED_OUTPUT VSInstancingDiffused(VS_DIFFUSED_INPUT input, uint nInstanceID : SV_InstanceID)
{
	VS_DIFFUSED_OUTPUT output;

	//output.position = mul(mul(mul(float4(input.position, 1.0f), gGameObjectInfo[nInstanceID].mtxGameObject), gcbCameraInfo.mtxView), gcbCameraInfo.mtxProjection);
	output.position = mul(mul(mul(float4(input.position, 1.0f), gGameObjectInfo[nInstanceID].mtxGameObject), gmtxView), gmtxProjection);
	output.color = input.color;

	return(output);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
cbuffer cbUIInfo : register(b4)
{
	uint		giPlayerHP : packoffset(c0.x);
};

Texture2D gtxtHPTexture : register(t2);

struct VS_UI_OUTPUT
{
	float4 position : SV_POSITION;
	float2 uv : TEXCOORD;
};

VS_UI_OUTPUT VS_UI(uint nVertexID : SV_VertexID)
{
	VS_UI_OUTPUT output;

	if (nVertexID == 0)
	{
		output.position = float4(1.0, 1.0, 0.0, 1.0);
		output.uv = float2(1.0, 0.0);
	}
	else if (nVertexID == 1)
	{
		output.position = float4(1.0, -1.0, 0.0, 1.0);
		output.uv = float2(1.0, 1.0);
	}
	else if (nVertexID == 2)
	{
		output.position = float4(-1.0, -1.0, 0.0, 1.0);
		output.uv = float2(0.0, 1.0);
	}
	else if (nVertexID == 3)
	{
		output.position = float4(-1.0, -1.0, 0.0, 1.0);
		output.uv = float2(0.0, 1.0);
	}
	else if (nVertexID == 4)
	{
		output.position = float4(-1.0, 1.0, 0.0, 1.0);
		output.uv = float2(0.0, 0.0);
	}
	else if (nVertexID == 5)
	{
		output.position = float4(1.0, 1.0, 0.0, 1.0);
		output.uv = float2(1.0, 0.0);
	}

	return(output);
}

float4 PS_UI(VS_UI_OUTPUT input) : SV_TARGET
{
	float4 cColor = gtxtTexture.Sample(gSamplerState, input.uv);
	if (cColor.a < 0.3) discard;

	return(cColor);
}