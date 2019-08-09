#ifndef DEFINE_DEFINE
#define DEFINE_DEFINE
#include "Define.hlsl"
#endif

struct VS_UI_INPUT
{
	float2 center : POSITION;
	float2 size : SIZE;
};

VS_UI_INPUT VSUI(VS_UI_INPUT input)
{
	return(input);
}

struct GS_UI_OUT
{
	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD;
};

[maxvertexcount(4)]
void GSUI(point VS_UI_INPUT input[1], uint primID : SV_PrimitiveID, inout TriangleStream<GS_UI_OUT> outStream)
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

cbuffer cbUIInfo : register(UI_INFO)
{
	int		giMaxValue;// : packoffset(c0.x);
	int		giValue; //: packoffset(c0.y);
};

[maxvertexcount(4)]
void GSUIBar(point VS_UI_INPUT input[1], uint primID : SV_PrimitiveID, inout TriangleStream<GS_UI_OUT> outStream)
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

float4 PSUI(GS_UI_OUT input) : SV_TARGET
{
	// 임시 텍스처 배열 인덱스는 0
	return gtxtTexture[0].Sample(gssClamp, input.uv);
}

float4 PSUIScreen(GS_UI_OUT input) : SV_TARGET
{
	// 임시 텍스처 배열 인덱스는 0
	return float4(0.0f, 0.0f, 0.0f, 0.5f);
}

float4 PSUIBullet(GS_UI_OUT input) : SV_TARGET
{
	float2 vNewUV = input.uv;
	float fStride = 1.0f / float(giMaxValue);
	vNewUV.y = 1 - fmod(vNewUV.y, fStride) * giMaxValue;

	float4 cColor = 1.0f;
	
	float r = float(giValue) / float(giMaxValue);
	if(1.0f - input.uv.y < r) cColor = gtxtTexture[0].Sample(gssClamp, vNewUV);
	else cColor = gtxtTexture[1].Sample(gssClamp, vNewUV);

	return(cColor);
}

cbuffer cbUIColorInfo : register(UI_COLOR_INFO)
{
	float4 gvUIColor;
}

float4 PSUIColored(GS_UI_OUT input) : SV_TARGET
{
	float4 cColor = gtxtTexture[0].Sample(gssClamp, input.uv);

	return cColor * gvUIColor;
}

[maxvertexcount(4)]
void GSUITeamHP(point VS_UI_INPUT input[1], uint primID : SV_PrimitiveID, inout TriangleStream<GS_UI_OUT> outStream)
{
	float xPos = float(giValue) / float(giMaxValue);
	float2 vValueByRight = float2(xPos, 0.0f);
	float2 vRight = float2(1.0f, 0.0f);
	float2 vUp = float2(0.0f, 1.0f);
	float fWidth = input[0].size.x * 2.0f;
	float fHalfH = input[0].size.y;

	input[0].center -= input[0].size.x * vRight;

	float4 fVertices[4];
	fVertices[0] = float4(input[0].center - fHalfH * vUp, 0.0f, 1.0f);
	fVertices[1] = float4(input[0].center, 0.0f, 1.0f);
	fVertices[2] = float4(input[0].center + fWidth * vValueByRight - fHalfH * vUp, 0.0f, 1.0f);
	fVertices[3] = float4(input[0].center + fWidth * vValueByRight, 0.0f, 1.0f);

	float2 fUVs[4];
	fUVs[0] = float2(0.0f, 1.0f);
	fUVs[1] = float2(0.0f, 0.0f);
	fUVs[2] = float2(xPos, 1.0f);
	fUVs[3] = float2(xPos, 0.0f);

	GS_UI_OUT output;

	for (int i = 0; i < 4; i++)
	{
		output.pos = fVertices[i];
		output.uv = fUVs[i];

		outStream.Append(output);
	}
}

