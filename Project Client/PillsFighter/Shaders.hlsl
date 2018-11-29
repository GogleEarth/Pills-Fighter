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

struct VS_UI_INPUT
{
	float2 center : POSITION;
	float2 size : SIZE;
};

struct VS_UI_OUTPUT
{
	float2 center : POSITION;
	float2 size : SIZE;
};

VS_UI_OUTPUT VS_UI(VS_UI_INPUT input)
{
	VS_UI_OUTPUT output;

	output.center = input.center;
	output.size = input.size;	
	//output.center = float3(0.0f, 0.0f, 0.0f);
	//output.size = float2(1.0f, 1.0f);

	return(output);
}

struct GS_OUT
{
	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD;
};

[maxvertexcount(4)]
void GS_UI(point VS_UI_OUTPUT input[1], uint primID : SV_PrimitiveID, inout TriangleStream<GS_OUT> outStream)
{
	float2 vUp = float2(0.0f, 1.0f);
	float2 vRight = float2(1.0f, 0.0f);
	float fHalfW = input[0].size.x;
	float fHalfH = input[0].size.y;

	float4 fVertices[4];
	fVertices[0] = float4(input[0].center + fHalfW * vRight, 0.0f, 1.0f);
	fVertices[1] = float4(input[0].center + fHalfW * vRight + fHalfH * vUp, 0.0f, 1.0f);
	fVertices[2] = float4(input[0].center - fHalfW * vRight, 0.0f, 1.0f);
	fVertices[3] = float4(input[0].center - fHalfW * vRight + fHalfH * vUp, 0.0f, 1.0f);

	float2 fUVs[4];
	fUVs[0] = float2(0.0f, 1.0f);
	fUVs[1] = float2(0.0f, 0.0f);
	fUVs[2] = float2(1.0f, 1.0f);
	fUVs[3] = float2(1.0f, 0.0f);

	GS_OUT output;

	for (int i = 0; i < 4; i++)
	{
		output.pos = fVertices[i];
		output.uv = fUVs[i];

		outStream.Append(output);
	}
}

cbuffer cbUIInfo : register(b4)
{
	int		giPlayerMaxHP;// : packoffset(c0.x);
	int		giPlayerHP; //: packoffset(c0.y);
};

[maxvertexcount(4)]
void GS_UI_HP(point VS_UI_OUTPUT input[1], uint primID : SV_PrimitiveID, inout TriangleStream<GS_OUT> outStream)
{
	float yPos = float(giPlayerHP) / float(giPlayerMaxHP);
	float2 vUp = float2(0.0f, yPos);
	float2 vRight = float2(1.0f, 0.0f);
	float fHalfW = input[0].size.x;
	float fHalfH = input[0].size.y;

	float4 fVertices[4];
	fVertices[0] = float4(input[0].center + fHalfW * vRight, 0.0f, 1.0f);
	fVertices[1] = float4(input[0].center + fHalfW * vRight + fHalfH * vUp, 0.0f, 1.0f);
	fVertices[2] = float4(input[0].center - fHalfW * vRight, 0.0f, 1.0f);
	fVertices[3] = float4(input[0].center - fHalfW * vRight + fHalfH * vUp, 0.0f, 1.0f);

	float2 fUVs[4];
	fUVs[0] = float2(0.0f, 1.0f);
	fUVs[1] = float2(0.0f, 0.0f);
	fUVs[2] = float2(1.0f, 1.0f);
	fUVs[3] = float2(1.0f, 0.0f);

	GS_OUT output;

	for (int i = 0; i < 4; i++)
	{
		output.pos = fVertices[i];
		output.uv = fUVs[i];

		outStream.Append(output);
	}
}

float4 PS_UI(GS_OUT input) : SV_TARGET
{
	float4 cColor = gtxtTexture.Sample(gSamplerState, input.uv);
	if (cColor.a < 0.3) discard;

	return(cColor);
}


///////////////////////////////////////////////////////////////////////////////////////////


Texture2D gtxtTerrainBaseTexture : register(t3);

struct VS_TERRAIN_INPUT
{
	float3 position : POSITION;
	float4 color : COLOR;
	float2 uv0 : TEXCOORD0;
	float2 uv1 : TEXCOORD1;
	//uint heightCount : HEIGHTCOUNT;
};

struct VS_TERRAIN_OUTPUT
{
	float4 position : SV_POSITION;
	float4 color : COLOR;
	float2 uv0 : TEXCOORD0;
	float2 uv1 : TEXCOORD1;
	//uint heightCount : HEIGHTCOUNT;
};

VS_TERRAIN_OUTPUT VSTerrain(VS_TERRAIN_INPUT input)
{
	VS_TERRAIN_OUTPUT output;

	output.position = mul(mul(mul(float4(input.position, 1.0f), gmtxGameObject), gmtxView), gmtxProjection);
	output.color = input.color;
	output.uv0 = input.uv0;
	output.uv1 = input.uv1;
	//output.heightCount = input.heightCount;

	return(output);
}

float4 PSTerrain(VS_TERRAIN_OUTPUT input) : SV_TARGET
{
	float4 cBaseTexColor = gtxtTerrainBaseTexture.Sample(gSamplerState, input.uv0);

	float4 cColor = input.color * saturate(cBaseTexColor * 0.5f);

	return(cColor);
}

