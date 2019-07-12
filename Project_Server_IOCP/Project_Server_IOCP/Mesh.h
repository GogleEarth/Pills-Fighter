#pragma once

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

	int								m_nSubMeshes = 0;
	int								*m_pnSubSetIndices = NULL;
	UINT							**m_ppnSubSetIndices = NULL;

	char							m_pstrName[64] = { 0 };
	int								m_nType = 0x00;

public:
	CMesh();
	virtual ~CMesh();

	BoundingBox m_xmAABB;
	void SetAABB(XMFLOAT3& xmCenter, XMFLOAT3& xmExtents) { m_xmAABB = BoundingBox(xmCenter, xmExtents); }
	void SetAABB(BoundingBox xmAABB) { m_xmAABB = xmAABB; }
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

	void LoadMeshFromFile(FILE *pfile);
};
