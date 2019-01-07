#pragma once

#include"Mesh.h"

class CCamera;
class CShader;

#define RESOURCE_TEXTURE2D			0x01
#define RESOURCE_TEXTURE2D_ARRAY	0x02	//[]
#define RESOURCE_TEXTURE2DARRAY		0x03
#define RESOURCE_TEXTURE_CUBE		0x04
#define RESOURCE_BUFFER				0x05

struct SRVROOTARGUMENTINFO
{
	UINT										m_nRootParameterIndex = 0;
	std::vector<D3D12_GPU_DESCRIPTOR_HANDLE>	m_vd3dSrvGpuDescriptorHandle;
};

#define TEXTURE_ALBEDO_MAP			0
#define TEXTURE_SPECULAR_MAP		1
#define TEXTURE_NORMAL_MAP			2
#define TEXTURE_METALLIC_MAP		3
#define TEXTURE_EMISSION_MAP		4
#define TEXTURE_DETAIL_ALBEDO_MAP	5
#define TEXTURE_DETAIL_NORMAL_MAP	6

class CTexture
{
protected:
	UINT							m_nTextureType = RESOURCE_TEXTURE2D;

	int								m_nTextures = 0;
	ID3D12Resource					**m_ppd3dTextures = NULL;
	ID3D12Resource					**m_ppd3dTextureUploadBuffers;

	SRVROOTARGUMENTINFO				*m_pRootArgumentInfos = NULL;

	int								m_nSamplers = 0;
	D3D12_GPU_DESCRIPTOR_HANDLE		*m_pd3dSamplerGpuDescriptorHandles = NULL;

public:
	CTexture(int nTextureResources = 1, UINT nResourceType = RESOURCE_TEXTURE2D, int nSamplers = 0);
	virtual ~CTexture();

	int SetRootArgument(int nIndex, UINT nRootParameterIndex, D3D12_GPU_DESCRIPTOR_HANDLE d3dsrvGpuDescriptorHandle);
	void SetSampler(int nIndex, D3D12_GPU_DESCRIPTOR_HANDLE d3dSamplerGpuDescriptorHandle);

	void UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList, int nHandleIndex);
	void UpdateShaderVariable(ID3D12GraphicsCommandList *pd3dCommandList, int nIndex, int nHandleIndex);

	void LoadTextureFromFile(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, wchar_t *pszFileName, UINT nIndex);
	ID3D12Resource *CreateTexture(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, UINT nWidth, UINT nHeight, DXGI_FORMAT dxgiFormat, D3D12_RESOURCE_FLAGS d3dResourceFlags, D3D12_RESOURCE_STATES d3dResourceStates, D3D12_CLEAR_VALUE *pd3dClearValue, UINT nIndex);

	int GetTextures() { return(m_nTextures); }
	ID3D12Resource *GetTexture(int nIndex) { return(m_ppd3dTextures[nIndex]); }
	UINT GetTextureType() { return(m_nTextureType); }

	void ReleaseUploadBuffers();

protected:
	int					m_nType = TEXTURE_ALBEDO_MAP;

public:
	int GetType() { return m_nType; }
	void SetType(int nType) { m_nType = nType; }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

#define MATERIAL_ALBEDO_MAP			0x01
#define MATERIAL_SPECULAR_MAP		0x02
#define MATERIAL_NORMAL_MAP			0x04
#define MATERIAL_METALLIC_MAP		0x08
#define MATERIAL_EMISSION_MAP		0x10
#define MATERIAL_DETAIL_ALBEDO_MAP	0x20
#define MATERIAL_DETAIL_NORMAL_MAP	0x40

struct MATERIAL
{
	XMFLOAT4		m_xmf4Ambient;
	XMFLOAT4		m_xmf4Diffuse;
	XMFLOAT4		m_xmf4Specular;
	XMFLOAT4		m_xmf4Emissive;
};

struct CB_GAMEOBJECT_INFO;

class CMaterial
{
public:
	CMaterial();
	virtual ~CMaterial();

protected:
	XMFLOAT4						m_xmf4AlbedoColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	XMFLOAT4						m_xmf4EmissiveColor = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	XMFLOAT4						m_xmf4SpecularColor = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	XMFLOAT4						m_xmf4AmbientColor = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);

	UINT							m_nType = 0x00;

	float							m_fGlossiness = 0.0f;
	float							m_fSmoothness = 0.0f;
	float							m_fSpecularHighlight = 0.0f;
	float							m_fMetallic = 0.0f;
	float							m_fGlossyReflection = 0.0f;

	std::vector<CTexture*>			m_vTextures; //Albedo, Specular, Metallic, Normal, Emission, DetailAlbedo, DetailNormal

public:
	void SetMaterialType(UINT nType) { m_nType |= nType; }
	void SetTexture(CTexture *pTexture);

	void UpdateShaderVariable(ID3D12GraphicsCommandList *pd3dCommandList, CB_GAMEOBJECT_INFO* pcbMappedGameObject);
	void UpdateTextureShaderVariable(ID3D12GraphicsCommandList *pd3dCommandList, int nHandleIndex);

	void ReleaseUploadBuffers();

public:
	void LoadMaterialFromFBX(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, FbxSurfaceMaterial *pfbxMaterial, const char *pstrFilePath);
	void CreateShaderResourceViewsInMaterial(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, CShader *pShader);

	int GetTextureCount() { return (int)m_vTextures.size(); }
};

////////////////////////////////////////////////////////////////////////////////

class CModel
{
public:
	CModel() {};
	CModel(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, char *pFileName);
	virtual ~CModel();

	void CreateShaderResourceViews(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, CShader *pShader);
protected:
	CMesh			*m_pMesh = NULL;
	CCubeMesh		*m_pCubeMesh = NULL;

	int				m_nMaterials;
	CMaterial		**m_ppMaterials = NULL;

	bool			m_bHasAnimation = false;

public:
	void ReleaseUploadBuffers();

	void UpdateCollisionBox(BoundingOrientedBox &xmOOBB, XMFLOAT4X4 &xmf4x4World);

	void RenderWire(ID3D12GraphicsCommandList *pd3dCommandList, CCamera* pCamera);
	void Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera* pCamera, CB_GAMEOBJECT_INFO* pcbMappedGameObject, int nHandleIndex);

	void SetMesh(CMesh *pMesh, CCubeMesh *pCubeMesh) { m_pMesh = pMesh; m_pCubeMesh = pCubeMesh; }
	void SetMaterial(CMaterial **ppMaterials, UINT nMaterials) { m_ppMaterials = ppMaterials; m_nMaterials = nMaterials; }

protected:
	char			*m_pstrName = NULL;

public:
	bool IsName(char *pstrName) { if(!strcmp(m_pstrName, pstrName))	return true; return false; };
};