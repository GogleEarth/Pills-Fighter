#define GAME_OBJECT_INFO				b0
#define CAMERA_INFO						b1
#define LIGHTS_INFO						b2
#define UI_INFO							b3
#define SPRITE_INFO						b4
#define BONE_OFFSETS					b5
#define BONE_TRANSFOMRS					b6
#define EFFECT_INFO						b7
#define PARTICLE_INFO					b8
#define CURSOR_INFO						b10
#define SCENE_INFO						b11
#define FONT_INFO						b12
#define LIGHT_CAMERA_INFO				b13
#define SCREEN_EFFECT					b14
#define UI_RELOAD_N_RESPAWN_INFO		b15
#define MINIMAP_ENEMY_POS				b16
#define MINIMAP_PLAYER_POS				b17
#define UI_COLOR_INFO					b18
#define UI_3D_INFO						b19
#define FOLLOW_EFFECT_INFO				b20
#define MINIMAP_TEAM_POS				b21
#define CUSTOM_UI_INFO					b22

#define DIFFUSE_TEXTURES				t1 // t1 ~ t3
#define SPECULAR_TEXTURE				t4
#define NORMAL_TEXTURE					t5
#define SKYCUBE_TEXTURE					t6
#define TILE_TEXTURES					t7 // t7 ~ t11
#define INSTANCING_GAME_OBJECTS_INFO	t12
#define ENVIRCUBE_TEXTURE				t13
#define EMISSIVE_TEXTURE				t14
#define SHADOW_MAP						t15
#define NORMAL_MAP						t16

#define WRAP_SAMPLERSTATE				s0
#define CLAMP_SAMPLERSTATE				s1
#define SHADOW_SAMPLERSTATE				s2

#define MATERIAL_ALBEDO_MAP				0x01
#define MATERIAL_SPECULAR_FACTOR_MAP	0x02
#define MATERIAL_NORMAL_MAP				0x04
#define MATERIAL_EMISSION_MAP			0x08

SamplerState gssWrap : register(WRAP_SAMPLERSTATE);
SamplerState gssClamp : register(CLAMP_SAMPLERSTATE);
SamplerComparisonState gscsShadow : register(SHADOW_SAMPLERSTATE);

TextureCube gtxtSkyCubeTexture : register(SKYCUBE_TEXTURE);
Texture2D gtxtTileTexture[5] : register(TILE_TEXTURES); //	t7~t11

Texture2D gtxtTexture[3] : register(DIFFUSE_TEXTURES); //	t1 ~ t3
Texture2D gtxtSpecularTexture : register(SPECULAR_TEXTURE);
Texture2D gtxtNormalTexture : register(NORMAL_TEXTURE);
Texture2D gtxtEmissiveTexture : register(EMISSIVE_TEXTURE);
TextureCube gtxtEnvirCubeTexture : register(ENVIRCUBE_TEXTURE);
Texture2D<float> gtxtShadowMap : register(SHADOW_MAP);
Texture2D<float4> gtxtScreenNormalTexture : register(NORMAL_MAP);

struct MATERIAL
{
	float4		m_cAmbient;
	float4		m_cDiffuse;
	float4		m_cSpecular; //a = power
	float4		m_cEmissive;
	float		m_cReflectionFactor;
};

cbuffer cbGameObjectInfo : register(GAME_OBJECT_INFO)
{
	matrix		gmtxGameObject : packoffset(c0);
	MATERIAL	gMaterial : packoffset(c4);
	uint		gnTexturesMask : packoffset(c8.y);
};

cbuffer cbCameraInfo : register(CAMERA_INFO)
{
	matrix		gmtxViewProjection : packoffset(c0);
	float3		gvCameraPosition : packoffset(c4);
};

cbuffer cbLightCameraInfo : register(LIGHT_CAMERA_INFO)
{
	matrix		gmtxLightViewProjection : packoffset(c0);
	matrix		gmtxShadowTransform : packoffset(c4);
};

float CalcShadowFactor(float4 f4ShadowPos)
{
	f4ShadowPos.xyz /= f4ShadowPos.w;

	float fDepth = f4ShadowPos.z;

	uint nWidth, nHeight, nMips;
	gtxtShadowMap.GetDimensions(0, nWidth, nHeight, nMips);

	float dx = 1.0f / (float)nWidth;

	float percentLit = 0.0f;

	const float2 offsets[9] =
	{
		float2(-dx, -dx), float2(0.0f, -dx), float2(dx, -dx),
		float2(-dx, 0.0f), float2(0.0f, 0.0f), float2(dx, 0.0f),
		float2(-dx, +dx), float2(0.0f, +dx), float2(dx, +dx),
	};

	[unroll] for (int i = 0; i < 9; i++)
	{
		percentLit += gtxtShadowMap.SampleCmpLevelZero(gscsShadow, f4ShadowPos.xy + offsets[i], fDepth).r;
	}

	float fFactor = max(percentLit / 9.0f, 0.5f);

	return fFactor;
}