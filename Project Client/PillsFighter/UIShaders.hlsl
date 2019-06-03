#ifndef DEFINE_DEFINE
#define DEFINE_DEFINE
#include "Define.hlsl"
#endif

cbuffer cbUIInfo : register(UI_INFO)
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

VS_UI_OUTPUT VSUi(VS_UI_INPUT input)
{
	return(input);
}

struct GS_UI_OUT
{
	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD;
};

[maxvertexcount(4)]
void GSUi(point VS_UI_OUTPUT input[1], uint primID : SV_PrimitiveID, inout TriangleStream<GS_UI_OUT> outStream)
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
void GSUiBar(point VS_UI_OUTPUT input[1], uint primID : SV_PrimitiveID, inout TriangleStream<GS_UI_OUT> outStream)
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

float4 PSUi(GS_UI_OUT input) : SV_TARGET
{
	// 임시 텍스처 배열 인덱스는 0
	return gtxtTexture[0].Sample(gssWrap, input.uv);
}

float4 PSUiBullet(GS_UI_OUT input) : SV_TARGET
{
	float2 vNewUV = input.uv;
	float fStride = 1.0f / float(giValue);
	vNewUV.y = 1 - fmod(vNewUV.y, fStride) * giValue;

	float4 cColor = gtxtTexture[0].Sample(gssWrap, vNewUV);

	return(cColor);
}

float4 PSUiMinimap(GS_UI_OUT input) : SV_TARGET
{
	float4 cColor = gtxtTexture[0].Sample(gssWrap, input.uv);

	float2 centerpos = float2(0.5f, 0.5f);
	cColor = (distance(input.uv, centerpos) < 0.015) ? float4(1, 0, 0, 1) : cColor;
	cColor = (distance(input.uv, centerpos) < 0.49) ? cColor : 0;
	cColor = (distance(input.uv, centerpos) >= 0.49) ? float4(0.5, 0.2, 0.05, 1) : cColor;
	cColor = (distance(input.uv, centerpos) < 0.5) ? cColor : 0;

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

	return fColor * input.color;
}

////////////////////////////////////////////////////////////////

cbuffer cbCursorInfo : register(CURSOR_INFO)
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
