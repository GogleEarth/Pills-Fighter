#pragma once

class CTexture;
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

	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList);

	virtual void SetText(CFontVertex *pFontVertex, int nLength);

	virtual void Release();
	virtual bool IsUsed() { return m_bUse; }

private:
	virtual void FreeText();

protected:
#define MAX_TEXT_LENGTH 100
	int							m_nCharacter = 0;
	CFontVertex					*m_pCharacters = NULL;

	ID3D12Resource				*m_pd3dcbFont = NULL;
	CFontVertex					*m_pcbMappedFont = NULL;
	D3D12_VERTEX_BUFFER_VIEW	m_d3dFontView;

	bool						m_bUse = false;
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

	CFontCharacter* GetChar(char c);
	const char* GetName() { return m_pstrName; }
	float GetKerning(char cFirst, char cSecond);

	void SetSrv(ID3D12Device *pd3dDevice);

	CTextObject* SetText(const char *pstrText, XMFLOAT2 xmf2Position, XMFLOAT2 xmf2Scale, XMFLOAT2 xmf2Padding, XMFLOAT4 xmf4Color);
	void Render(ID3D12GraphicsCommandList *pd3dCommandList);

protected:
	char m_pstrName[32] = { 0 };
	int m_nSize = 0;
	float m_fTopPadding = 0;
	float m_fBottomPadding = 0;
	float m_fLeftPadding = 0;
	float m_fRightPadding = 0;

	float m_fLineHeight = 0;
	float m_fBaseHeight = 0;
	int m_nTextureWidth = 0;
	int m_nTextureHeight = 0;

	char m_pstrImageFile[64] = { 0 };

	int m_nCharacters = 0;
	CFontCharacter *m_pCharacters = NULL;

	int m_nKernings = 0;
	CFontKerning * m_pKernings = NULL;

	CTexture *m_pFontTexture = NULL;

#define MAX_TEXT_SIZE 200
	std::queue<CTextObject*> m_qpTempTextObjects;
	std::vector<CTextObject*> m_vpTextObjects;
};



/*
프레임 워크에서 Font 객체 Initialize 해준 후 ( 파일 로드 ) 사용할 폰트를 Scene에 AddFont 해주면 씬에서 사용 준비 끝.
Scene의 AddText()한 후 리턴한 FontObject를 받아 놓으면 일단 글씨가 렌더링 됨.
다 사용했을 경우 FontObject의 Release함수를 호출해 주면 끝.
SetText()의 Position은 스크린 좌표 ( -1.0 ~ 1.0 )
*/