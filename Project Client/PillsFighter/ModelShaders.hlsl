#ifndef DEFINE_DEFINE
#define DEFINE_DEFINE
#include "Define.hlsl"
#endif

#ifndef DEFINE_MATH
#define DEFINE_MATH
#include "Math.hlsl"
#endif

#ifndef DEFINE_LIGHT
#define DEFINE_LIGHT
#include "Light.hlsl"
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

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
	float4 shadowPosH : SHADOWPOS;
};

struct PS_OUTPUT
{
	float4 color : SV_Target0;
	float4 glow : SV_Target1;
	float4 normal : SV_Target2;
};

VS_STANDARD_OUTPUT VSStandard(VS_STANDARD_INPUT input)
{
	VS_STANDARD_OUTPUT output;

	output.normalW = mul(input.normal, (float3x3)gmtxGameObject);
	output.binormalW = mul(input.binormal, (float3x3)gmtxGameObject);
	output.tangentW = mul(input.tangent, (float3x3)gmtxGameObject);
	output.positionW = (float3)mul(float4(input.position, 1.0f), gmtxGameObject);
	output.position = mul(float4(output.positionW, 1.0f), gmtxViewProjection);
	output.uv = input.uv;
	output.reflection = reflect(output.positionW - gvCameraPosition, output.normalW);
	output.shadowPosH = mul(float4(output.positionW, 1.0f), gmtxShadowTransform);

	return(output);
}

