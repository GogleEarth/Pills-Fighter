#pragma once

class CVertex
{
public:
	XMFLOAT3 m_xmf3Position;

public:
	CVertex() { m_xmf3Position = XMFLOAT3(0.0f, 0.0f, 0.0f); }
	CVertex(XMFLOAT3 xmf3Position) { m_xmf3Position = xmf3Position; }
	~CVertex() { }

	XMFLOAT3 GetPosition() { return m_xmf3Position; }
};

//////////////////////////////////////////////////////////////////////////////

class CIlluminatedVertex : public CVertex
{
protected:
	XMFLOAT3 m_xmf3Normal;

public:
	CIlluminatedVertex() 
	{
		m_xmf3Position = XMFLOAT3(0.0f, 0.0f, 0.0f); 
		m_xmf3Normal =	XMFLOAT3(0.0f, 0.0f, 0.0f); 
	}

	CIlluminatedVertex(float x, float y, float z, XMFLOAT3 xmf3Normal) 
	{
		m_xmf3Position = XMFLOAT3(x, y, z); 
		m_xmf3Normal = xmf3Normal;
	}

	CIlluminatedVertex(XMFLOAT3 xmf3Position, XMFLOAT3 xmf3Normal) 
	{
		m_xmf3Position = xmf3Position; 
		m_xmf3Normal = xmf3Normal;
	}

	~CIlluminatedVertex() { }
};

//////////////////////////////////////////////////////////////////////////////

class CDiffusedVertex : public CVertex
{
public:
	XMFLOAT4 m_xmf4Diffuse;

public:
	CDiffusedVertex()
	{
		m_xmf3Position = XMFLOAT3(0.0f, 0.0f, 0.0f);
		m_xmf4Diffuse = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	}

	CDiffusedVertex(float x, float y, float z, XMFLOAT4 xmf4Diffuse)
	{
		m_xmf3Position = XMFLOAT3(x, y, z);
		m_xmf4Diffuse = xmf4Diffuse;
	}

	CDiffusedVertex(XMFLOAT3 xmf3Position, XMFLOAT4 xmf4Diffuse)
	{
		m_xmf3Position = xmf3Position;
		m_xmf4Diffuse = xmf4Diffuse;
	}

	~CDiffusedVertex() {}
};

class CDiffused2TexturedVertex : public CDiffusedVertex
{
public:
	XMFLOAT2						m_xmf2TexCoord0;
	XMFLOAT2						m_xmf2TexCoord1;

public:
	CDiffused2TexturedVertex() {
		m_xmf3Position = XMFLOAT3(0.0f, 0.0f, 0.0f); m_xmf4Diffuse = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);	m_xmf2TexCoord0 = m_xmf2TexCoord1 = XMFLOAT2(0.0f, 0.0f);
	}
	CDiffused2TexturedVertex(float x, float y, float z,	XMFLOAT4 xmf4Diffuse, XMFLOAT2 xmf2TexCoord0, XMFLOAT2 xmf2TexCoord1) {
		m_xmf3Position = XMFLOAT3(x, y, z); m_xmf4Diffuse = xmf4Diffuse;
		m_xmf2TexCoord0 = xmf2TexCoord0; m_xmf2TexCoord1 = xmf2TexCoord1;
	}
	CDiffused2TexturedVertex(XMFLOAT3 xmf3Position, XMFLOAT4 xmf4Diffuse = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f),
		XMFLOAT2 xmf2TexCoord0 = XMFLOAT2(0.0f, 0.0f), XMFLOAT2 xmf2TexCoord1 = XMFLOAT2(0.0f, 0.0f)) {
		m_xmf3Position = xmf3Position; m_xmf4Diffuse = xmf4Diffuse;
		m_xmf2TexCoord0 = xmf2TexCoord0; m_xmf2TexCoord1 = xmf2TexCoord1;
	}
	~CDiffused2TexturedVertex() { }
};


class CTexturedVertex : public CVertex
{
public:
	XMFLOAT2	m_xmf2TexCoord;

public:
	CTexturedVertex()
	{
		m_xmf3Position = XMFLOAT3(0.0f, 0.0f, 0.0f);
		m_xmf2TexCoord = XMFLOAT2(0.0f, 0.0f);
	}

