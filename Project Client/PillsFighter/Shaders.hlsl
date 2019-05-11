struct MATERIAL
{
	float4				m_cAmbient;
	float4				m_cDiffuse;
	float4				m_cSpecular; //a = power
	float4				m_cEmissive;
	float				m_cReflectionFactor;
};

cbuffer cbGameObjectInfo : register(b0)
{
	matrix		gmtxGameObject : packoffset(c0);
	MATERIAL	gMaterial : packoffset(c4);
	uint		gnTexturesMask : packoffset(c8.y);
};

cbuffer cbCameraInfo : register(b1)
{
	matrix		gmtxView : packoffset(c0);
	matrix		gmtxProjection : packoffset(c4);
	float3		gvCameraPosition : packoffset(c8);
};

// b0 ~ b6
#include "Light.hlsl"
#include "Math.hlsl"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
Texture2D gtxtTexture[3] : register(t1); //	t1 ~ t3
Texture2D gtxtSpecularTexture : register(t4);
Texture2D gtxtNormalTexture : register(t5);
TextureCube gtxtSkyCubeTexture : register(t6);

TextureCube gtxtEnvirCubeTexture : register(t13);

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
	float3 reflection : REFLECTION;
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
	output.reflection = reflect(output.positionW - gvCameraPosition, output.normalW);

	return(output);
}

float4 PSTextured(VS_STANDARD_OUTPUT input) : SV_TARGET
{
	//float4 cCubeColor = gtxtSkyCubeTexture.Sample(gssClamp, input.reflection);
	float4 cCubeColor = gtxtEnvirCubeTexture.Sample(gssClamp, input.reflection);
	cCubeColor.w = gMaterial.m_cReflectionFactor;

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
	//float4 cColor = f4AlbedoColor;
	float4 cColor = lerp(f4AlbedoColor, cCubeColor, 0.5);

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

struct INSTANCINGOBJECTINFO
{
	matrix		m_mtxGameObject;
	MATERIAL	m_Material;
	uint		m_nTexturesMask;
};

struct VS_INSTANCING_OUTPUT
{
	float4 position : SV_POSITION;
	float3 positionW : POSITION;
	float3 normalW : NORMAL;
	float3 binormalW : BINORMAL;
	float3 tangentW : TANGENT;
	float2 uv : TEXCOORD;
	//float3 reflection : REFLECTION;
	uint instanceID : SV_InstanceID;
};

StructuredBuffer<INSTANCINGOBJECTINFO> gGameObjectsInfo : register(t12);

VS_INSTANCING_OUTPUT VSInsTextured(VS_STANDARD_INPUT input, uint nInsID : SV_InstanceID)
{
	VS_INSTANCING_OUTPUT output;

	output.normalW = mul(input.normal, (float3x3)gGameObjectsInfo[nInsID].m_mtxGameObject);
	output.binormalW = mul(input.binormal, (float3x3)gGameObjectsInfo[nInsID].m_mtxGameObject);
	output.tangentW = mul(input.tangent, (float3x3)gGameObjectsInfo[nInsID].m_mtxGameObject);
	output.positionW = (float3)mul(float4(input.position, 1.0f), gGameObjectsInfo[nInsID].m_mtxGameObject);
	output.position = mul(mul(float4(output.positionW, 1.0f), gmtxView), gmtxProjection);
	output.uv = input.uv;
	//output.reflection = reflect(output.positionW - gvCameraPosition, output.normalW);
	output.instanceID = nInsID;

	return(output);
}

float4 PSInsTextured(VS_INSTANCING_OUTPUT input) : SV_TARGET
{
	//float4 cCubeColor = gtxtSkyCubeTexture.Sample(gssClamp, input.reflection);

	// 임시 텍스처 배열 인덱스는 0
	float4 f4AlbedoColor = float4(1.0f, 0.0f, 0.0f, 1.0f);
	uint nTexMask = gGameObjectsInfo[input.instanceID].m_nTexturesMask;

	if (nTexMask & MATERIAL_ALBEDO_MAP)
		f4AlbedoColor = gtxtTexture[0].Sample(gssWrap, input.uv);

	float4 f4NormalColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
	if (nTexMask & MATERIAL_NORMAL_MAP)
		f4NormalColor = gtxtNormalTexture.Sample(gssWrap, input.uv);

	float fSpecularFactor = 0.0f;
	if (nTexMask & MATERIAL_SPECULAR_FACTOR_MAP)
		fSpecularFactor = gtxtSpecularTexture.Sample(gssWrap, input.uv).x;

	float4 cIllumination = float4(1.0f, 1.0f, 1.0f, 1.0f);
	float4 cColor = f4AlbedoColor;
	//float4 cColor = lerp(f4AlbedoColor, cCubeColor, 0.7);

	float3 normalW = normalize(input.normalW);

	if (nTexMask & MATERIAL_NORMAL_MAP)
	{
		float3x3 TBN = float3x3(normalize(input.tangentW), normalize(input.binormalW), normalW);
		float3 vNormal = normalize(f4NormalColor.rgb * 2.0f - 1.0f); //[0, 1] (Color) → [-1, 1]
		normalW = normalize(mul(vNormal, TBN));
	}

	cIllumination = Lighting(input.positionW, normalW, gGameObjectsInfo[input.instanceID].m_Material, fSpecularFactor);

	return(cColor * cIllumination);
}

VS_WIRE_OUTPUT VSInsWire(VS_WIRE_INPUT input, uint nInsID : SV_InstanceID)
{
	VS_WIRE_OUTPUT output;

	output.position = mul(mul(mul(float4(input.position, 1.0f), gGameObjectsInfo[nInsID].m_mtxGameObject), gmtxView), gmtxProjection);

	return(output);
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
		output.positionW += input.weights[i] * mul(float4(input.position, 1.0f), mtxVertexToBoneWorld).xyz;
		output.normalW += input.weights[i] * mul(input.normal, (float3x3)mtxVertexToBoneWorld);
		output.tangentW += input.weights[i] * mul(input.tangent, (float3x3)mtxVertexToBoneWorld);
		output.binormalW += input.weights[i] * mul(input.binormal, (float3x3)mtxVertexToBoneWorld);
	}

	output.position = mul(mul(float4(output.positionW, 1.0f), gmtxView), gmtxProjection);
	output.uv = input.uv;
	output.reflection = reflect(output.positionW - gvCameraPosition, output.normalW);

	return(output);
}

