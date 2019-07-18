#pragma once

#include"Model.h"
class CFontShader;

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

struct CFontVertex
{
	XMFLOAT2 xmf2Pos;
	XMFLOAT2 xmf2Size;
	XMFLOAT2 xmf2UVPos;
	XMFLOAT2 xmf2UVSize;
	XMFLOAT4 xmf4Color;
};

class CTextObject
{
public:
	CTextObject(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	virtual ~CTextObject();

	virtual void UpdateVertexBuffer(ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList);

	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList);

	virtual void SetText(const wchar_t *pstrText, CFontVertex *pFontVertex, int nLength);
	virtual void SetPosition(XMFLOAT2 xmf2Position) { m_xmf2Position = xmf2Position; };
	virtual void MovePosition(XMFLOAT2 xmf2Position) { m_xmf2Position.x += xmf2Position.x; m_xmf2Position.y += xmf2Position.y; };
	virtual void SetColor(XMFLOAT4 xmf4Color) { m_xmf4Color = xmf4Color; };

	virtual void Release();
	virtual bool IsUsed() { return m_bUse; }

	virtual void FreeText();
	wchar_t* GetText() { return m_pText; }
	bool IsHide() { return m_bHide; }
	void Hide() { m_bHide = true; }
	void Display() { m_bHide = false; }

protected:
#define MAX_TEXT_LENGTH 100
	int							m_nCharacter;
	CFontVertex					*m_pCharacters;

	ID3D12Resource				*m_pd3dcbFont;
	CFontVertex					*m_pcbMappedFont;
	D3D12_VERTEX_BUFFER_VIEW	m_d3dFontView;

	bool						m_bUse;
	wchar_t						m_pText[MAX_TEXT_LENGTH];

	XMFLOAT2					m_xmf2Position;
	XMFLOAT4					m_xmf4Color;

	bool						m_bHide = false;
};

class CFont
{
public:
	CFont();
	~CFont();

	void LoadDataFromFile(const char *pstrFileName);

	void Initialize(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, const char *pstrFile);
	void CheckUsingTexts();
	void Destroy();
	void ReleaseUploadBuffers();
	void OnPrepareRender(ID3D12GraphicsCommandList *pd3dCommandList) { m_pFontTexture->UpdateShaderVariables(pd3dCommandList); }
	void ClearTexts();

	CFontCharacter* GetChar(wchar_t c);
	const char* GetName() { return m_pstrName; }
	float GetKerning(wchar_t cFirst, wchar_t cSecond);

	void SetSrv(ID3D12Device *pd3dDevice);

#define LEFT_ALIGN 0
#define RIGHT_ALIGN 1
	void CreateText(int nLength, CFontVertex* pFontVertices, const wchar_t *pstrText, XMFLOAT2 xmf2Position, XMFLOAT2 xmf2Scale, XMFLOAT2 xmf2Padding, XMFLOAT4 xmf4Color, int nType);
	CTextObject* SetText(const wchar_t *pstrText, XMFLOAT2 xmf2Position, XMFLOAT2 xmf2Scale, XMFLOAT2 xmf2Padding, XMFLOAT4 xmf4Color, int nType);
	void ChangeText(CTextObject *pTextObject, const wchar_t *pstrText, XMFLOAT2 xmf2Position, XMFLOAT2 xmf2Scale, XMFLOAT2 xmf2Padding, XMFLOAT4 xmf4Color, int nType);
	void Render(ID3D12GraphicsCommandList *pd3dCommandList);

protected:
	char m_pstrName[32];
	int m_nSize;
	float m_fTopPadding;
	float m_fBottomPadding;
	float m_fLeftPadding;
	float m_fRightPadding;

	float m_fLineHeight;
	float m_fBaseHeight;
	int m_nTextureWidth;
	int m_nTextureHeight;

	char m_pstrImageFile[64];

	int m_nCharacters;
	CFontCharacter *m_pCharacters;

	int m_nKernings;
	CFontKerning * m_pKernings;

	CTexture *m_pFontTexture;

#define MAX_TEXT_SIZE 200
	std::queue<CTextObject*> m_qpTempTextObjects;
	std::vector<CTextObject*> m_vpTextObjects;
};



/*
프레임 워크에서 Font 객체 Initialize 해준 후 ( 파일 로드 ) 사용할 폰트를 Scene에 AddFont 해주면 씬에서 사용 준비 끝.
Scene의 AddText()한 후 리턴한 FontObject를 받아 놓으면 일단 글씨가 렌더링 됨.
다 사용했을 경우 FontObject의 Release함수를 호출해 주면 끝.
호출 안하면 게임 끝날 때까지 없어지지 않고. 게임이 끝나면 알아서 Release함.
다시 말해서 중간에 없어져야되는 글자는 받아놓고 특정 조건에 맞춰서 Release해줘야 됨.
SetText()의 Position은 스크린 좌표 ( -1.0 ~ 1.0 )
*/