	CTexturedVertex(float x, float y, float z, XMFLOAT2 xmf2TexCoord)
	{
		m_xmf3Position = XMFLOAT3(x, y, z);
		m_xmf2TexCoord = xmf2TexCoord;
	}

	CTexturedVertex(XMFLOAT3 xmf3Position, XMFLOAT2 xmf2TexCoord)
	{
		m_xmf3Position = xmf3Position;
		m_xmf2TexCoord = xmf2TexCoord;
	}

	~CTexturedVertex() { }
};

//////////////////////////////////////////////////////////////////////////////

class CIlluminatedTexturedVertex : public CIlluminatedVertex
{
public:
	XMFLOAT2	m_xmf2TexCoord;

public:
	CIlluminatedTexturedVertex()
	{ 
		m_xmf3Position = XMFLOAT3(0.0f, 0.0f, 0.0f);
		m_xmf3Normal = XMFLOAT3(0.0f, 0.0f, 0.0f);
		m_xmf2TexCoord = XMFLOAT2(0.0f, 0.0f); 
	}

	CIlluminatedTexturedVertex(float x, float y, float z, XMFLOAT3 xmf3Normal, XMFLOAT2 xmf2TexCoord)
	{
		m_xmf3Position = XMFLOAT3(x, y, z);
		m_xmf3Normal = xmf3Normal;
		m_xmf2TexCoord = xmf2TexCoord;
	}

	CIlluminatedTexturedVertex(XMFLOAT3 xmf3Position, XMFLOAT3 xmf3Normal, XMFLOAT2 xmf2TexCoord)
	{ 
		m_xmf3Position = xmf3Position;
		m_xmf3Normal = xmf3Normal;
		m_xmf2TexCoord = xmf2TexCoord; 
	}

	~CIlluminatedTexturedVertex() { }
};

//////////////////////////////////////////////////////////////////////////////

class CIlluminatedDiffusedTexturedVertex : public CIlluminatedTexturedVertex
{
public:
	XMFLOAT4 m_xmf4Diffuse;

public:
	CIlluminatedDiffusedTexturedVertex()
	{ 
		m_xmf3Position = XMFLOAT3(0.0f, 0.0f, 0.0f);
		m_xmf3Normal = XMFLOAT3(0.0f, 0.0f, 0.0f);
		m_xmf2TexCoord = XMFLOAT2(0.0f, 0.0f);
		m_xmf4Diffuse = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	}

	CIlluminatedDiffusedTexturedVertex(float x, float y, float z, XMFLOAT3 xmf3Normal, XMFLOAT4 xmf4Diffuse, XMFLOAT2 xmf2TexCoord)
	{ 
		m_xmf3Position = XMFLOAT3(x, y, z);
		m_xmf3Normal = xmf3Normal;
		m_xmf4Diffuse = xmf4Diffuse; 
		m_xmf2TexCoord = xmf2TexCoord;
	}

	CIlluminatedDiffusedTexturedVertex(XMFLOAT3 xmf3Position, XMFLOAT3 xmf3Normal, XMFLOAT4 xmf4Diffuse, XMFLOAT2 xmf2TexCoord)
	{ 
		m_xmf3Position = xmf3Position;
		m_xmf3Normal = xmf3Normal;
		m_xmf4Diffuse = xmf4Diffuse; 
		m_xmf2TexCoord = xmf2TexCoord; 
	}

	~CIlluminatedDiffusedTexturedVertex() { }
};

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//


class CMesh
{
protected:
	UINT							m_nVertices = 0;
	CIlluminatedTexturedVertex		*m_pVertices = NULL;

	D3D12_PRIMITIVE_TOPOLOGY		m_d3dPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	UINT							m_nSlot = 0;
	UINT							m_nStride = 0;
	UINT							m_nOffset = 0;

	ID3D12Resource					*m_pd3dVertexBuffer = NULL;
	ID3D12Resource					*m_pd3dVertexUploadBuffer = NULL;
	D3D12_VERTEX_BUFFER_VIEW		m_d3dVertexBufferView;

	/*인덱스 버퍼(인덱스의 배열)와 인덱스 버퍼를 위한 업로드 버퍼에 대한 인터페이스 포인터이다.
	인덱스 버퍼는 정점 버퍼(배열)에 대한 인덱스를 가진다.*/
	UINT							m_nIndices = 0;
	UINT							*m_pnIndices = NULL;