///////////////////////////////////////////////////////////////////////////////////////////
Texture2D gtxtTileTexture[5] : register(t7); //	t7~t11

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
	return(input);
}

struct GS_UI_OUT
{
	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD;
};

[maxvertexcount(4)]
void GS_UI(point VS_UI_OUTPUT input[1], uint primID : SV_PrimitiveID, inout TriangleStream<GS_UI_OUT> outStream)
{
	float2 vUp = float2(0.0f, 1.0f);
	float2 vRight = float2(1.0f, 0.0f);
	float fHalfW = input[0].size.x;
	float fHalfH = input[0].size.y;

	float4 fVertices[4];
	fVertices[0] = float4(input[0].center - fHalfW * vRight - fHalfH * vUp, 0.0f, 1.0f);
	fVertices[1] = float4(input[0].center - fHalfW * vRight, 0.0f, 1.0f);
	fVertices[2] = float4(input[0].center + fHalfW * vRight - fHalfH * vUp, 0.0f, 1.0f);
	fVertices[3] = float4(input[0].center + fHalfW * vRight, 0.0f, 1.0f);

	float2 fUVs[4];
	fUVs[0] = float2(0.0f, 1.0f);
	fUVs[1] = float2(0.0f, 0.0f);
	fUVs[2] = float2(1.0f, 1.0f);
	fUVs[3] = float2(1.0f, 0.0f);

	GS_UI_OUT output;

	for (int i = 0; i < 4; i++)
	{
		output.pos = fVertices[i];
		output.uv = fUVs[i];

		outStream.Append(output);
	}
}

