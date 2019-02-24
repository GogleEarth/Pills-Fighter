struct MATERIAL
{
	float4				m_cAmbient;
	float4				m_cDiffuse;
	float4				m_cSpecular; //a = power
	float4				m_cEmissive;
};

cbuffer cbGameObjectInfo : register(b0)
{
	matrix		gmtxGameObject : packoffset(c0);
	MATERIAL	gMaterial : packoffset(c4);
	uint		gnTexturesMask : packoffset(c8);
};

cbuffer cbCameraInfo : register(b1)
{
	matrix		gmtxView : packoffset(c0);
	matrix		gmtxProjection : packoffset(c4);
	float3		gvCameraPosition : packoffset(c8);
};

cbuffer cbTextureSprite : register(b4)
{
	float4	gfTextureSpriteInfo : packoffset(c0);
};
// b0 ~ b6
#include "Light.hlsl"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

Texture2D gtxtTexture[3] : register(t1); //	t1 ~ t3
Texture2D gtxtSpecularTexture : register(t4);
Texture2D gtxtNormalTexture : register(t5);
TextureCube gtxtSkyCubeTexture : register(t6);

SamplerState gssWrap : register(s0);
SamplerState gssClamp : register(s1);

struct VS_STANDARD_INPUT
{
	float3 position : POSITION;
	float3 normal : NORMAL;
	float3 binormal : BINORMAL;
	float3 tangent : TANGENT;
	float2 uv : TEXCOORD;
};

struct VS_STANDARD_OUTPUT
{
	float4 position : SV_POSITION;
	float3 positionW : POSITION;
	float3 normalW : NORMAL;
	float3 binormalW : BINORMAL;
	float3 tangentW : TANGENT;
	float2 uv : TEXCOORD;
	//float3 reflection : REFLECTION;
};

#define MATERIAL_ALBEDO_MAP				0x01
#define MATERIAL_SPECULAR_FACTOR_MAP	0x02
#define MATERIAL_NORMAL_MAP				0x04

VS_STANDARD_OUTPUT VSTextured(VS_STANDARD_INPUT input)
{
	VS_STANDARD_OUTPUT output;
	
	output.normalW = mul(input.normal, (float3x3)gmtxGameObject);
	output.binormalW = mul(input.binormal, (float3x3)gmtxGameObject);
	output.tangentW = mul(input.tangent, (float3x3)gmtxGameObject);
	output.positionW = (float3)mul(float4(input.position, 1.0f), gmtxGameObject);
	output.position = mul(mul(float4(output.positionW, 1.0f), gmtxView), gmtxProjection);
	output.uv = input.uv;
	//output.reflection = reflect(output.positionW - gvCameraPosition, output.normalW);

	return(output);
}

