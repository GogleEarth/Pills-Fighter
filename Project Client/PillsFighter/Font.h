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
������ ��ũ���� Font ��ü Initialize ���� �� ( ���� �ε� ) ����� ��Ʈ�� Scene�� AddFont ���ָ� ������ ��� �غ� ��.
Scene�� AddText()�� �� ������ FontObject�� �޾� ������ �ϴ� �۾��� ������ ��.
�� ������� ��� FontObject�� Release�Լ��� ȣ���� �ָ� ��.
SetText()�� Position�� ��ũ�� ��ǥ ( -1.0 ~ 1.0 )
*/