#pragma once

#include"Mesh.h"

class CCamera;
class CShader;
class CAnimationSet;

#define RESOURCE_TEXTURE2D			0x01
#define RESOURCE_TEXTURE2D_ARRAY	0x02	//[]
#define RESOURCE_TEXTURE2DARRAY		0x03
#define RESOURCE_TEXTURE_CUBE		0x04
#define RESOURCE_BUFFER				0x05

struct SRVROOTARGUMENTINFO
{
	UINT										m_nRootParameterIndex = 0;
	D3D12_GPU_DESCRIPTOR_HANDLE					m_d3dSrvGpuDescriptorHandle;
};

class CTexture
{
protected:
	UINT							m_nTextureType = RESOURCE_TEXTURE2D;

	int								m_nTextures = 0;
	ID3D12Resource					**m_ppd3dTextures = NULL;
	ID3D12Resource					**m_ppd3dTextureUploadBuffers = NULL;

	SRVROOTARGUMENTINFO				*m_pRootArgumentInfos = NULL;

	int								m_nSamplers = 0;
	D3D12_GPU_DESCRIPTOR_HANDLE		*m_pd3dSamplerGpuDescriptorHandles = NULL;

public:
	CTexture(int nTextureResources = 1, UINT nResourceType = RESOURCE_TEXTURE2D, int nSamplers = 0);
	virtual ~CTexture();

	void SetRootArgument(int nIndex, UINT nRootParameterIndex, D3D12_GPU_DESCRIPTOR_HANDLE d3dsrvGpuDescriptorHandle);
	void SetSampler(int nIndex, D3D12_GPU_DESCRIPTOR_HANDLE d3dSamplerGpuDescriptorHandle);

	void UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList);
	void UpdateShaderVariable(ID3D12GraphicsCommandList *pd3dCommandList, int nIndex);

	void LoadTextureFromFile(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, wchar_t *pszFileName, UINT nIndex);
	ID3D12Resource* CreateTexture(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, UINT nWidth, UINT nHeight, DXGI_FORMAT dxgiFormat, D3D12_RESOURCE_FLAGS d3dResourceFlags, D3D12_RESOURCE_STATES d3dResourceStates, D3D12_CLEAR_VALUE *pd3dClearValue, UINT nIndex);

	int GetTextures() { return(m_nTextures); }
	ID3D12Resource *GetTexture(int nIndex) { return(m_ppd3dTextures[nIndex]); }
	UINT GetTextureType() { return(m_nTextureType); }

	void ReleaseUploadBuffers();
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

#define MATERIAL_ALBEDO_MAP				0x01
#define MATERIAL_SPECULAR_FACTOR_MAP	0x02
#define MATERIAL_NORMAL_MAP				0x04
#define MATERIAL_EMISSION_MAP			0x08
#define MATERIAL_METALLIC_MAP			0x10
#define MATERIAL_DETAIL_ALBEDO_MAP		0x20
#define MATERIAL_DETAIL_NORMAL_MAP		0x40

struct CB_GAMEOBJECT_INFO;
struct VS_VB_INSTANCE;

class CMaterial
{
public:
	CMaterial();
	virtual ~CMaterial();

protected:
	XMFLOAT4						m_xmf4DiffuseColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	XMFLOAT4						m_xmf4EmissiveColor = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	XMFLOAT4						m_xmf4SpecularColor = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	XMFLOAT4						m_xmf4AmbientColor = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	XMFLOAT4						m_xmf4ReflectionColor = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);

	UINT							m_nType = 0x00;

	float							m_fShininess = 0.0f;
	float							m_fTransparency = 0.0f;

	float							m_fSmoothness = 0.0f;
	float							m_fSpecularHighlight = 0.0f;
	float							m_fMetallic = 0.0f;
	float							m_fGlossyReflection = 0.0f;

	std::vector<CTexture*>			m_vTextures; 


public:
	void SetMaterialType(UINT nType) { m_nType |= nType; }
	void SetTexture(CTexture *pTexture);

	void UpdateShaderVariable(ID3D12GraphicsCommandList *pd3dCommandList, CB_GAMEOBJECT_INFO* pcbMappedGameObject);
	void UpdateShaderVariable(VS_VB_INSTANCE* pcbMappedGameObject);
	void UpdateTextureShaderVariable(ID3D12GraphicsCommandList *pd3dCommandList);

	void ReleaseUploadBuffers();

public:
	void LoadMaterialFromFile(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, FILE *pfile, const char *pstrFilePath);
};

////////////////////////////////////////////////////////////////////////////////

class CModel
{
public:
	CModel();
	virtual ~CModel();

public:
	void AddRef();
	void Release();

protected:
	int				m_nReferences = 0;

protected:
	CMesh			*m_pMesh = NULL;
	CCubeMesh		*m_pCubeMesh = NULL;