[maxvertexcount(4)]
void GS_UI_Bar(point VS_UI_OUTPUT input[1], uint primID : SV_PrimitiveID, inout TriangleStream<GS_UI_OUT> outStream)
{
	float yPos = float(giValue) / float(giMaxValue);
	float2 vValueByUp = float2(0.0f, 1.0f - yPos);
	float2 vUp = float2(0.0f, 1.0f);
	float2 vRight = float2(1.0f, 0.0f);
	float fHalfW = input[0].size.x;
	float fHalfH = input[0].size.y;

	float4 fVertices[4];
	fVertices[0] = float4(input[0].center - fHalfW * vRight - fHalfH * vUp, 0.0f, 1.0f);
	fVertices[1] = float4(input[0].center - fHalfW * vRight - fHalfH * vValueByUp, 0.0f, 1.0f);
	fVertices[2] = float4(input[0].center + fHalfW * vRight - fHalfH * vUp, 0.0f, 1.0f);
	fVertices[3] = float4(input[0].center + fHalfW * vRight - fHalfH * vValueByUp, 0.0f, 1.0f);

	float2 fUVs[4];
	fUVs[0] = float2(0.0f, 1.0f);
	fUVs[1] = float2(0.0f, 0.0f);
	fUVs[2] = float2(1.0f, 1.0f);
	fUVs[3] = float2(1.0f, 0.0f);

	GS_UI_OUT output;

	for (int i = 0; i < 4; i++)
	{
		output.pos = fVertices[i];
		output.uv = fUVs[i];

		outStream.Append(output);
	}
}

float4 PS_UI(GS_UI_OUT input) : SV_TARGET
{
	// 임시 텍스처 배열 인덱스는 0
	float4 cColor = gtxtTexture[0].Sample(gssWrap, input.uv);

	return(cColor);
}

float4 PS_UI_BULLET(GS_UI_OUT input) : SV_TARGET
{
	float2 vNewUV = input.uv;
	float fStride = 1.0f / float(giValue);
	vNewUV.y = 1 - fmod(vNewUV.y, fStride) * giValue;

	float4 cColor = gtxtTexture[0].Sample(gssWrap, vNewUV);

	return(cColor);
}