float4 PSUITeamHP(GS_UI_OUT input) : SV_TARGET
{
	return gtxtTexture[0].Sample(gssClamp, input.uv);
}

cbuffer cbReloadNRespawnInfo : register(UI_RELOAD_N_RESPAWN_INFO)
{
	float4 gf4FillColor;
	float4 gf4TextureColor;
	float gfNormalizedTime;
}

float4 PSUIReload(GS_UI_OUT input) : SV_TARGET
{
	float4 cColor = gf4FillColor;

	if (input.uv.y < gfNormalizedTime)
	{
		cColor = gtxtTexture[0].Sample(gssClamp, input.uv) * gf4TextureColor;
	}

	return cColor;
}

float4 PSUIRespawn(GS_UI_OUT input) : SV_TARGET
{
	float4 cColor = gf4FillColor;

	if (input.uv.x > gfNormalizedTime)
	{
		cColor = gtxtTexture[0].Sample(gssClamp, input.uv);
	}

	return cColor;
}

////////////////////////////////////////////////////////////////
//

struct CUSTOMUI
{
	float2 m_f2Scale;
};

ConstantBuffer<CUSTOMUI> gCustomUI : register(CUSTOM_UI_INFO);

[maxvertexcount(4)]
void GSUICustom(point VS_UI_INPUT input[1], uint primID : SV_PrimitiveID, inout TriangleStream<GS_UI_OUT> outStream)
{
	float2 vUp = float2(0.0f, 1.0f);
	float2 vRight = float2(1.0f, 0.0f);
	float fHalfW = input[0].size.x * gCustomUI.m_f2Scale.x;
	float fHalfH = input[0].size.y * gCustomUI.m_f2Scale.y;

	float4 fVertices[4];
	fVertices[0] = float4(input[0].center - fHalfW * vRight - fHalfH * vUp, 0.0f, 1.0f);
	fVertices[1] = float4(input[0].center - fHalfW * vRight + fHalfH * vUp, 0.0f, 1.0f);
	fVertices[2] = float4(input[0].center + fHalfW * vRight - fHalfH * vUp, 0.0f, 1.0f);
	fVertices[3] = float4(input[0].center + fHalfW * vRight + fHalfH * vUp, 0.0f, 1.0f);

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

////////////////////////////////////////////

cbuffer cbFontInfo : register(FONT_INFO)
{
	float4 gf4FontColor;
	float3 gf3FontPosition;
}

struct VS_FONT_IN
{
	float2 pos : POSITION;
	float2 size : SIZE;
	float2 uvPos : UVPOSITION;
	float2 uvSize : UVSIZE;
	float4 color : COLOR;
	uint   texindex : TEXINDEX;
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
	uint   texindex : TEXINDEX;
};

[maxvertexcount(4)]
void GSFont(point VS_FONT_IN input[1], inout TriangleStream<GS_FONT_OUT> outStream)
{
	float fWidth = input[0].size.x;
	float fHeight = input[0].size.y;
	float2 pos = input[0].pos + gf3FontPosition.xy;

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
		output.texindex = input[0].texindex;

		outStream.Append(output);
	}
}

float4 PSFont(GS_FONT_OUT input) : SV_TARGET
{
	float4 fColor = gtxtTexture[NonUniformResourceIndex(input.texindex)].Sample(gssWrap, input.uv);

	return fColor * input.color * gf4FontColor;
}

////////////////////////////////////////////////////////////////
//

cbuffer cb3DUIInfo : register(UI_3D_INFO)
{
	float4 gf43DUIColor;
	float3 gf33DUIWorldPosition;
}