	int				m_nMaterials;
	CMaterial		**m_ppMaterials = NULL;

	XMFLOAT4X4		m_xmf4x4ToParent;
	XMFLOAT4X4		m_xmf4x4World;

	CModel			*m_pParent = NULL;
	CModel			*m_pSibling = NULL;
	CModel			*m_pChild = NULL;

	char			m_pstrModelName[64] = { 0 };

	CAnimationSet	*m_ppAnimationSets[2] = { 0 };

public:
	void ReleaseUploadBuffers();
	void UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList, ID3D12Resource* vd3dcbGameObject, CB_GAMEOBJECT_INFO* vcbMappedGameObject);

	void RenderWire(ID3D12GraphicsCommandList *pd3dCommandList, CCamera* pCamera, std::vector<ID3D12Resource*>& vd3dcbGameObject, std::vector<CB_GAMEOBJECT_INFO*>& vcbMappedGameObject, int *pnIndex, int nInstances = 1);
	void Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera* pCamera, std::vector<ID3D12Resource*>& vd3dcbGameObject, std::vector<CB_GAMEOBJECT_INFO*>& vcbMappedGameObject, int *pnIndex, bool bSetTexture = true);
	void Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera, int nInstances = 1);
	void RenderToShadow(ID3D12GraphicsCommandList *pd3dCommandList, CCamera* pCamera, std::vector<ID3D12Resource*>& vd3dcbGameObject, std::vector<CB_GAMEOBJECT_INFO*>& vcbMappedGameObject, int *pnIndex, bool bSetTexture = true);
	void RenderToShadow(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera, int nInstances = 1);
	void UpdateInstanceShaderVariables(VS_VB_INSTANCE *m_pcbMappedGameObjects, int *pnIndex);

	void SetMesh(CMesh *pMesh, CCubeMesh *pCubeMesh, bool bIsSkinned);
	void SetMaterial(CMaterial **ppMaterials, UINT nMaterials) { m_ppMaterials = ppMaterials; m_nMaterials = nMaterials; }
	XMFLOAT4X4 GetWorldTransf() { return m_xmf4x4World; }
	CAnimationSet* GetAnimationSet(int nIndex) { return m_ppAnimationSets[nIndex]; }

	CMesh* GetMesh() { return m_pMesh; }

public:
	void SetChild(CModel *pChild, bool bAddReference = false);
	void SetToParent(XMFLOAT4X4 xmf4x4ToParent) { m_xmf4x4ToParent = xmf4x4ToParent; }
	const char* GetModelName() { return m_pstrModelName; };

	XMFLOAT4X4 GetToParent() { return m_xmf4x4ToParent; }

public:
	void UpdateCollisionBox(std::vector<BoundingBox>& vxmAABB, int *pnIndex);
	void UpdateWorldTransform(XMFLOAT4X4 *pxmf4x4Parent);

	void GetMeshes(int *pnStandardMeshes, int *pnSkinnedMeshes);
	void GetSkinnedMeshes(std::vector<CSkinnedMesh*>& vMeshes);

public:
	CModel* FindFrame(const char *pstrFrame);
	void CacheSkinningBoneFrames(CModel *pRootModel);

public: // Root Model
	const char* GetFileName() { return m_pstrFileName; };
	void SetModelMeshCount(int nMeshes, int nSkinnedMeshes) { m_nMeshes = nMeshes; m_nSkinnedMeshes = nSkinnedMeshes; }
	void SetFileName(const char *pstrFileName) { m_pstrFileName = pstrFileName; }

	int GetMeshes() { return m_nMeshes; }
	int GetSkinnedMeshes() { return m_nSkinnedMeshes; }

	XMFLOAT3 GetRight() { return XMFLOAT3(m_xmf4x4World._11, m_xmf4x4World._12, m_xmf4x4World._13); }
	XMFLOAT3 GetUp() { return XMFLOAT3(m_xmf4x4World._21, m_xmf4x4World._22, m_xmf4x4World._23); }
	XMFLOAT3 GetLook() { return XMFLOAT3(m_xmf4x4World._31, m_xmf4x4World._32, m_xmf4x4World._33); }
	XMFLOAT3 GetPosition() { return XMFLOAT3(m_xmf4x4World._41, m_xmf4x4World._42, m_xmf4x4World._43); }

protected:
	// Root Model = Total
	int				m_nMeshes = 0; // All Mesh
	int				m_nSkinnedMeshes = 0; // Only Skinning Mesh

	const char		*m_pstrFileName = NULL;

public:
	static CModel* LoadGeometryAndAnimationFromFile(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, char *pstrFileName, char *pstrUpperAniFileName, char *pstrUnderAniFileName);
	static CModel* LoadModelFromFile(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, FILE *pfile, const char *pstrFileName, const char *pstrFilePath);
	static CAnimationSet* LoadAnimationFromFile(CModel *pModel, const char* pstrFileName);
};