float4 PS_UI_MINIMAP(GS_UI_OUT input) : SV_TARGET
{
	float4 cColor = gtxtTexture[0].Sample(gssWrap, input.uv);

	float2 centerpos = float2(0.5f, 0.5f);
	cColor = (distance(input.uv, centerpos) < 0.015) ? float4(1, 0, 0, 1) : cColor;
	cColor = (distance(input.uv, centerpos) < 0.49) ? cColor : 0;
	cColor = (distance(input.uv, centerpos) >= 0.49) ? float4(0.5, 0.2, 0.05, 1) : cColor;
	cColor = (distance(input.uv, centerpos) < 0.5) ? cColor : 0;

	return(cColor);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

struct EFFECT
{
	float	m_fElapsedTime;
	float	m_fDuration;
};

ConstantBuffer<EFFECT> gEffect : register(b7);

struct VS_EFFECT_INPUT
{
	float3 position : POSITION;
	float age : AGE;
	float2 size : SIZE;
	uint texindex : TEXINDEX;
};

struct VS_EFFECT_OUTPUT
{
	float3 position : POSITION;
	uint texindex : TEXINDEX;
	float4 color : COLOR;
	float2 size : SIZE;
};

VS_EFFECT_OUTPUT VSEffectDraw(VS_EFFECT_INPUT input)
{
	VS_EFFECT_OUTPUT output;

	float fOpacity = 1.0f - smoothstep(0.0f, gEffect.m_fDuration, input.age);
	output.color = float4(1.0f, 1.0f, 1.0f, fOpacity);

	output.size = input.size;
	output.position = input.position;
	output.texindex = input.texindex;

	return output;
}

struct GS_EFFECT_OUTPUT
{
	float4 position : SV_POSITION;
	float4 color : COLOR;
	float2 uv : TEXCOORD;
	uint texindex : TEXINDEX;
};

[maxvertexcount(4)]
void GSEffectDraw(point VS_EFFECT_OUTPUT input[1], inout TriangleStream<GS_EFFECT_OUTPUT> outStream)
{
	float3 vUp = float3(0.0f, 1.0f, 0.0f);
	float3 vLook = normalize(gvCameraPosition.xyz - input[0].position);
	float3 vRight = normalize(cross(vUp, vLook));
	float fDistance = distance(gvCameraPosition.xyz, input[0].position);

	float fHalfW = input[0].size.x + fDistance * input[0].size.x;
	float fHalfH = input[0].size.y + fDistance * input[0].size.y;

	float4 fVertices[4];
	fVertices[0] = float4(input[0].position + fHalfW * vRight - fHalfH * vUp, 1.0f);
	fVertices[1] = float4(input[0].position + fHalfW * vRight + fHalfH * vUp, 1.0f);
	fVertices[2] = float4(input[0].position - fHalfW * vRight - fHalfH * vUp, 1.0f);
	fVertices[3] = float4(input[0].position - fHalfW * vRight + fHalfH * vUp, 1.0f);

	float2 fUVs[4];
	fUVs[0] = float2(0.0f, 1.0f);
	fUVs[1] = float2(0.0f, 0.0f);
	fUVs[2] = float2(1.0f, 1.0f);
	fUVs[3] = float2(1.0f, 0.0f);

	GS_EFFECT_OUTPUT output;
	output.texindex = input[0].texindex;
	output.color = input[0].color;

	for (int i = 0; i < 4; i++)
	{
		output.position = mul(mul(fVertices[i], gmtxView), gmtxProjection);
		output.uv = fUVs[i];

		outStream.Append(output);
	}
}

float4 PSEffectDraw(GS_EFFECT_OUTPUT input) : SV_TARGET
{
	float4 cColor = gtxtTexture[NonUniformResourceIndex(input.texindex)].Sample(gssWrap, input.uv);

	return(cColor *input.color);
}

VS_EFFECT_INPUT VSEffectStreamOut(VS_EFFECT_INPUT input)
{
	return input;
}

[maxvertexcount(1)]
void GSEffectStreamOut(point VS_EFFECT_INPUT input[1], inout PointStream<VS_EFFECT_INPUT> outStream)
{
	input[0].age += gEffect.m_fElapsedTime;

	if (input[0].age <= gEffect.m_fDuration)
		outStream.Append(input[0]);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

#define EFFECT_TYPE_SPRITE_ONE 0
#define EFFECT_TYPE_SPRITE_LOOP 1

struct SPRITE
{
	float2 m_f2SpriteSize;
	uint m_nMaxSpriteX;
	uint m_nMaxSpriteY;
	uint m_nMaxSprite;
	float m_fDurationPerSprite;
};

ConstantBuffer<SPRITE> gSprite : register(b4);

struct VS_SPRITE_INPUT
{
	float3 position : POSITION;
	float2 size : SIZE;
	uint2 spritepos : SPRITEPOS;
	float age : AGE;
	uint texindex : TEXINDEX;
	uint type : TYPE;
};

struct VS_SPRITE_OUTPUT
{
	float3 position : POSITION;
	float2 size : SIZE;
	uint2 spritepos : SPRITEPOS;
	uint texindex : TEXINDEX;
};

struct GS_SPRITE_OUTPUT
{
	float4 position : SV_POSITION;
	float2 uv : TEXCOORD;
	uint texindex : TEXINDEX;
};

VS_SPRITE_OUTPUT VSSpriteDraw(VS_SPRITE_INPUT input)
{
	VS_SPRITE_OUTPUT output;

	output.position = input.position;
	output.size = input.size;
	output.spritepos = input.spritepos;
	output.texindex = input.texindex;

	return output;
}

[maxvertexcount(4)]
void GSSpriteDraw(point VS_SPRITE_OUTPUT input[1], inout TriangleStream<GS_SPRITE_OUTPUT> outStream)
{
	float3 vUp = float3(0.0f, 1.0f, 0.0f);
	float3 vLook = normalize(gvCameraPosition.xyz - input[0].position);
	float3 vRight = normalize(cross(vUp, vLook));

	float fHalfW = input[0].size.x;
	float fHalfH = input[0].size.y;

	float4 fVertices[4];
	fVertices[0] = float4(input[0].position + fHalfW * vRight - fHalfH * vUp, 1.0f);
	fVertices[1] = float4(input[0].position + fHalfW * vRight + fHalfH * vUp, 1.0f);
	fVertices[2] = float4(input[0].position - fHalfW * vRight - fHalfH * vUp, 1.0f);
	fVertices[3] = float4(input[0].position - fHalfW * vRight + fHalfH * vUp, 1.0f);

	float2 fUVs[4];
	fUVs[0] = float2(0.0f, 1.0f);
	fUVs[1] = float2(0.0f, 0.0f);
	fUVs[2] = float2(1.0f, 1.0f);
	fUVs[3] = float2(1.0f, 0.0f);

	GS_SPRITE_OUTPUT output;
	output.texindex = input[0].texindex;

	for (int i = 0; i < 4; i++)
	{
		output.position = mul(mul(fVertices[i], gmtxView), gmtxProjection);
		float3x3 f3x3Sprite = float3x3(gSprite.m_f2SpriteSize.x, 0.0f, 0.0f, 0.0f, gSprite.m_f2SpriteSize.y, 0.0f, fUVs[i].x * gSprite.m_f2SpriteSize.x, fUVs[i].y * gSprite.m_f2SpriteSize.y, 1.0f);
		float3 f3Sprite = float3(input[0].spritepos, 1.0f);
		output.uv = (float2)mul(f3Sprite, f3x3Sprite);

		outStream.Append(output);
	}
}

float4 PSSpriteDraw(GS_SPRITE_OUTPUT input) : SV_TARGET
{
	float4 cColor = gtxtTexture[NonUniformResourceIndex(input.texindex)].Sample(gssWrap, input.uv);

	return(cColor);
}

VS_SPRITE_INPUT VSSpriteStreamOut(VS_SPRITE_INPUT input)
{
	return input;
}

[maxvertexcount(1)]
void GSSpriteStreamOut(point VS_SPRITE_INPUT input[1], inout PointStream<VS_SPRITE_INPUT> outStream)
{
	uint nSpritePos = input[0].spritepos.x + input[0].spritepos.y * gSprite.m_nMaxSpriteX;

	if (nSpritePos == gSprite.m_nMaxSprite)
	{
		if (input[0].type == EFFECT_TYPE_SPRITE_ONE) return;
		else if(input[0].type == EFFECT_TYPE_SPRITE_LOOP)
		{
			input[0].spritepos.x = input[0].spritepos.y = 0;
			input[0].age = 0.0f;
		}
	}
	else
	{
		input[0].age += gEffect.m_fElapsedTime;

		float fNextSpriteTime = nSpritePos * gSprite.m_fDurationPerSprite;

		if (input[0].age >= fNextSpriteTime)
		{
			input[0].spritepos.x += 1;

			if (input[0].spritepos.x == gSprite.m_nMaxSpriteX)
			{
				input[0].spritepos.x = 0;
				input[0].spritepos.y += 1;
			}
		}
	}

	outStream.Append(input[0]);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

#define PARTICLE_TYPE_EMITTER 0

struct PARTICLE
{
	float4	m_vRandom;
	float3	m_vPosition;
	float	m_fSpeed;
	float3	m_vDirection;
	float	m_fDuration;
	float3  m_vRight;
	float	m_fElapsedTime;
	float3  m_vUp;
	float	m_fEmitInterval;
	float3  m_vLook;
	bool	m_bEmit;
	float3	m_vAngles;
};

ConstantBuffer<PARTICLE> gParticle : register(b8);

struct VS_PARTICLE_INPUT
{
	float3	position : POSITION;
	float3	velocity : VELOCITY;
	float2	size : SIZE;
	uint	type : TYPE;
	float	age : AGE;
};

struct VS_PARTICLE_SO_OUTPUT
{
	float3	position : POSITION;
	float3	velocity : VELOCITY;
	float2	size : SIZE;
	uint	type : TYPE;
	float	age : AGE;
	uint	verid : VERTEXID;
};

VS_PARTICLE_SO_OUTPUT VSParticleStreamOut(VS_PARTICLE_INPUT input, uint nVerID : SV_VertexID)
{
	VS_PARTICLE_SO_OUTPUT output;
	output.position = input.position;
	output.velocity = input.velocity;
	output.size = input.size;
	output.type = input.type;
	output.age = input.age;
	output.verid = nVerID;

	return output;
}

[maxvertexcount(2)]
void GSParticleStreamOut(point VS_PARTICLE_SO_OUTPUT input[1], inout PointStream<VS_PARTICLE_INPUT> pointStream)
{
	VS_PARTICLE_INPUT output;
	output.position = input[0].position;
	output.velocity = input[0].velocity;
	output.size = input[0].size;
	output.type = input[0].type;
	output.age = input[0].age + gParticle.m_fElapsedTime;

	if (output.type == PARTICLE_TYPE_EMITTER)
	{
		if ((gParticle.m_bEmit == true) && (output.age > gParticle.m_fEmitInterval))
		{
			float4 vRandom = gParticle.m_vRandom * input[0].verid * input[0].verid;
			float fX = fmod(vRandom.x, gParticle.m_vAngles.x) - gParticle.m_vAngles.x / 2.0f;
			float fY = fmod(vRandom.y, gParticle.m_vAngles.y) - gParticle.m_vAngles.y / 2.0f;
			float fZ = fmod(vRandom.z, gParticle.m_vAngles.z) - gParticle.m_vAngles.z / 2.0f;

			float3 vDirection = gParticle.m_vDirection;

			if (fX > 0.0001f) vDirection = mul(vDirection, RotateAxis(gParticle.m_vRight, fX));
			if (fY > 0.0001f) vDirection = mul(vDirection, RotateAxis(gParticle.m_vUp, fY));
			if (fZ > 0.0001f) vDirection = mul(vDirection, RotateAxis(gParticle.m_vLook, fZ));

			VS_PARTICLE_INPUT particle;
			particle.position = output.position + gParticle.m_vPosition;
			particle.velocity = gParticle.m_fSpeed * vDirection;
			particle.size = output.size;
			particle.age = gParticle.m_fElapsedTime;
			particle.type = 1;

			pointStream.Append(particle);

			output.age = 0.0f;
		}

		pointStream.Append(output);
	}
	else
	{
		if (output.age < gParticle.m_fDuration) pointStream.Append(output);
	}
}

////////////////////////////////////////////

cbuffer cbSceneInfo : register(b11)
{
	float gfGravity;
}

struct VS_PARTICLE_OUTPUT
{
	float3 position : POSITION;
	float2 size : SIZE;
	float4 color : COLOR;
	uint type : TYPE;
};

struct GS_PARTICLE_OUTPUT
{
	float4 position : SV_POSITION;
	float4 color : COLOR;
	float2 uv : TEXCOORD;
};

VS_PARTICLE_OUTPUT VSParticleDraw(VS_PARTICLE_INPUT input)
{
	VS_PARTICLE_OUTPUT output;

	float t = input.age;
	output.position = (input.velocity * t * (gfGravity * 0.05f)) + input.position;

	float fOpacity = 1.0f - smoothstep(0.0f, gParticle.m_fDuration, input.age);
	output.color = float4(1.0f, 1.0f, 1.0f, fOpacity);

	output.size = input.size;
	output.type = input.type;

	return output;
}

static float2 gvQuadTexCoord[4] = { float2(0.0f, 1.0f), float2(0.0f, 0.0f), float2(1.0f, 1.0f), float2(1.0f, 0.0f) };

[maxvertexcount(4)]
void GSParticleDraw(point VS_PARTICLE_OUTPUT input[1], inout TriangleStream<GS_PARTICLE_OUTPUT> triStream)
{
	if (input[0].type == PARTICLE_TYPE_EMITTER) return;

	float3 vUp = float3(0.0f, 1.0f, 0.0f);
	float3 vLook = gvCameraPosition.xyz - input[0].position;
	vLook = normalize(vLook);
	vLook.y = 0.0f;
	float3 vRight = normalize(cross(vUp, vLook));

	float fHalfWidth = 0.5f * input[0].size.x;
	float fHalfHeight = 0.5f * input[0].size.y;

	float4 vQuads[4];
	vQuads[0] = float4(input[0].position + fHalfWidth * vRight - fHalfHeight * vUp, 1.0f);
	vQuads[1] = float4(input[0].position + fHalfWidth * vRight + fHalfHeight * vUp, 1.0f);
	vQuads[2] = float4(input[0].position - fHalfWidth * vRight - fHalfHeight * vUp, 1.0f);
	vQuads[3] = float4(input[0].position - fHalfWidth * vRight + fHalfHeight * vUp, 1.0f);

	matrix mtxViewProjection = mul(gmtxView, gmtxProjection);

	GS_PARTICLE_OUTPUT output;
	for (int i = 0; i < 4; i++)
	{
		output.position = mul(vQuads[i], mtxViewProjection);
		output.uv = gvQuadTexCoord[i];
		output.color = input[0].color;
		triStream.Append(output);
	}
}

float4 PSParticleDraw(GS_PARTICLE_OUTPUT input) : SV_TARGET
{
	float4 cColor = gtxtTexture[0].Sample(gssWrap, input.uv) * input.color;

	return(cColor);
}

////////////////////////////////////////////

struct VS_FONT_IN
{
	float2 pos : POSITION;
	float2 size : SIZE;
	float2 uvPos : UVPOSITION;
	float2 uvSize : UVSIZE;
	float4 color : COLOR;
};

VS_FONT_IN VSFont(VS_FONT_IN input)
{
	return input;
}

struct GS_FONT_OUT
{
	float4 position : SV_POSITION;
	float2 uv : TEXCOORD;
	float4 color : COLOR;
};

[maxvertexcount(4)]
void GSFont(point VS_FONT_IN input[1], inout TriangleStream<GS_FONT_OUT> outStream)
{
	float fWidth = input[0].size.x;
	float fHeight = input[0].size.y;
	float2 pos = input[0].pos;

	float4 fVertices[4];
	fVertices[0] = float4(pos.x, pos.y, 0.0f, 1.0f);
	fVertices[1] = float4(pos.x + fWidth, pos.y, 0.0f, 1.0f);
	fVertices[2] = float4(pos.x, pos.y - fHeight, 0.0f, 1.0f);
	fVertices[3] = float4(pos.x + fWidth, pos.y - fHeight, 0.0f, 1.0f);

	float fUVWidth = input[0].uvSize.x;
	float fUVHeight = input[0].uvSize.y;
	float2 uvPos = input[0].uvPos;

	float2 fUvs[4];
	fUvs[0] = float2(uvPos.x, uvPos.y);
	fUvs[1] = float2(uvPos.x + fUVWidth, uvPos.y);
	fUvs[2] = float2(uvPos.x, uvPos.y + fUVHeight);
	fUvs[3] = float2(uvPos.x + fUVWidth, uvPos.y + fUVHeight);

	for (int i = 0; i < 4; i++)
	{
		GS_FONT_OUT output;

		output.position = fVertices[i];
		output.uv = fUvs[i];
		output.color = input[0].color;

		outStream.Append(output);
	}
}
 
float4 PSFont(GS_FONT_OUT input) : SV_TARGET
{
	float4 fColor = gtxtTexture[0].Sample(gssWrap, input.uv);

	return fColor *input.color;
}

////////////////////////////////////////////////////////////////

cbuffer cbCursorInfo : register(b10)
{
	float2 gvCursorPos;
}

[maxvertexcount(4)]
void GSCursor(point VS_UI_INPUT input[1], inout TriangleStream<GS_UI_OUT> outStream)
{
	float2 vUp = float2(0.0f, 1.0f);
	float2 vRight = float2(1.0f, 0.0f);
	float fHalfW = input[0].size.x;
	float fHalfH = input[0].size.y;

	float4 fVertices[4];
	fVertices[0] = float4(gvCursorPos + input[0].center - fHalfW * vRight - fHalfH * vUp, 0.0f, 1.0f);
	fVertices[1] = float4(gvCursorPos + input[0].center - fHalfW * vRight, 0.0f, 1.0f);
	fVertices[2] = float4(gvCursorPos + input[0].center + fHalfW * vRight - fHalfH * vUp, 0.0f, 1.0f);
	fVertices[3] = float4(gvCursorPos + input[0].center + fHalfW * vRight, 0.0f, 1.0f);

	float2 fUVs[4];
	fUVs[0] = float2(0.0f, 1.0f);
	fUVs[1] = float2(0.0f, 0.0f);
	fUVs[2] = float2(1.0f, 1.0f);
	fUVs[3] = float2(1.0f, 0.0f);

	GS_UI_OUT output;

	for (int i = 0; i < 4; i++)
	{
		output.pos = fVertices[i];
		output.uv = fUVs[i];

		outStream.Append(output);
	}
}


////////////////////////////////////////////////////////////////

cbuffer cbMinimapRobotPos : register(b12)
{
	float2 gvMinimapRobotPos[8];
	bool enemyOrTeam[8];
}

cbuffer cbMinimapPlayerPos : register(b13)
{
	float2 gvMinimapPlayerPos;
	float2 gvMinimapPlayerLook;
	float2 gvMinimapPlayerRight;
}

static int robotCnt = 0;

struct GS_UI_MINIMAPROBOT_OUT
{
	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD;
	bool eort : ENEMYORTEAM;
};

[maxvertexcount(4)]
void GSMinimapEnemy(point VS_UI_INPUT input[1], inout TriangleStream<GS_UI_MINIMAPROBOT_OUT> outStream)
{
	float2 vUp = float2(0.0f, 1.0f);
	float2 vRight = float2(1.0f, 0.0f);
	float fHalfW = input[0].size.x;
	float fHalfH = input[0].size.y;

	float2 enemyPos = float2(0.0f, 0.0f);
	enemyPos.x = (gvMinimapRobotPos[robotCnt].x - gvMinimapPlayerPos.x) / 200; // X 변환
	enemyPos.y = (gvMinimapRobotPos[robotCnt].y - gvMinimapPlayerPos.y) / 200; // Z 변환
	bool eOrT = enemyOrTeam[robotCnt];
	robotCnt++;
	if (robotCnt >= 7) {
		robotCnt = 0;
	}

	float4 fVertices[4];
	fVertices[0] = float4(enemyPos + input[0].center - fHalfW * vRight - fHalfH * vUp, 0.0f, 1.0f);
	fVertices[1] = float4(enemyPos + input[0].center - fHalfW * vRight, 0.0f, 1.0f);
	fVertices[2] = float4(enemyPos + input[0].center + fHalfW * vRight - fHalfH * vUp, 0.0f, 1.0f);
	fVertices[3] = float4(enemyPos + input[0].center + fHalfW * vRight, 0.0f, 1.0f);

	float2 fUVs[4];
	fUVs[0] = float2(0.0f, 1.0f);
	fUVs[1] = float2(0.0f, 0.0f);
	fUVs[2] = float2(1.0f, 1.0f);
	fUVs[3] = float2(1.0f, 0.0f);


	GS_UI_MINIMAPROBOT_OUT output;

	for (int i = 0; i < 4; i++)
	{
		output.pos = fVertices[i];
		output.uv = fUVs[i];
		output.eort = eOrT;

		outStream.Append(output);
	}
}


float4 PSMinimapEnemy(GS_UI_MINIMAPROBOT_OUT input) : SV_TARGET
{
	float4 cColor;
	if (input.eort == true) {
		// 1: 적
		cColor = gtxtTexture[0].Sample(gssWrap, input.uv);
	}
	else { cColor = gtxtTexture[1].Sample(gssWrap, input.uv); }

	if (cColor.r >= 0.9 && cColor.g >= 0.9 && cColor.b >= 0.9) discard;

	return(cColor);
}