[maxvertexcount(4)]
void GS3DUI(point VS_UI_INPUT input[1], uint primID : SV_PrimitiveID, inout TriangleStream<GS_UI_OUT> outStream)
{
	float3 f3Position = gf33DUIWorldPosition + float3(input[0].center, 0.0f);

	float3 vUp = float3(0.0f, 1.0f, 0.0f);
	float3 vLook = normalize(f3Position - gvCameraPosition);
	float3 vRight = normalize(cross(vUp, vLook));

	float fHalfW = input[0].size.x;
	float fHalfH = input[0].size.y;

	float4 fVertices[4];
	fVertices[0] = float4(f3Position - fHalfW * vRight + fHalfH * vUp, 1.0f);
	fVertices[1] = float4(f3Position + fHalfW * vRight + fHalfH * vUp, 1.0f);
	fVertices[2] = float4(f3Position - fHalfW * vRight - fHalfH * vUp, 1.0f);
	fVertices[3] = float4(f3Position + fHalfW * vRight - fHalfH * vUp, 1.0f);

	float2 fUVs[4];
	fUVs[0] = float2(0.0f, 0.0f);
	fUVs[1] = float2(1.0f, 0.0f);
	fUVs[2] = float2(0.0f, 1.0f);
	fUVs[3] = float2(1.0f, 1.0f);

	GS_UI_OUT output;

	for (int i = 0; i < 4; i++)
	{
		output.pos = mul(fVertices[i], gmtxViewProjection);
		output.uv = fUVs[i];

		outStream.Append(output);
	}
}

float4 PS3DUI(GS_UI_OUT input) : SV_TARGET
{
	float4 cColor = gtxtTexture[0].Sample(gssClamp, input.uv);

	return cColor * gf43DUIColor;
}

////////////////////////////////////////////////////////////////
//

cbuffer cbCursorInfo : register(CURSOR_INFO)
{
	float2 gvCursorPos;
}

[maxvertexcount(4)]
void GSCursor(point VS_UI_INPUT input[1], inout TriangleStream<GS_UI_OUT> outStream)
{
	float2 vUp = float2(0.0f, 1.0f);
	float2 vRight = float2(1.0f, 0.0f);
	float fWidth = input[0].size.x * 2.0f;
	float fHeight = input[0].size.y * 2.0f;

	float4 fVertices[4];
	fVertices[0] = float4(gvCursorPos + input[0].center, 0.0f, 1.0f);
	fVertices[1] = float4(gvCursorPos + input[0].center - vUp * fHeight, 0.0f, 1.0f);
	fVertices[2] = float4(gvCursorPos + input[0].center + vRight * fWidth, 0.0f, 1.0f);
	fVertices[3] = float4(gvCursorPos + input[0].center + vRight * fWidth - vUp * fHeight, 0.0f, 1.0f);

	float2 fUVs[4];
	fUVs[0] = float2(0.0f, 0.0f);
	fUVs[1] = float2(0.0f, 1.0f);
	fUVs[2] = float2(1.0f, 0.0f);
	fUVs[3] = float2(1.0f, 1.0f);

	GS_UI_OUT output;

	for (int i = 0; i < 4; i++)
	{
		output.pos = fVertices[i];
		output.uv = fUVs[i];

		outStream.Append(output);
	}
}



////////////////////////////////////////////////////////////////

struct VS_UI_MINIMAPROBOT_INPUT
{
	float2 center : POSITION;
	float2 size : SIZE;
	uint index : INDEX;
};

struct VS_UI_MINIMAPROBOT_OUTPUT
{
	float2 center : POSITION;
	float2 size : SIZE;
	uint index : INDEX;
};

VS_UI_MINIMAPROBOT_OUTPUT VSMinimapRobot(VS_UI_MINIMAPROBOT_INPUT input)
{
	return(input);
}

cbuffer cbMinimapEnemyPos : register(MINIMAP_ENEMY_POS)
{
	float2 gvMinimapEnemyPos;
	bool gvMinimapEnemyCut;
}
cbuffer cbMinimapTeamPos : register(MINIMAP_TEAM_POS)
{
	float2 gvMinimapTeamPos;
	bool gvMinimapTeamCut;
}

