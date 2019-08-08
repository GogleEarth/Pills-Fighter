#pragma once

#define TYPE_STANDARD_MESH	0x01
#define	TYPE_SKINNED_MESH	0x02

class CModel;

class CMesh
{
protected:
	D3D12_PRIMITIVE_TOPOLOGY		m_d3dPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	UINT							m_nSlot = 0;
	UINT							m_nOffset = 0;

protected:
	UINT							m_nVertices = 0;

	XMFLOAT3						*m_pxmf3Positions = NULL;
	XMFLOAT4						*m_pxmf4Colors = NULL;
	XMFLOAT3						*m_pxmf3Normals = NULL;
	XMFLOAT3						*m_pxmf3Tangents = NULL;
	XMFLOAT3						*m_pxmf3Binormals = NULL;
	XMFLOAT2						*m_pxmf2TextureCoords0 = NULL;
	XMFLOAT2						*m_pxmf2TextureCoords1 = NULL;

	ID3D12Resource					*m_pd3dPositionBuffer = NULL;
	ID3D12Resource					*m_pd3dPositionUploadBuffer = NULL;
	D3D12_VERTEX_BUFFER_VIEW		m_d3dPositionBufferView;

	ID3D12Resource					*m_pd3dColorBuffer = NULL;
	ID3D12Resource					*m_pd3dColorUploadBuffer = NULL;
	D3D12_VERTEX_BUFFER_VIEW		m_d3dColorBufferView;

	ID3D12Resource					*m_pd3dNormalBuffer = NULL;
	ID3D12Resource					*m_pd3dNormalUploadBuffer = NULL;
	D3D12_VERTEX_BUFFER_VIEW		m_d3dNormalBufferView;

	ID3D12Resource					*m_pd3dTangentBuffer = NULL;
	ID3D12Resource					*m_pd3dTangentUploadBuffer = NULL;
	D3D12_VERTEX_BUFFER_VIEW		m_d3dTangentBufferView;

	ID3D12Resource					*m_pd3dBinormalBuffer = NULL;
	ID3D12Resource					*m_pd3dBinormalUploadBuffer = NULL;
	D3D12_VERTEX_BUFFER_VIEW		m_d3dBinormalBufferView;

	ID3D12Resource					*m_pd3dTextureCoord0Buffer = NULL;
	ID3D12Resource					*m_pd3dTextureCoord0UploadBuffer = NULL;
	D3D12_VERTEX_BUFFER_VIEW		m_d3dTextureCoord0BufferView;

	ID3D12Resource					*m_pd3dTextureCoord1Buffer = NULL;
	ID3D12Resource					*m_pd3dTextureCoord1UploadBuffer = NULL;
	D3D12_VERTEX_BUFFER_VIEW		m_d3dTextureCoord1BufferView;

	int								m_nSubMeshes = 0;
	int								*m_pnSubSetIndices = NULL;
	UINT							**m_ppnSubSetIndices = NULL;

	ID3D12Resource					**m_ppd3dSubSetIndexBuffers = NULL;
	ID3D12Resource					**m_ppd3dSubSetIndexUploadBuffers = NULL;
	D3D12_INDEX_BUFFER_VIEW			*m_pd3dSubSetIndexBufferViews = NULL;

