#pragma once

struct CFontCharacter
{
	int nId;

	float u, v;
	float tw, th;
	float w, h;
	float xOffset, yOffset, xAdvance;
};

struct CFontKerning
{
	int nFirstId, nSecondId;
	float fAmount;
};

struct CFont
{

};

class CFontVertex
{
public:
	CFontVertex(XMFLOAT4 xmf4Position, XMFLOAT4 xmf4Color, XMFLOAT4 xmf4UV) : m_xmf4Position(xmf4Position), m_xmf4Color(xmf4Color), m_xmf4UV(xmf4UV) {}
	virtual ~CFontVertex() {};

	XMFLOAT4 m_xmf4Position;
	XMFLOAT4 m_xmf4Color;
	XMFLOAT4 m_xmf4UV;
};