float4 PSTextured(VS_STANDARD_OUTPUT input) : SV_TARGET
{
	//float4 cCubeColor = gtxtSkyCubeTexture.Sample(gssClamp, input.reflection);

	// 임시 텍스처 배열 인덱스는 0
	float4 f4AlbedoColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
	if(gnTexturesMask & MATERIAL_ALBEDO_MAP)
		f4AlbedoColor = gtxtTexture[0].Sample(gssWrap, input.uv);

	float4 f4NormalColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
	if (gnTexturesMask & MATERIAL_NORMAL_MAP)
		f4NormalColor = gtxtNormalTexture.Sample(gssWrap, input.uv);

	float fSpecularFactor = 0.0f;
	if (gnTexturesMask & MATERIAL_SPECULAR_FACTOR_MAP)
		fSpecularFactor = gtxtSpecularTexture.Sample(gssWrap, input.uv).x;

	float4 cIllumination = float4(1.0f, 1.0f, 1.0f, 1.0f);
	float4 cColor = f4AlbedoColor;
	//float4 cColor = lerp(f4AlbedoColor, cCubeColor, 0.7);

	float3 normalW = normalize(input.normalW);

	if (gnTexturesMask & MATERIAL_NORMAL_MAP)
	{
		float3x3 TBN = float3x3(normalize(input.tangentW), normalize(input.binormalW), normalW);
		float3 vNormal = normalize(f4NormalColor.rgb * 2.0f - 1.0f); //[0, 1] (Color) → [-1, 1]
		normalW = normalize(mul(vNormal, TBN));
	}

	cIllumination = Lighting(input.positionW, normalW, gMaterial, fSpecularFactor);

	return(cColor * cIllumination);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
struct VS_WIRE_INPUT
{
	float3 position : POSITION;
};

struct VS_WIRE_OUTPUT
{
	float4 position : SV_POSITION;
};

VS_WIRE_OUTPUT VSWire(VS_WIRE_INPUT input)
{
	VS_WIRE_OUTPUT output;

	output.position = mul(mul(mul(float4(input.position, 1.0f), gmtxGameObject), gmtxView), gmtxProjection);

	return(output);
}

float4 PSWire(VS_WIRE_OUTPUT input) : SV_TARGET
{
	return(float4(1.0f, 0.0f, 0.0f, 1.0f));
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

#define MAX_VERTEX_INFLUENCES			4
#define SKINNED_ANIMATION_BONES			128

cbuffer cbBoneOffsets : register(b5)
{
	float4x4 gpmtxBoneOffsets[SKINNED_ANIMATION_BONES];
};

cbuffer cbBoneTransforms : register(b6)
{
	float4x4 gpmtxBoneTransforms[SKINNED_ANIMATION_BONES];
};

struct VS_SKINNED_STANDARD_INPUT
{
	float3 position : POSITION;
	float3 normal : NORMAL;
	float3 binormal : BINORMAL;
	float3 tangent : TANGENT;
	float2 uv : TEXCOORD;
	uint4 indices : BONEINDEX;
	float4 weights : BONEWEIGHT;
};

VS_STANDARD_OUTPUT VSSkinnedAnimationStandard(VS_SKINNED_STANDARD_INPUT input)
{
	VS_STANDARD_OUTPUT output;

	output.positionW = float3(0.0f, 0.0f, 0.0f);
	output.normalW = float3(0.0f, 0.0f, 0.0f);
	output.tangentW = float3(0.0f, 0.0f, 0.0f);
	output.binormalW = float3(0.0f, 0.0f, 0.0f);

	matrix mtxVertexToBoneWorld;

	for (int i = 0; i < MAX_VERTEX_INFLUENCES; i++)
	{
		if (input.weights[i] < 0.00000001)
			continue;

		mtxVertexToBoneWorld = mul(gpmtxBoneOffsets[input.indices[i]], gpmtxBoneTransforms[input.indices[i]]);
		//mtxVertexToBoneWorld = gpmtxBoneOffsets[input.indices[i]];
		//mtxVertexToBoneWorld = gpmtxBoneTransforms[input.indices[i]];
		output.positionW += input.weights[i] * mul(float4(input.position, 1.0f), mtxVertexToBoneWorld).xyz;
		output.normalW += input.weights[i] * mul(input.normal, (float3x3)mtxVertexToBoneWorld);
		output.tangentW += input.weights[i] * mul(input.tangent, (float3x3)mtxVertexToBoneWorld);
		output.binormalW += input.weights[i] * mul(input.binormal, (float3x3)mtxVertexToBoneWorld);
	}

	output.position = mul(mul(float4(output.positionW, 1.0f), gmtxView), gmtxProjection);
	output.uv = input.uv;
	//output.reflection = reflect(output.positionW - gvCameraPosition, output.normalW);

	return(output);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

cbuffer cbUIInfo : register(b3)
{
	int		giMaxValue;// : packoffset(c0.x);
	int		giValue; //: packoffset(c0.y);
};

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

[maxvertexcount(4)]
void GS_UI_Bar(point VS_UI_OUTPUT input[1], uint primID : SV_PrimitiveID, inout TriangleStream<GS_OUT> outStream)
{
	float yPos = float(giValue) / float(giMaxValue);
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
	fUVs[0] = float2(0.0f, yPos);
	fUVs[1] = float2(0.0f, 0.0f);
	fUVs[2] = float2(1.0f, yPos);
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
	// 임시 텍스처 배열 인덱스는 0
	float4 cColor = gtxtTexture[0].Sample(gssWrap, input.uv);

	return(cColor);
}

///////////////////////////////////////////////////////////////////////////////////////////

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
	// 지형 텍스처 배열 인덱스 0 = Base, 1 = Detail
	float4 cBaseTexColor = gtxtTexture[0].Sample(gssWrap, input.uv0);
	float4 cDetailTexColor = gtxtTexture[1].Sample(gssWrap, input.uv1);

	float4 cColor = input.color * saturate( (cBaseTexColor * 0.5f) + (cDetailTexColor * 0.5f));

	return(cColor);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
struct VS_SKYBOX_CUBEMAP_INPUT
{
	float3 position : POSITION;
};

struct VS_SKYBOX_CUBEMAP_OUTPUT
{
	float3	positionL : POSITION;
	float4	position : SV_POSITION;
};

VS_SKYBOX_CUBEMAP_OUTPUT VSSkyBox(VS_SKYBOX_CUBEMAP_INPUT input)
{
	VS_SKYBOX_CUBEMAP_OUTPUT output;

	output.position = mul(mul(mul(float4(input.position, 1.0f), gmtxGameObject), gmtxView), gmtxProjection);
	output.positionL = input.position;

	return(output);
}

float4 PSSkyBox(VS_SKYBOX_CUBEMAP_OUTPUT input) : SV_TARGET
{
	float4 cColor = gtxtSkyCubeTexture.Sample(gssClamp, input.positionL);

	return(cColor);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

struct VS_SPRITE_INPUT
{
	float3 position : POSITION;
	float2 uv : TEXCOORD;
};

struct VS_SPRITE_OUTPUT
{
	float4 position : SV_POSITION;
	float2 uv : TEXCOORD;
};

VS_SPRITE_OUTPUT VSSprite(VS_SPRITE_INPUT input)
{
	VS_SPRITE_OUTPUT output;

	output.position = mul(mul(mul(float4(input.position, 1.0f), gmtxGameObject), gmtxView), gmtxProjection);

	float3x3 f3x3Sprite = float3x3(gfTextureSpriteInfo.x, 0.0f, 0.0f, 0.0f, gfTextureSpriteInfo.y, 0.0f, input.uv.x * gfTextureSpriteInfo.x, input.uv.y * gfTextureSpriteInfo.y, 1.0f);
	float3 f3Sprite = float3(gfTextureSpriteInfo.zw, 1.0f);
	output.uv = (float2)mul(f3Sprite, f3x3Sprite);

	return(output);
}

float4 PSSprite(VS_SPRITE_OUTPUT input, uint primitiveID : SV_PrimitiveID) : SV_TARGET
{
	// 임시 텍스처 배열 인덱스는 0
	float4 cColor = gtxtTexture[0].SampleLevel(gssWrap, input.uv, 0);

	return(cColor);
}
