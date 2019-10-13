#pragma once

#include"Model.h"
class CFontShader;

struct CFontCharacter
{
	int nId, nTexIndex;

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
	UINT	 nTexIndex;
};

struct CB_FONT_INFO
{
	XMFLOAT4	m_xmf4Color;
	XMFLOAT3	m_xmf3Position;
};

class CTextObject
{
public:
	CTextObject(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	virtual ~CTextObject();

	virtual void CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void ReleaseShaderVariables();
	virtual void UpdateVertexBuffer(ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList);

	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList);

	virtual void SetText(const wchar_t *pstrText, CFontVertex *pFontVertex, int nLength);
	virtual void SetPosition(XMFLOAT3 xmf3Position) { m_xmf3Position = xmf3Position; };
	virtual XMFLOAT3 GetPosition() { return m_xmf3Position; };
	virtual void SetPosition(XMFLOAT2 xmf2Position) { m_xmf3Position = XMFLOAT3(xmf2Position.x, xmf2Position.y, 0.0f); };
	virtual void MovePosition(XMFLOAT3 xmf3Position) { m_xmf3Position.x += xmf3Position.x; m_xmf3Position.y += xmf3Position.y; m_xmf3Position.z += xmf3Position.z; };
	virtual void SetColor(XMFLOAT4 xmf4Color) { m_xmf4Color = xmf4Color; };
	virtual void SetAlpha(float fAlpha) { m_xmf4Color.w = fAlpha; };
	float GetEndPointX() { return m_pCharacters[m_nCharacter - 1].xmf2Pos.x + m_pCharacters[m_nCharacter - 1].xmf2Size.x; }

	virtual void Release();
	virtual bool IsUsed() { return m_bUse; }

	virtual void FreeText();
	wchar_t* GetText() { return m_pText; }
	bool IsHide() { return m_bHide; }
	void Hide() { m_bHide = true; }
	void Display() { m_bHide = false; }
	void SetRightAlign(float x) { m_fRightAlignX = x; }

protected:
#define MAX_TEXT_LENGTH 100
	int							m_nCharacter;
	CFontVertex					*m_pCharacters;

	ID3D12Resource				*m_pd3dcbFont;
	CFontVertex					*m_pcbMappedFont;
	D3D12_VERTEX_BUFFER_VIEW	m_d3dFontView;

	XMFLOAT4					m_xmf4Color;
	XMFLOAT3					m_xmf3Position;
	float						m_fRightAlignX;

	ID3D12Resource				*m_pd3dcbFontInfo = NULL;
	CB_FONT_INFO				*m_pcbMappedFontInfo = NULL;

	bool						m_bUse;
	wchar_t						m_pText[MAX_TEXT_LENGTH];


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
	void CreateText(int nLength, CFontVertex* pFontVertices, const wchar_t *pstrText, XMFLOAT2 xmf2Scale, XMFLOAT2 xmf2Padding, int nType);
	CTextObject* SetText(const wchar_t *pstrText, XMFLOAT2 xmf2Position, XMFLOAT2 xmf2Scale, XMFLOAT2 xmf2Padding, XMFLOAT4 xmf4Color, int nType);
	void ChangeText(CTextObject *pTextObject, const wchar_t *pstrText, XMFLOAT2 xmf2Scale, XMFLOAT2 xmf2Padding, int nType);

	XMINT2 Create3DText(int nLength, CFontVertex* pFontVertices, const wchar_t *pstrText, XMFLOAT2 xmf2Scale, XMFLOAT2 xmf2Padding);
	CTextObject* Set3DText(int& nWidth, int& nHeight, const wchar_t *pstrText, XMFLOAT2 xmf2Scale, XMFLOAT2 xmf2Padding, XMFLOAT4 xmf4Color);

	void Render(ID3D12GraphicsCommandList *pd3dCommandList);
	void Render3DFont(ID3D12GraphicsCommandList *pd3dCommandList);

protected:
	char m_pstrName[32];
	int m_nSize;
	float m_fTopPadding;
	float m_fBottomPadding;
	float m_fLeftPadding;
	float m_fRightPadding;

	float m_fLineHeight;
	int m_nTextureWidth;
	int m_nTextureHeight;

	int m_nCharacters;
	CFontCharacter *m_pCharacters;

	int m_nKernings;
	CFontKerning * m_pKernings;

	CTexture *m_pFontTexture;

	std::vector<std::string> m_vTextureInfo;

#define MAX_TEXT_SIZE 200
	std::queue<CTextObject*> m_qpTempTextObjects;
	std::vector<CTextObject*> m_vpTextObjects;
	std::vector<CTextObject*> m_vp3DTextObjects;
};



/*
프레임 워크에서 Font 객체 Initialize 해준 후 ( 파일 로드 ) 사용할 폰트를 Scene에 AddFont 해주면 씬에서 사용 준비 끝.
Scene의 AddText()한 후 리턴한 FontObject를 받아 놓으면 일단 글씨가 렌더링 됨.
다 사용했을 경우 FontObject의 Release함수를 호출해 주면 끝.
호출 안하면 게임 끝날 때까지 없어지지 않고. 게임이 끝나면 알아서 Release함.
다시 말해서 중간에 없어져야되는 글자는 받아놓고 특정 조건에 맞춰서 Release해줘야 됨.
SetText()의 Position은 스크린 좌표 ( -1.0 ~ 1.0 )
*/