cbuffer cbMinimapPlayerPos : register(MINIMAP_PLAYER_POS)
{
	matrix gmtxPlayerView;
	float2 gvMinimapPlayerPosition;
	float2 gvMinimapPlayerLook;
	float2 gvMinimapPlayerRight;
}

struct GS_UI_MINIMAPROBOT_OUT
{
	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD;
	bool outOfRange : OUTOFRANGE;
};

[maxvertexcount(4)]
void GSMinimapEnemy(point VS_UI_MINIMAPROBOT_OUTPUT input[1], inout TriangleStream<GS_UI_MINIMAPROBOT_OUT> outStream)
{
	float2 vUp = float2(0.0f, 1.0f);
	float2 vRight = float2(1.0f, 0.0f);
	float fHalfW = input[0].size.x *0.9;
	float fHalfH = input[0].size.y *1.6;

	float2 fVertices[4];
	fVertices[0] = float2(input[0].center - fHalfW * vRight - fHalfH * vUp);
	fVertices[1] = float2(input[0].center - fHalfW * vRight + fHalfH * vUp);
	fVertices[2] = float2(input[0].center + fHalfW * vRight - fHalfH * vUp);
	fVertices[3] = float2(input[0].center + fHalfW * vRight + fHalfH * vUp);
	float2 fUVs[4];
	fUVs[0] = float2(0.0f, 1.0f);
	fUVs[1] = float2(0.0f, 0.0f);
	fUVs[2] = float2(1.0f, 1.0f);
	fUVs[3] = float2(1.0f, 0.0f);

	GS_UI_MINIMAPROBOT_OUT output;

	float2 world = mul(float4(gvMinimapEnemyPos.x, 0.0f, gvMinimapEnemyPos.y, 1.0f), gmtxPlayerView).xz;
	world.x *= 0.0003f;
	world.y *= 0.00053f;

	float2 enemyPos;
	enemyPos.x = (gvMinimapEnemyPos.x - gvMinimapPlayerPosition.x);
	enemyPos.y = (gvMinimapEnemyPos.y - gvMinimapPlayerPosition.y);
	enemyPos = mul(float4(enemyPos.x, 0.0f, enemyPos.y, 1.0f), gmtxPlayerView).xz;
	if (gvMinimapEnemyCut == true) output.outOfRange = true;

	for (int i = 0; i < 4; i++)
	{
		output.pos = float4(fVertices[i] + world, 0.0f, 1.0f);
		output.uv = fUVs[i];

		outStream.Append(output);
	}
}
[maxvertexcount(4)]
void GSMinimapTeam(point VS_UI_MINIMAPROBOT_OUTPUT input[1], inout TriangleStream<GS_UI_MINIMAPROBOT_OUT> outStream)
{
	float2 vUp = float2(0.0f, 1.0f);
	float2 vRight = float2(1.0f, 0.0f);
	float fHalfW = input[0].size.x *0.9;
	float fHalfH = input[0].size.y *1.6;

	float2 fVertices[4];
	fVertices[0] = float2(input[0].center - fHalfW * vRight - fHalfH * vUp);
	fVertices[1] = float2(input[0].center - fHalfW * vRight + fHalfH * vUp);
	fVertices[2] = float2(input[0].center + fHalfW * vRight - fHalfH * vUp);
	fVertices[3] = float2(input[0].center + fHalfW * vRight + fHalfH * vUp);
	float2 fUVs[4];
	fUVs[0] = float2(0.0f, 1.0f);
	fUVs[1] = float2(0.0f, 0.0f);
	fUVs[2] = float2(1.0f, 1.0f);
	fUVs[3] = float2(1.0f, 0.0f);

	GS_UI_MINIMAPROBOT_OUT output;

	float2 world = mul(float4(gvMinimapTeamPos.x, 0.0f, gvMinimapTeamPos.y, 1.0f), gmtxPlayerView).xz;
	world.x *= 0.0003f;
	world.y *= 0.00053f;

	float2 teamPos;
	teamPos.x = (gvMinimapTeamPos.x - gvMinimapPlayerPosition.x);
	teamPos.y = (gvMinimapTeamPos.y - gvMinimapPlayerPosition.y);
	teamPos = mul(float4(teamPos.x, 0.0f, teamPos.y, 1.0f), gmtxPlayerView).xz;
	if (gvMinimapTeamCut == true) output.outOfRange = true;

	for (int i = 0; i < 4; i++)
	{
		output.pos = float4(fVertices[i] + world, 0.0f, 1.0f);
		output.uv = fUVs[i];

		outStream.Append(output);
	}
}