PS_OUTPUT PSStandard(VS_STANDARD_OUTPUT input) : SV_TARGET
{
	PS_OUTPUT output;

	float4 f4GlowColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
	if (gnTexturesMask & MATERIAL_EMISSION_MAP)
		f4GlowColor = gtxtEmissiveTexture.Sample(gssWrap, input.uv);

	//float4 cCubeColor = gtxtSkyCubeTexture.Sample(gssClamp, input.reflection);
	float4 cCubeColor = gtxtEnvirCubeTexture.Sample(gssClamp, input.reflection);
	cCubeColor.w = gMaterial.m_cReflectionFactor;

	// 임시 텍스처 배열 인덱스는 0
	float4 f4AlbedoColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
	if (gnTexturesMask & MATERIAL_ALBEDO_MAP)
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

	float3 shadowFactor = float3(1.0f, 1.0f, 1.0f);
	shadowFactor = CalcShadowFactor(input.shadowPosH);

	cIllumination = Lighting(input.positionW, normalW, gMaterial, fSpecularFactor, shadowFactor);

	output.color = cColor * cIllumination;
	output.glow = f4GlowColor;
	output.normal = float4(normalW, 1.0f);

	return output;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

struct VS_STANDARD_SHADOW_INPUT
{
	float3 position : POSITION;
	float2 uv : TEXCOORD;
};

struct VS_STANDARD_SHADOW_OUTPUT
{
	float4 position : SV_POSITION;
	float2 uv : TEXCOORD;
};

VS_STANDARD_SHADOW_OUTPUT VSStandardShadow(VS_STANDARD_SHADOW_INPUT input)
{
	VS_STANDARD_SHADOW_OUTPUT output;

	output.position = mul(mul(float4(input.position, 1.0f), gmtxGameObject), gmtxLightViewProjection);
	output.uv = input.uv;

	return(output);
}

void PSStandardShadow(VS_STANDARD_SHADOW_OUTPUT input)
{
	float4 f4AlbedoColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
	if (gnTexturesMask & MATERIAL_ALBEDO_MAP)
		f4AlbedoColor = gtxtTexture[0].Sample(gssWrap, input.uv);

	clip(f4AlbedoColor.a - 0.1f);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

struct PS_PLAYER
{
	float4 color : SV_Target0;
	float4 glow : SV_Target1;
	float4 normal : SV_Target2;
	float4 mask : SV_Target3;
};

PS_PLAYER PSPlayer(VS_STANDARD_OUTPUT input) : SV_TARGET
{
	PS_PLAYER output;

	float4 f4GlowColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
	if (gnTexturesMask & MATERIAL_EMISSION_MAP)
		f4GlowColor = gtxtEmissiveTexture.Sample(gssWrap, input.uv);

	//float4 cCubeColor = gtxtSkyCubeTexture.Sample(gssClamp, input.reflection);
	float4 cCubeColor = gtxtEnvirCubeTexture.Sample(gssClamp, input.reflection);
	cCubeColor.w = gMaterial.m_cReflectionFactor;

	// 임시 텍스처 배열 인덱스는 0
	float4 f4AlbedoColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
	if (gnTexturesMask & MATERIAL_ALBEDO_MAP)
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

	float3 shadowFactor = float3(1.0f, 1.0f, 1.0f);
	shadowFactor = CalcShadowFactor(input.shadowPosH);

	cIllumination = Lighting(input.positionW, normalW, gMaterial, fSpecularFactor, shadowFactor);

	output.color = cColor * cIllumination;
	output.glow = f4GlowColor;
	output.normal = float4(normalW, 1.0f);
	output.mask = 1;

	return output;
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

	output.position = mul(mul(float4(input.position, 1.0f), gmtxGameObject), gmtxViewProjection);

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

StructuredBuffer<INSTANCINGOBJECTINFO> gGameObjectsInfo : register(INSTANCING_GAME_OBJECTS_INFO);

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
	float4 shadowPosH : SHADOWPOS;
};

VS_INSTANCING_OUTPUT VSInsTextured(VS_STANDARD_INPUT input, uint nInsID : SV_InstanceID)
{
	VS_INSTANCING_OUTPUT output;

	output.normalW = mul(input.normal, (float3x3)gGameObjectsInfo[nInsID].m_mtxGameObject);
	output.binormalW = mul(input.binormal, (float3x3)gGameObjectsInfo[nInsID].m_mtxGameObject);
	output.tangentW = mul(input.tangent, (float3x3)gGameObjectsInfo[nInsID].m_mtxGameObject);
	output.positionW = (float3)mul(float4(input.position, 1.0f), gGameObjectsInfo[nInsID].m_mtxGameObject);
	output.position = mul(float4(output.positionW, 1.0f), gmtxViewProjection);
	output.uv = input.uv;
	//output.reflection = reflect(output.positionW - gvCameraPosition, output.normalW);
	output.instanceID = nInsID;
	output.shadowPosH = mul(float4(output.positionW, 1.0f), gmtxShadowTransform);

	return(output);
}

PS_OUTPUT PSInsTextured(VS_INSTANCING_OUTPUT input) : SV_TARGET
{
	PS_OUTPUT output;
	//float4 cCubeColor = gtxtSkyCubeTexture.Sample(gssClamp, input.reflection);

	uint nTexMask = gGameObjectsInfo[input.instanceID].m_nTexturesMask;

	float4 f4GlowColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
	if (nTexMask & MATERIAL_EMISSION_MAP)
		f4GlowColor = gtxtEmissiveTexture.Sample(gssWrap, input.uv);

	// 임시 텍스처 배열 인덱스는 0
	float4 f4AlbedoColor = float4(1.0f, 0.0f, 0.0f, 1.0f);

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

	float3 shadowFactor = float3(1.0f, 1.0f, 1.0f);
	shadowFactor = CalcShadowFactor(input.shadowPosH);

	cIllumination = Lighting(input.positionW, normalW, gGameObjectsInfo[input.instanceID].m_Material, fSpecularFactor, shadowFactor);

	output.color = cColor * cIllumination;
	output.glow = f4GlowColor;
	output.normal = float4(normalW, 1.0f);

	return output;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

struct VS_INSTANCING_SHADOW_OUTPUT
{
	float4 position : SV_POSITION;
	float2 uv : TEXCOORD;
	uint instanceID : SV_InstanceID;
};

VS_INSTANCING_SHADOW_OUTPUT VSInsShadow(VS_STANDARD_SHADOW_INPUT input, uint nInsID : SV_InstanceID)
{
	VS_INSTANCING_SHADOW_OUTPUT output;

	output.position = mul(mul(float4(input.position, 1.0f), gGameObjectsInfo[nInsID].m_mtxGameObject), gmtxLightViewProjection);
	output.uv = input.uv;
	output.instanceID = nInsID;

	return(output);
}

void PSInsShadow(VS_INSTANCING_SHADOW_OUTPUT input)
{
	float4 f4AlbedoColor = float4(1.0f, 0.0f, 0.0f, 1.0f);
	uint nTexMask = gGameObjectsInfo[input.instanceID].m_nTexturesMask;

	if (nTexMask & MATERIAL_ALBEDO_MAP)
		f4AlbedoColor = gtxtTexture[0].Sample(gssWrap, input.uv);

	clip(f4AlbedoColor.a - 0.1f);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

VS_WIRE_OUTPUT VSInsWire(VS_WIRE_INPUT input, uint nInsID : SV_InstanceID)
{
	VS_WIRE_OUTPUT output;

	output.position = mul(mul(float4(input.position, 1.0f), gGameObjectsInfo[nInsID].m_mtxGameObject), gmtxViewProjection);

	return(output);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

#define MAX_VERTEX_INFLUENCES			4
#define SKINNED_ANIMATION_BONES			128

cbuffer cbBoneOffsets : register(BONE_OFFSETS)
{
	float4x4 gpmtxBoneOffsets[SKINNED_ANIMATION_BONES];
};

cbuffer cbBoneTransforms : register(BONE_TRANSFOMRS)
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

	output.position = mul(float4(output.positionW, 1.0f), gmtxViewProjection);
	output.uv = input.uv;
	output.reflection = reflect(output.positionW - gvCameraPosition, output.normalW);
	output.shadowPosH = mul(float4(output.positionW, 1.0f), gmtxShadowTransform);

	return(output);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

struct VS_SKINNED_STANDARD_SHADOW_INPUT
{
	float3 position : POSITION;
	float2 uv : TEXCOORD;
	uint4 indices : BONEINDEX;
	float4 weights : BONEWEIGHT;
};

VS_STANDARD_SHADOW_OUTPUT VSSkinnedAnimationStandardShadow(VS_SKINNED_STANDARD_SHADOW_INPUT input)
{
	VS_STANDARD_SHADOW_OUTPUT output;

	float3 positionW = float3(0.0f, 0.0f, 0.0f);

	matrix mtxVertexToBoneWorld;

	for (int i = 0; i < MAX_VERTEX_INFLUENCES; i++)
	{
		if (input.weights[i] < 0.00000001)
			continue;

		mtxVertexToBoneWorld = mul(gpmtxBoneOffsets[input.indices[i]], gpmtxBoneTransforms[input.indices[i]]);
		positionW += input.weights[i] * mul(float4(input.position, 1.0f), mtxVertexToBoneWorld).xyz;
	}

	output.position = mul(float4(positionW, 1.0f), gmtxLightViewProjection);
	output.uv = input.uv;

	return(output);
}