	char							m_pstrName[64] = { 0 };
	int								m_nType = 0x00;

public:
	CMesh() {};
	CMesh(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	virtual ~CMesh();

	virtual void CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList) { }
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList) { }
	virtual void ReleaseShaderVariables() { }

	virtual void ReleaseUploadBuffers();
	virtual void OnPreRender(ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, int nSubSet, int nInstances = 1);
	virtual void OnPreRenderToShadow(ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void RenderToShadow(ID3D12GraphicsCommandList *pd3dCommandList, int nSubSet, int nInstances = 1);

	BoundingBox m_xmAABB;
	void SetAABB(XMFLOAT3& xmCenter, XMFLOAT3& xmExtents) { m_xmAABB = BoundingBox(xmCenter, xmExtents); }
	void SetAABB(BoundingBox xmAABB) { m_xmAABB = xmAABB; }
	XMFLOAT3& GetExtents() { return m_xmAABB.Extents; }
	XMFLOAT3& GetCenter() { return m_xmAABB.Center; }
	int GetMeshType() { return m_nType; }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CCubeMesh : public CMesh
{
public:
	CCubeMesh(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, XMFLOAT3 xmf3Center, XMFLOAT3 xmf3Extents);
	virtual ~CCubeMesh();
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CStandardMesh : public CMesh
{
public:
	CStandardMesh() {}
	CStandardMesh(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	virtual ~CStandardMesh();

	virtual void OnPreRender(ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void OnPreRenderToShadow(ID3D12GraphicsCommandList *pd3dCommandList);
	void LoadMeshFromFile(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, FILE *pfile);
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define SKINNED_ANIMATION_BONES		128

class CSkinnedMesh : public CStandardMesh
{
public:
	CSkinnedMesh() {}
	CSkinnedMesh(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	virtual ~CSkinnedMesh();

	virtual void OnPreRender(ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void OnPreRenderToShadow(ID3D12GraphicsCommandList *pd3dCommandList);
	void LoadSkinInfoFromFile(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, FILE *pfile);

protected:
	char						m_pstrSkinnedMeshName[64] = { 0 };

	int							m_nBonesPerVertex = 4;

	XMUINT4						*m_pxmu4BoneIndices = NULL;
	XMFLOAT4					*m_pxmf4BoneWeights = NULL;

	ID3D12Resource				*m_pd3dBoneIndexBuffer = NULL;
	ID3D12Resource				*m_pd3dBoneIndexUploadBuffer = NULL;
	D3D12_VERTEX_BUFFER_VIEW	m_d3dBoneIndexBufferView;

	ID3D12Resource				*m_pd3dBoneWeightBuffer = NULL;
	ID3D12Resource				*m_pd3dBoneWeightUploadBuffer = NULL;
	D3D12_VERTEX_BUFFER_VIEW	m_d3dBoneWeightBufferView;

public:
	int							m_nSkinningBones = 0;

	char						(*m_ppstrSkinningBoneNames)[64];
	XMFLOAT4X4					*m_pxmf4x4BindPoseBoneOffsets = NULL;

	CModel						**m_ppSkinningBoneFrameCaches = NULL;

	ID3D12Resource				*m_pd3dcbBoneOffsets = NULL;
	XMFLOAT4X4					*m_pcbxmf4x4BoneOffsets = NULL;

	ID3D12Resource				*m_pd3dcbBoneTransforms = NULL;
	XMFLOAT4X4					*m_pcbxmf4x4BoneTransforms = NULL;

public:

	virtual void CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void ReleaseShaderVariables();

	virtual void ReleaseUploadBuffers();
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
	CHeightMapGridMesh(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, int xStart, int zStart, int nWidth, int nLength, XMFLOAT3 xmf3Scale = XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT4 xmf4Color = XMFLOAT4(1.0f, 1.0f, 0.0f, 0.0f), void *pContext0 = NULL, void *pContext1 = NULL);
	virtual ~CHeightMapGridMesh();

	XMFLOAT3 GetScale() { return(m_xmf3Scale); }
	int GetWidth() { return(m_nWidth); }
	int GetLength() { return(m_nLength); }

	virtual float OnGetHeight(int x, int z, void *pContext);
	virtual XMFLOAT4 OnGetColor(int x, int z, void *pContext);

	virtual void OnPreRender(ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void OnPreRenderToShadow(ID3D12GraphicsCommandList *pd3dCommandList);
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
class CSkyBoxMesh : public CMesh
{
public:
	CSkyBoxMesh(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, float fWidth = 20.0f, float fHeight = 20.0f, float fDepth = 20.0f);
	virtual ~CSkyBoxMesh();

	virtual void OnPreRender(ID3D12GraphicsCommandList *pd3dCommandList);
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CRect : public CMesh
{
protected:
	XMFLOAT2						*m_pxmf2Positions = NULL;

	XMFLOAT2						*m_pxmf2Sizes = NULL;

	ID3D12Resource					*m_pd3dSizeBuffer = NULL;
	ID3D12Resource					*m_pd3dSizeUploadBuffer = NULL;
	D3D12_VERTEX_BUFFER_VIEW		m_d3dSizeBufferView;

public:
	CRect(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, XMFLOAT2 xmf2Center, XMFLOAT2 xmf2Size);
	virtual ~CRect();

	void ReleaseUploadBuffers();
	virtual void OnPreRender(ID3D12GraphicsCommandList *pd3dCommandList);
};