float4 PSMinimapEnemy(GS_UI_MINIMAPROBOT_OUT input) : SV_TARGET
{
	if (input.outOfRange) discard;

	float4 cColor;
	cColor = gtxtTexture[0].Sample(gssWrap, input.uv);

	if (cColor.r >= 0.9 && cColor.g >= 0.9 && cColor.b >= 0.9) discard;

	return(cColor);
}
float4 PSMinimapTeam(GS_UI_MINIMAPROBOT_OUT input) : SV_TARGET
{
	if (input.outOfRange) discard;

	float4 cColor;
	cColor = gtxtTexture[1].Sample(gssWrap, input.uv);

	if (cColor.r >= 0.9 && cColor.g >= 0.9 && cColor.b >= 0.9) discard;

	return(cColor);
}

/////////////////////////////////////////////////////////////////////////////////////////

[maxvertexcount(4)]
void GSMinimapSight(point VS_UI_INPUT input[1], uint primID : SV_PrimitiveID, inout TriangleStream<GS_UI_OUT> outStream)
{
	float2 vUp = float2(0.0f, 1.0f);
	float2 vRight = float2(1.0f, 0.0f);
	float fHalfW = input[0].size.x;
	float fHalfH = input[0].size.y;

	float2 fVertices[4];
	fVertices[0] = float2(-fHalfW * vRight - fHalfH * vUp);
	fVertices[1] = float2(-fHalfW * vRight + fHalfH * vUp);
	fVertices[2] = float2(+fHalfW * vRight - fHalfH * vUp);
	fVertices[3] = float2(+fHalfW * vRight + fHalfH * vUp);

	float2 fUVs[4];
	fUVs[0] = float2(0.0f, 1.0f);
	fUVs[1] = float2(0.0f, 0.0f);
	fUVs[2] = float2(1.0f, 1.0f);
	fUVs[3] = float2(1.0f, 0.0f);

	GS_UI_OUT output;

	for (int i = 0; i < 4; i++)
	{
		output.pos = float4(fVertices[i] + input[0].center, 0.0f, 1.0f);
		output.uv = fUVs[i];

		outStream.Append(output);
	}
}

float4 PSMinimapSight(GS_UI_OUT input) : SV_TARGET
{
	float4 cColor = 0.0f;

	float fDistance = distance(input.uv, 0.5f);

	if (fDistance >= 0.49f) // 원 밖 컷
		discard;
	else
		cColor = gtxtTexture[0].Sample(gssWrap, input.uv);

	return(cColor);
}

float4 PSMinimapTerrain(GS_UI_OUT input) : SV_TARGET
{
	float4 cColor = 0.0f;

	float fDistance = distance(input.uv, 0.5f);

	if (fDistance < 0.015f) // 플레이어 점
		cColor = float4(1, 0, 0, 1);
	else if (fDistance >= 0.49f && fDistance < 0.5f) // 바깥선
		cColor = float4(0.5, 0.2, 0.05, 1);
	else if (fDistance >= 0.49f) // 원 밖 컷
		discard;
	else
	{
		cColor = gtxtTexture[0].Sample(gssWrap, input.uv);
		cColor.a = 0.9;
	}

	return(cColor);
}