	UINT							m_nStartIndex = 0;
	int								m_nBaseVertex = 0;

	ID3D12Resource					*m_pd3dIndexBuffer = NULL;
	ID3D12Resource					*m_pd3dIndexUploadBuffer = NULL;
	D3D12_INDEX_BUFFER_VIEW			m_d3dIndexBufferView;

public:
	CMesh() {};
	CMesh(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, const char *pstrFileName);
	virtual ~CMesh();

	void ReleaseUploadBuffers();
	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, UINT nInstances = 1);

	BoundingOrientedBox m_xmOOBB;
	void SetOOBB(XMFLOAT3& xmCenter, XMFLOAT3& xmExtents, XMFLOAT4& xmOrientation) { m_xmOOBB = BoundingOrientedBox(xmCenter, xmExtents, xmOrientation); }
	XMFLOAT3& GetExtents() { return m_xmOOBB.Extents; }
	XMFLOAT3& GetCenter() { return m_xmOOBB.Center; }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CCubeMesh : public CMesh
{
public:
	CCubeMesh(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, XMFLOAT3 xmf3Center, float fWidth = 2.0f, float fHeight = 2.0f, float fDepth = 2.0f);
	virtual ~CCubeMesh();
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CUIVertex
{
public:
	XMFLOAT2						m_xmfPosition;
	XMFLOAT2						m_xmf2Size;

public:
	CUIVertex() { m_xmfPosition = XMFLOAT2(0.0f, 0.0f); m_xmf2Size = XMFLOAT2(0.0f, 0.0f); }
	CUIVertex(XMFLOAT2 xmf3Position, XMFLOAT2 xmf2Size) { m_xmfPosition = xmf3Position; m_xmf2Size = xmf2Size; }
	~CUIVertex() { }
};

class CUIRect
{
protected:
	CUIVertex						*m_pVertices;

	D3D12_PRIMITIVE_TOPOLOGY		m_d3dPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_POINTLIST;
	UINT							m_nSlot = 0;
	UINT							m_nStride = 0;
	UINT							m_nOffset = 0;

	ID3D12Resource					*m_pd3dVertexBuffer = NULL;
	ID3D12Resource					*m_pd3dVertexUploadBuffer = NULL;
	D3D12_VERTEX_BUFFER_VIEW		m_d3dVertexBufferView;

public:
	CUIRect(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, XMFLOAT2 xmf2Center, XMFLOAT2 xmf2Size);
	virtual ~CUIRect();

	void ReleaseUploadBuffers();
	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList);
};



/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
class CHeightMapImage
{
private:
	BYTE						*m_pHeightMapPixels;

	int							m_nWidth;
	int							m_nLength;
	XMFLOAT3					m_xmf3Scale;

public:
	CHeightMapImage(LPCTSTR pFileName, int nWidth, int nLength, XMFLOAT3 xmf3Scale);
	~CHeightMapImage(void);

	float GetHeight(float x, float z, bool bReverseQuad = false);
	XMFLOAT3 GetHeightMapNormal(int x, int z);
	XMFLOAT3 GetScale() { return(m_xmf3Scale); }

	BYTE *GetHeightMapPixels() { return(m_pHeightMapPixels); }
	int GetHeightMapWidth() { return(m_nWidth); }
	int GetHeightMapLength() { return(m_nLength); }
};

class CHeightMapGridMesh : public CMesh
{
protected:
	int							m_nWidth;
	int							m_nLength;
	XMFLOAT3					m_xmf3Scale;

public:
	CHeightMapGridMesh(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, int xStart, int zStart, int nWidth, int nLength, XMFLOAT3 xmf3Scale = XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT4 xmf4Color = XMFLOAT4(1.0f, 1.0f, 0.0f, 0.0f), void *pContext = NULL);
	virtual ~CHeightMapGridMesh();

	XMFLOAT3 GetScale() { return(m_xmf3Scale); }
	int GetWidth() { return(m_nWidth); }
	int GetLength() { return(m_nLength); }

	virtual float OnGetHeight(int x, int z, void *pContext);
	virtual XMFLOAT4 OnGetColor(int x, int z, void *pContext);
};
