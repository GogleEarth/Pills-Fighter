#pragma once
#include"fbxsdk.h"

class CMesh
{
protected:
	UINT							m_nVertices = 0;

	XMFLOAT3						*m_pxmf3Positions = NULL;
	XMFLOAT4						*m_pxmf4Colors = NULL;
	XMFLOAT3						*m_pxmf3Normals = NULL;
	XMFLOAT3						*m_pxmf3Tangents = NULL;
	XMFLOAT3						*m_pxmf3Binormals = NULL;
	XMFLOAT2						*m_pxmf2TextureCoords0 = NULL;
	XMFLOAT2						*m_pxmf2TextureCoords1 = NULL;
	int								*m_pnMaterialIndices = NULL;

public:
	CMesh();
	virtual ~CMesh();

	BoundingBox m_xmAABB;
	void SetAABB(XMFLOAT3& xmCenter, XMFLOAT3& xmExtents, XMFLOAT4& xmOrientation) { m_xmAABB = BoundingBox(xmCenter, xmExtents); }
	XMFLOAT3& GetExtents() { return m_xmAABB.Extents; }
	XMFLOAT3& GetCenter() { return m_xmAABB.Center; }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CCubeMesh : public CMesh
{
public:
	CCubeMesh(XMFLOAT3 xmf3Center, XMFLOAT3 xmf3Extents);
	virtual ~CCubeMesh();
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CStandardMesh : public CMesh
{
public:
	CStandardMesh();
	virtual ~CStandardMesh();

	void LoadMeshFromFBX(FbxMesh *pfbxMesh);
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
	CHeightMapGridMesh(int xStart, int zStart, int nWidth, int nLength, XMFLOAT3 xmf3Scale = XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT4 xmf4Color = XMFLOAT4(1.0f, 1.0f, 0.0f, 0.0f), void *pContext0 = NULL, void *pContext1 = NULL);
	virtual ~CHeightMapGridMesh();

	XMFLOAT3 GetScale() { return(m_xmf3Scale); }
	int GetWidth() { return(m_nWidth); }
	int GetLength() { return(m_nLength); }

	virtual float OnGetHeight(int x, int z, void *pContext);
};