#include "stdafx.h"
#include "Model.h"
#include "Mesh.h"
#include "Shader.h"
#include "Scene.h"
#include "Animation.h"

CTexture::CTexture(int nTextures, UINT nTextureType, int nSamplers)
{
	m_nTextureType = nTextureType;
	m_nTextures = nTextures;

	if (m_nTextures > 0)
	{
		m_pRootArgumentInfos = new SRVROOTARGUMENTINFO[m_nTextures];
		::ZeroMemory(m_pRootArgumentInfos, sizeof(SRVROOTARGUMENTINFO) *m_nTextures);

		m_ppd3dTextureUploadBuffers = new ID3D12Resource*[m_nTextures];

		::ZeroMemory(m_ppd3dTextureUploadBuffers, sizeof(ID3D12Resource*) *m_nTextures);

		m_ppd3dTextures = new ID3D12Resource*[m_nTextures];
		::ZeroMemory(m_ppd3dTextures, sizeof(ID3D12Resource*) *m_nTextures);
	}

	m_nSamplers = nSamplers;
	if (m_nSamplers > 0) m_pd3dSamplerGpuDescriptorHandles = new D3D12_GPU_DESCRIPTOR_HANDLE[m_nSamplers];
}

CTexture::~CTexture()
{
	if (m_ppd3dTextures)
	{
		for (int i = 0; i < m_nTextures; i++) 
			if (m_ppd3dTextures[i]) m_ppd3dTextures[i]->Release();
		delete[] m_ppd3dTextures;
		m_ppd3dTextures = NULL;
	}

	if (m_pRootArgumentInfos)
	{
		delete[] m_pRootArgumentInfos;
		m_pRootArgumentInfos = NULL;
	}

	if (m_pd3dSamplerGpuDescriptorHandles)
	{
		delete[] m_pd3dSamplerGpuDescriptorHandles;
		m_pd3dSamplerGpuDescriptorHandles = NULL;
	}
}

void CTexture::SetRootArgument(int nIndex, UINT nRootParameterIndex, D3D12_GPU_DESCRIPTOR_HANDLE d3dSrvGpuDescriptorHandle)
{
	m_pRootArgumentInfos[nIndex].m_nRootParameterIndex = nRootParameterIndex;
	m_pRootArgumentInfos[nIndex].m_d3dSrvGpuDescriptorHandle = d3dSrvGpuDescriptorHandle;
}

void CTexture::SetSampler(int nIndex, D3D12_GPU_DESCRIPTOR_HANDLE d3dSamplerGpuDescriptorHandle)
{
	m_pd3dSamplerGpuDescriptorHandles[nIndex] = d3dSamplerGpuDescriptorHandle;
}

void CTexture::UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList)
{
	if (m_nTextureType == RESOURCE_TEXTURE2D_ARRAY)
	{
		pd3dCommandList->SetGraphicsRootDescriptorTable(m_pRootArgumentInfos[0].m_nRootParameterIndex, m_pRootArgumentInfos[0].m_d3dSrvGpuDescriptorHandle);
	}
	else
	{
		for (int i = 0; i < m_nTextures; i++)
		{
			pd3dCommandList->SetGraphicsRootDescriptorTable(m_pRootArgumentInfos[i].m_nRootParameterIndex, m_pRootArgumentInfos[i].m_d3dSrvGpuDescriptorHandle);
		}
	}
}

void CTexture::UpdateShaderVariable(ID3D12GraphicsCommandList *pd3dCommandList, int nIndex)
{
	pd3dCommandList->SetGraphicsRootDescriptorTable(m_pRootArgumentInfos[nIndex].m_nRootParameterIndex, m_pRootArgumentInfos[nIndex].m_d3dSrvGpuDescriptorHandle);
}

void CTexture::ReleaseUploadBuffers()
{
	if (m_ppd3dTextureUploadBuffers)
	{
		for (int i = 0; i < m_nTextures; i++)
		{
			if (m_ppd3dTextureUploadBuffers[i]) m_ppd3dTextureUploadBuffers[i]->Release();
			m_ppd3dTextureUploadBuffers[i] = NULL;
		}

		delete[] m_ppd3dTextureUploadBuffers;
		m_ppd3dTextureUploadBuffers = NULL;
	}
}

void CTexture::LoadTextureFromFile(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, wchar_t *pszFileName, UINT nIndex)
{
	m_ppd3dTextures[nIndex] = ::CreateTextureResourceFromFile(pd3dDevice, pd3dCommandList, pszFileName, &m_ppd3dTextureUploadBuffers[nIndex], D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
}

ID3D12Resource * CTexture::CreateTexture(ID3D12Device * pd3dDevice, ID3D12GraphicsCommandList * pd3dCommandList, UINT nWidth, UINT nHeight, DXGI_FORMAT dxgiFormat, D3D12_RESOURCE_FLAGS d3dResourceFlags, D3D12_RESOURCE_STATES d3dResourceStates, D3D12_CLEAR_VALUE * pd3dClearValue, UINT nIndex)
{
	m_ppd3dTextures[nIndex] = ::CreateTexture2DResource(pd3dDevice, pd3dCommandList, nWidth, nHeight, dxgiFormat, d3dResourceFlags, d3dResourceStates, pd3dClearValue);
	return(m_ppd3dTextures[nIndex]);
}

///////////////////////////////////////////////////////////////////////////////

CMaterial::CMaterial()
{
}

CMaterial::~CMaterial()
{
	for (auto& Texture : m_vTextures)
		if (Texture)
			delete Texture;

	m_vTextures.empty();
}

void CMaterial::SetTexture(CTexture *pTexture)
{
	m_vTextures.emplace_back(pTexture);
}

void CMaterial::UpdateShaderVariable(ID3D12GraphicsCommandList *pd3dCommandList, CB_GAMEOBJECT_INFO* pcbMappedGameObject)
{
	pcbMappedGameObject->m_Material.m_xmf4Ambient = m_xmf4AmbientColor;
	pcbMappedGameObject->m_Material.m_xmf4Diffuse = m_xmf4DiffuseColor;
	pcbMappedGameObject->m_Material.m_xmf4Emissive = m_xmf4EmissiveColor;
	pcbMappedGameObject->m_Material.m_xmf4Specular = m_xmf4SpecularColor;
	pcbMappedGameObject->m_Material.m_fReflectionFactor = m_xmf4ReflectionColor.w;
	pcbMappedGameObject->m_nTexturesMask = m_nType;
}

void CMaterial::UpdateShaderVariable(VS_VB_INSTANCE* pcbMappedGameObject)
{
	pcbMappedGameObject->m_Material.m_xmf4Ambient = m_xmf4AmbientColor;
	pcbMappedGameObject->m_Material.m_xmf4Diffuse = m_xmf4DiffuseColor;
	pcbMappedGameObject->m_Material.m_xmf4Emissive = m_xmf4EmissiveColor;
	pcbMappedGameObject->m_Material.m_xmf4Specular = m_xmf4SpecularColor;
	pcbMappedGameObject->m_Material.m_fReflectionFactor = m_xmf4ReflectionColor.w;
	pcbMappedGameObject->m_nTexturesMask = m_nType;
}

void CMaterial::UpdateTextureShaderVariable(ID3D12GraphicsCommandList *pd3dCommandList)
{
	for (auto& Texture : m_vTextures)
	{
		if (Texture)
		{
			Texture->UpdateShaderVariables(pd3dCommandList);
		}
	}
}

void CMaterial::ReleaseUploadBuffers()
{
	for (auto& Texture : m_vTextures)
	{
		if (Texture)
			Texture->ReleaseUploadBuffers();
	}
}

void CMaterial::LoadMaterialFromFile(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, FILE *pfile, const char *pstrFilePath)
{
	BYTE nstrLength;
	char pstrToken[64] = { 0 };

	while(true)
	{
		fread_s(&nstrLength, sizeof(BYTE), sizeof(BYTE), 1, pfile);
		fread_s(pstrToken, sizeof(char) * 64, sizeof(char), nstrLength, pfile);  // <Transforms>:
		pstrToken[nstrLength] = '\0';

		if (!strcmp(pstrToken, "<AmbientColor>:"))
		{
			fread_s(&m_xmf4AmbientColor, sizeof(XMFLOAT4), sizeof(XMFLOAT4), 1, pfile);
		}
		else if (!strcmp(pstrToken, "<DiffuseColor>:"))
		{
			fread_s(&m_xmf4DiffuseColor, sizeof(XMFLOAT4), sizeof(XMFLOAT4), 1, pfile);
		}
		else if (!strcmp(pstrToken, "<EmissiveColor>:"))
		{
			fread_s(&m_xmf4EmissiveColor, sizeof(XMFLOAT4), sizeof(XMFLOAT4), 1, pfile);
		}
		else if (!strcmp(pstrToken, "<ReflectionColor>:"))
		{
			fread_s(&m_xmf4ReflectionColor, sizeof(XMFLOAT4), sizeof(XMFLOAT4), 1, pfile);
		}
		else if (!strcmp(pstrToken, "<SpecularColor>:"))
		{
			fread_s(&m_xmf4SpecularColor, sizeof(XMFLOAT4), sizeof(XMFLOAT4), 1, pfile);
		}
		else if (!strcmp(pstrToken, "<ReflectionFactor>:"))
		{
			fread_s(&m_xmf4ReflectionColor.w, sizeof(float), sizeof(float), 1, pfile);
		}
		else if (!strcmp(pstrToken, "<Shininess>:"))
		{
			fread_s(&m_fShininess, sizeof(float), sizeof(float), 1, pfile);
		}
		else if (!strcmp(pstrToken, "<SpecularFactor>:"))
		{
			fread_s(&m_xmf4SpecularColor.w, sizeof(float), sizeof(float), 1, pfile);
		}
		else if (!strcmp(pstrToken, "<Transparency>:"))
		{
			fread_s(&m_fTransparency, sizeof(float), sizeof(float), 1, pfile);
		}
		else if (!strcmp(pstrToken, "<DiffuseMap>:"))
		{
			char pstrFileName[64] = { 0 };
			fread_s(&nstrLength, sizeof(BYTE), sizeof(BYTE), 1, pfile);
			fread_s(pstrFileName, sizeof(char) * 64, sizeof(char), nstrLength, pfile);

			if (!strcmp(pstrFileName, "null")) continue;

			char pstrDDSFile[64] = { 0 };
			strcpy_s(pstrDDSFile, pstrFilePath);
			strcat_s(pstrDDSFile, pstrFileName);
			strcat_s(pstrDDSFile, ".dds");

			INT nLen = (int)(strlen(pstrDDSFile)) + 1;
			WCHAR* pwstrFileName = (LPWSTR)new WCHAR[sizeof(WCHAR)*nLen];
			MultiByteToWideChar(949, 0, pstrDDSFile, -1, pwstrFileName, nLen);

			CTexture *pTexture = new CTexture(1, RESOURCE_TEXTURE2D, 0);

			m_nType |= MATERIAL_ALBEDO_MAP;

			pTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, pwstrFileName, 0);
			CScene::CreateShaderResourceViews(pd3dDevice, pTexture, ROOT_PARAMETER_INDEX_DIFFUSE_TEXTURE_ARRAY, false, true);
			printf("Diffuse Map Name : [%s]\n", pstrFileName);
			m_vTextures.emplace_back(pTexture);

			delete pwstrFileName;
		}
		else if (!strcmp(pstrToken, "<NormalMap>:"))
		{
			char pstrFileName[64] = { 0 };
			fread_s(&nstrLength, sizeof(BYTE), sizeof(BYTE), 1, pfile);
			fread_s(pstrFileName, sizeof(char) * 64, sizeof(char), nstrLength, pfile);

			if (!strcmp(pstrFileName, "null")) continue;

			char pstrDDSFile[64] = { 0 };
			strcpy_s(pstrDDSFile, pstrFilePath);
			strcat_s(pstrDDSFile, pstrFileName);
			strcat_s(pstrDDSFile, ".dds");

			INT nLen = (int)(strlen(pstrDDSFile)) + 1;
			WCHAR* pwstrFileName = (LPWSTR)new WCHAR[sizeof(WCHAR)*nLen];
			MultiByteToWideChar(949, 0, pstrDDSFile, -1, pwstrFileName, nLen);

			CTexture *pTexture = new CTexture(1, RESOURCE_TEXTURE2D, 0);

			m_nType |= MATERIAL_NORMAL_MAP;

			pTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, pwstrFileName, 0);
			CScene::CreateShaderResourceViews(pd3dDevice, pTexture, ROOT_PARAMETER_INDEX_NORMAL_TEXTURE, false, true);
			printf("Normal Map Name : [%s]\n", pstrFileName);
			m_vTextures.emplace_back(pTexture);

			delete pwstrFileName;
		}
		else if (!strcmp(pstrToken, "<SpecularFactorMap>:"))
		{
			char pstrFileName[64] = { 0 };
			fread_s(&nstrLength, sizeof(BYTE), sizeof(BYTE), 1, pfile);
			fread_s(pstrFileName, sizeof(char) * 64, sizeof(char), nstrLength, pfile);

			if(!strcmp(pstrFileName, "null")) continue;

			char pstrDDSFile[64] = { 0 };
			strcpy_s(pstrDDSFile, pstrFilePath);
			strcat_s(pstrDDSFile, pstrFileName);
			strcat_s(pstrDDSFile, ".dds");

			INT nLen = (int)(strlen(pstrDDSFile)) + 1;
			WCHAR* pwstrFileName = (LPWSTR)new WCHAR[sizeof(WCHAR)*nLen];
			MultiByteToWideChar(949, 0, pstrDDSFile, -1, pwstrFileName, nLen);

			CTexture *pTexture = new CTexture(1, RESOURCE_TEXTURE2D, 0);

			m_nType |= MATERIAL_SPECULAR_FACTOR_MAP;

			pTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, pwstrFileName, 0);
			CScene::CreateShaderResourceViews(pd3dDevice, pTexture, ROOT_PARAMETER_INDEX_SPECULAR_TEXTURE, false, true);
			printf("Specular Factor Map Name : [%s]\n", pstrFileName);
			m_vTextures.emplace_back(pTexture);

			delete pwstrFileName;
		}
		else if (!strcmp(pstrToken, "<SpecularMap>:"))
		{
			char pstrFileName[64] = { 0 };
			fread_s(&nstrLength, sizeof(BYTE), sizeof(BYTE), 1, pfile);
			fread_s(pstrFileName, sizeof(char) * 64, sizeof(char), nstrLength, pfile);

			if (!strcmp(pstrFileName, "null")) continue;
		}
		else if (!strcmp(pstrToken, "<EmissiveMap>:"))
		{
			char pstrFileName[64] = { 0 };
			fread_s(&nstrLength, sizeof(BYTE), sizeof(BYTE), 1, pfile);
			fread_s(pstrFileName, sizeof(char) * 64, sizeof(char), nstrLength, pfile);

			if (!strcmp(pstrFileName, "null")) continue;

			char pstrDDSFile[64] = { 0 };
			strcpy_s(pstrDDSFile, pstrFilePath);
			strcat_s(pstrDDSFile, pstrFileName);
			strcat_s(pstrDDSFile, ".dds");

			INT nLen = (int)(strlen(pstrDDSFile)) + 1;
			WCHAR* pwstrFileName = (LPWSTR)new WCHAR[sizeof(WCHAR)*nLen];
			MultiByteToWideChar(949, 0, pstrDDSFile, -1, pwstrFileName, nLen);

			CTexture *pTexture = new CTexture(1, RESOURCE_TEXTURE2D, 0);

			m_nType |= MATERIAL_EMISSION_MAP;

			pTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, pwstrFileName, 0);
			CScene::CreateShaderResourceViews(pd3dDevice, pTexture, ROOT_PARAMETER_INDEX_EMISSIVE_TEXTURE, false, true);
			printf("Emissive Map Name : [%s]\n", pstrFileName);
			m_vTextures.emplace_back(pTexture);

			delete pwstrFileName;
		}
		else if (!strcmp(pstrToken, "</Material>"))
		{
			break;
		}
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CModel::CModel()
{
	m_xmf4x4ToParent = Matrix4x4::Identity();
	m_xmf4x4World = Matrix4x4::Identity();
}

CModel::~CModel()
{
	if (m_pMesh)
	{
		delete m_pMesh;
		m_pMesh = NULL;

		if (m_pCubeMesh)
		{
			delete m_pCubeMesh;
			m_pCubeMesh = NULL;
		}
	}

	if (m_ppMaterials)
	{
		for (int i = 0; i < m_nMaterials; i++)
			if (m_ppMaterials[i]) delete m_ppMaterials[i];
		delete[] m_ppMaterials;
		m_ppMaterials = NULL;
	}

	if (m_ppAnimationSets[0])
	{
		delete m_ppAnimationSets[0];
		m_ppAnimationSets[0] = NULL;
	}
	if (m_ppAnimationSets[1])
	{
		delete m_ppAnimationSets[1];
		m_ppAnimationSets[1] = NULL;
	}
}

void CModel::AddRef()
{
	m_nReferences++;

	if (m_pSibling) m_pSibling->AddRef();
	if (m_pChild) m_pChild->AddRef();
}

void CModel::Release()
{
	if (m_pSibling) m_pSibling->Release();
	if (m_pChild) m_pChild->Release();

	if (--m_nReferences <= 0) delete this;
}

void CModel::SetChild(CModel *pChild, bool bAddReference)
{
	pChild->m_pParent = this;
	if (bAddReference) pChild->AddRef();

	if (m_pChild)
	{
		pChild->m_pSibling = m_pChild->m_pSibling;
		m_pChild->m_pSibling = pChild;
	}
	else m_pChild = pChild;
}

void CModel::SetMesh(CMesh *pMesh, CCubeMesh *pCubeMesh, bool bIsSkinned)
{
	if (!m_pMesh)
	{
		if(bIsSkinned) m_nSkinnedMeshes++;

		m_nMeshes++;
	}

	m_pMesh = pMesh; 
	m_pCubeMesh = pCubeMesh;
}

void CModel::ReleaseUploadBuffers()
{
	if (m_pMesh)
	{
		m_pMesh->ReleaseUploadBuffers();
		if (m_pCubeMesh) m_pCubeMesh->ReleaseUploadBuffers();
	}

	if (m_ppMaterials)
	{
		for (int i = 0; i < m_nMaterials; i++)
		{
			if (m_ppMaterials[i]) m_ppMaterials[i]->ReleaseUploadBuffers();
		}
	}

	if (m_pSibling) m_pSibling->ReleaseUploadBuffers();
	if (m_pChild) m_pChild->ReleaseUploadBuffers();
}

void CModel::GetMeshes(int *pnMeshes, int *pnSkinnedMeshes)
{
	(*pnMeshes) += m_nMeshes;
	(*pnSkinnedMeshes) += m_nSkinnedMeshes;

	if (m_pSibling) m_pSibling->GetMeshes(pnMeshes, pnSkinnedMeshes);
	if (m_pChild) m_pChild->GetMeshes(pnMeshes, pnSkinnedMeshes);
}

void CModel::GetSkinnedMeshes(std::vector<CSkinnedMesh*>& vMeshes)
{
	if (m_pMesh && (m_pMesh->GetMeshType() & TYPE_SKINNED_MESH))
		vMeshes.emplace_back((CSkinnedMesh*)m_pMesh);

	if (m_pSibling) m_pSibling->GetSkinnedMeshes(vMeshes);
	if (m_pChild) m_pChild->GetSkinnedMeshes(vMeshes);
}

CModel* CModel::FindFrame(const char *pstrFrame)
{
	if (!strcmp(m_pstrModelName, pstrFrame))
		return this;

	CModel *pReturnModel = NULL;

	if (m_pSibling && (!pReturnModel)) pReturnModel = m_pSibling->FindFrame(pstrFrame);
	if (m_pChild && (!pReturnModel)) pReturnModel = m_pChild->FindFrame(pstrFrame);

	return pReturnModel;
}

void CModel::UpdateWorldTransform(XMFLOAT4X4 *pxmf4x4Parent)
{
	m_xmf4x4World = (pxmf4x4Parent) ? Matrix4x4::Multiply(m_xmf4x4ToParent, *pxmf4x4Parent) : m_xmf4x4ToParent;

	if (m_pSibling) m_pSibling->UpdateWorldTransform(pxmf4x4Parent);
	if (m_pChild) m_pChild->UpdateWorldTransform(&m_xmf4x4World);
}

void CModel::UpdateCollisionBox(std::vector<BoundingBox>& vxmAABB, int *pnIndex)
{
	if ((*pnIndex) == vxmAABB.size())
		return;

	if (m_pMesh) m_pMesh->m_xmAABB.Transform(vxmAABB[(*pnIndex)++], XMLoadFloat4x4(&m_xmf4x4World));

	if (m_pSibling) m_pSibling->UpdateCollisionBox(vxmAABB, pnIndex);
	if (m_pChild) m_pChild->UpdateCollisionBox(vxmAABB, pnIndex);
}

void CModel::UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList, ID3D12Resource* pd3dcbGameObject, CB_GAMEOBJECT_INFO* pcbMappedGameObject)
{
	D3D12_GPU_VIRTUAL_ADDRESS d3dGpuVirtualAddress = pd3dcbGameObject->GetGPUVirtualAddress();
	pd3dCommandList->SetGraphicsRootConstantBufferView(ROOT_PARAMETER_INDEX_OBJECT, d3dGpuVirtualAddress);

	XMStoreFloat4x4(&pcbMappedGameObject->m_xmf4x4World, XMMatrixTranspose(XMLoadFloat4x4(&m_xmf4x4World)));
}

void CModel::UpdateInstanceShaderVariables(VS_VB_INSTANCE *m_pcbMappedGameObjects, int *pnIndex)
{
	if (m_pMesh)
	{
		XMStoreFloat4x4(&m_pcbMappedGameObjects[*pnIndex].m_xmf4x4World, XMMatrixTranspose(XMLoadFloat4x4(&m_xmf4x4World)));

		if (m_nMaterials > 0)
		{
			for (int i = 0; i < m_nMaterials; i++)
			{
				if (m_ppMaterials[i]) m_ppMaterials[i]->UpdateShaderVariable(&m_pcbMappedGameObjects[*pnIndex]);
			}
		}

		(*pnIndex)++;
	}

	if (m_pSibling) m_pSibling->UpdateInstanceShaderVariables(m_pcbMappedGameObjects, pnIndex);
	if (m_pChild) m_pChild->UpdateInstanceShaderVariables(m_pcbMappedGameObjects, pnIndex);
}

void CModel::Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera* pCamera, std::vector<ID3D12Resource*>& vd3dcbGameObject, std::vector<CB_GAMEOBJECT_INFO*>& vcbMappedGameObject, int *pnIndex, bool bSetTexture)
{
	if ((*pnIndex) == vd3dcbGameObject.size())
		return;

	if (m_pMesh)
	{
		if(m_nMeshes > 0) UpdateShaderVariables(pd3dCommandList, vd3dcbGameObject[*pnIndex], vcbMappedGameObject[*pnIndex]);

		if (m_nMaterials > 0)
		{

			for (int i = 0; i < m_nMaterials; i++)
			{
				if (m_ppMaterials[i])
				{
					m_ppMaterials[i]->UpdateShaderVariable(pd3dCommandList, vcbMappedGameObject[*pnIndex]);

					if(bSetTexture) m_ppMaterials[i]->UpdateTextureShaderVariable(pd3dCommandList);
				}

				m_pMesh->Render(pd3dCommandList, i);
			}
		}

		(*pnIndex)++;
	}

	if (m_pSibling) m_pSibling->Render(pd3dCommandList, pCamera, vd3dcbGameObject, vcbMappedGameObject, pnIndex, bSetTexture);
	if (m_pChild) m_pChild->Render(pd3dCommandList, pCamera, vd3dcbGameObject, vcbMappedGameObject, pnIndex, bSetTexture);
}

void CModel::Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera, int nInstances)
{
	if (m_pMesh)
	{
		if (m_nMaterials > 0)
		{
			for (int i = 0; i < m_nMaterials; i++)
			{
				if (m_ppMaterials[i])
				{
					m_ppMaterials[i]->UpdateTextureShaderVariable(pd3dCommandList);
				}

				m_pMesh->Render(pd3dCommandList, i, nInstances);
			}
		}
	}

	if (m_pSibling) m_pSibling->Render(pd3dCommandList, pCamera, nInstances);
	if (m_pChild) m_pChild->Render(pd3dCommandList, pCamera, nInstances);
}

void CModel::RenderToShadow(ID3D12GraphicsCommandList *pd3dCommandList, CCamera* pCamera, std::vector<ID3D12Resource*>& vd3dcbGameObject, std::vector<CB_GAMEOBJECT_INFO*>& vcbMappedGameObject, int *pnIndex, bool bSetTexture)
{
	if ((*pnIndex) == vd3dcbGameObject.size())
		return;

	if (m_pMesh)
	{
		if(m_nMeshes > 0) UpdateShaderVariables(pd3dCommandList, vd3dcbGameObject[*pnIndex], vcbMappedGameObject[*pnIndex]);

		if (m_nMaterials > 0)
		{
			for (int i = 0; i < m_nMaterials; i++)
			{
				if (m_ppMaterials[i])
				{
					m_ppMaterials[i]->UpdateShaderVariable(pd3dCommandList, vcbMappedGameObject[*pnIndex]);

					if(bSetTexture) m_ppMaterials[i]->UpdateTextureShaderVariable(pd3dCommandList);
				}

				m_pMesh->RenderToShadow(pd3dCommandList, i);
			}
		}

		(*pnIndex)++;
	}

	if (m_pSibling) m_pSibling->RenderToShadow(pd3dCommandList, pCamera, vd3dcbGameObject, vcbMappedGameObject, pnIndex, bSetTexture);
	if (m_pChild) m_pChild->RenderToShadow(pd3dCommandList, pCamera, vd3dcbGameObject, vcbMappedGameObject, pnIndex, bSetTexture);
}

void CModel::RenderToShadow(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera, int nInstances)
{
	if (m_pMesh)
	{
		if (m_nMaterials > 0)
		{
			for (int i = 0; i < m_nMaterials; i++)
			{
				if (m_ppMaterials[i])
				{
					m_ppMaterials[i]->UpdateTextureShaderVariable(pd3dCommandList);
				}

				m_pMesh->RenderToShadow(pd3dCommandList, i, nInstances);
			}
		}
	}

	if (m_pSibling) m_pSibling->RenderToShadow(pd3dCommandList, pCamera, nInstances);
	if (m_pChild) m_pChild->RenderToShadow(pd3dCommandList, pCamera, nInstances);
}

void CModel::RenderWire(ID3D12GraphicsCommandList *pd3dCommandList, CCamera* pCamera, std::vector<ID3D12Resource*>& vd3dcbGameObject, std::vector<CB_GAMEOBJECT_INFO*>& vcbMappedGameObject, int *pnIndex, int nInstances)
{
	if ((*pnIndex) == vd3dcbGameObject.size())
		return;

	if (m_pCubeMesh)
	{
		if (m_nMeshes > 0 && nInstances == 1) UpdateShaderVariables(pd3dCommandList, vd3dcbGameObject[*pnIndex], vcbMappedGameObject[*pnIndex]);

		m_pCubeMesh->Render(pd3dCommandList, 0, nInstances);

		(*pnIndex)++;
	}

	if (m_pSibling) m_pSibling->RenderWire(pd3dCommandList, pCamera, vd3dcbGameObject, vcbMappedGameObject, pnIndex, nInstances);
	if (m_pChild) m_pChild->RenderWire(pd3dCommandList, pCamera, vd3dcbGameObject, vcbMappedGameObject, pnIndex, nInstances);
}

CModel* CModel::LoadGeometryAndAnimationFromFile(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, char *pstrFileName, char *pstrUpperAniFileName, char *pstrUnderAniFileName)
{
	FILE *pFile;
	fopen_s(&pFile, pstrFileName, "rb");
	printf("File : %s\n", pstrFileName);

	std::string pstrFilePath = ::GetFilePath(pstrFileName);
	printf("File Path : %s\n", pstrFilePath.c_str());

	CModel *pRootModel = CModel::LoadModelFromFile(pd3dDevice, pd3dCommandList, pFile, pstrFileName, pstrFilePath.c_str());
	int nMeshes = 0, nSkinnedMeshes = 0;
	pRootModel->GetMeshes(&nMeshes, &nSkinnedMeshes);
	pRootModel->SetModelMeshCount(nMeshes, nSkinnedMeshes);
	if (pstrUpperAniFileName) pRootModel->m_ppAnimationSets[ANIMATION_UP] = CModel::LoadAnimationFromFile(pRootModel, pstrUpperAniFileName);
	if (pstrUnderAniFileName) pRootModel->m_ppAnimationSets[ANIMATION_DOWN] = CModel::LoadAnimationFromFile(pRootModel, pstrUnderAniFileName);
	pRootModel->CacheSkinningBoneFrames(pRootModel);

	return pRootModel;
}

CModel* CModel::LoadModelFromFile(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, FILE *pfile, const char *pstrFileName, const char *pstrFilePath)
{
	CModel *pModel = NULL;

	while (true)
	{
		BYTE nstrLength;
		char pstrToken[64] = { 0 };
		fread_s(&nstrLength, sizeof(BYTE), sizeof(BYTE), 1, pfile);
		fread_s(pstrToken, sizeof(char) * 64, sizeof(char), nstrLength, pfile);
		pstrToken[nstrLength] = '\0';

		if (!strcmp(pstrToken, "<Frame>:"))
		{
			pModel = new CModel();
			pModel->SetFileName(pstrFileName);

			fread_s(&nstrLength, sizeof(BYTE), sizeof(BYTE), 1, pfile);
			fread_s(pModel->m_pstrModelName, sizeof(char) * 64, sizeof(char), nstrLength, pfile);
		}
		else if (!strcmp(pstrToken, "<TransformMatrix>:"))
		{
			fread_s(&pModel->m_xmf4x4ToParent, sizeof(XMFLOAT4X4), sizeof(XMFLOAT4X4), 1, pfile);
		}
		else if (!strcmp(pstrToken, "<SkinningInfo>:"))
		{
			CSkinnedMesh *pSkinnedMesh = new CSkinnedMesh(pd3dDevice, pd3dCommandList);
			pSkinnedMesh->LoadSkinInfoFromFile(pd3dDevice, pd3dCommandList, pfile);
			pSkinnedMesh->CreateShaderVariables(pd3dDevice, pd3dCommandList);

			fread_s(&nstrLength, sizeof(BYTE), sizeof(BYTE), 1, pfile);
			fread_s(pstrToken, sizeof(char) * 64, sizeof(char), nstrLength, pfile);
			pstrToken[nstrLength] = '\0';

			pSkinnedMesh->LoadMeshFromFile(pd3dDevice, pd3dCommandList, pfile);
			CCubeMesh *pCubeMesh = new CCubeMesh(pd3dDevice, pd3dCommandList, pSkinnedMesh->GetCenter(), pSkinnedMesh->GetExtents());
			pModel->SetMesh(pSkinnedMesh, pCubeMesh, true);
		}
		else if (!strcmp(pstrToken, "<Mesh>:"))
		{
			CStandardMesh *pMesh = new CStandardMesh(pd3dDevice, pd3dCommandList);
			pMesh->LoadMeshFromFile(pd3dDevice, pd3dCommandList, pfile);
			CCubeMesh *pCubeMesh = new CCubeMesh(pd3dDevice, pd3dCommandList, pMesh->GetCenter(), pMesh->GetExtents());
			pModel->SetMesh(pMesh, pCubeMesh, false);
		}
		else if (!strcmp(pstrToken, "<Materials>:"))
		{
			fread_s(&pModel->m_nMaterials, sizeof(int), sizeof(int), 1, pfile);

			if (pModel->m_nMaterials > 0)
			{
				pModel->m_ppMaterials = new CMaterial*[pModel->m_nMaterials];

				for (int i = 0; i < pModel->m_nMaterials; i++)
				{
					fread_s(&nstrLength, sizeof(BYTE), sizeof(BYTE), 1, pfile);
					fread_s(pstrToken, sizeof(char) * 64, sizeof(char), nstrLength, pfile);
					pstrToken[nstrLength] = '\0';

					if (!strcmp(pstrToken, "<Material>:"))
					{
						pModel->m_ppMaterials[i] = new CMaterial();
						pModel->m_ppMaterials[i]->LoadMaterialFromFile(pd3dDevice, pd3dCommandList, pfile, pstrFilePath);
					}
				}
			}
		}
		else if (!strcmp(pstrToken, "<Children>:"))
		{
			int nChild = 0;
			fread_s(&nChild, sizeof(int), sizeof(int), 1, pfile);

			if (nChild > 0)
			{
				for (int i = 0; i < nChild; i++)
				{
					CModel *pChild = CModel::LoadModelFromFile(pd3dDevice, pd3dCommandList, pfile, pstrFileName, pstrFilePath);
					if (pChild) pModel->SetChild(pChild);
				}
			}

		}
		else if (!strcmp(pstrToken, "</Frame>"))
		{
			break;
		}
	}

	return pModel;
}

CAnimationSet* CModel::LoadAnimationFromFile(CModel *pModel, const char* pstrFileName)
{
	FILE *pfile;
	fopen_s(&pfile, pstrFileName, "rb");

	BYTE nstrLength;
	char pstrToken[64] = { 0 };

	CAnimationSet *pAnimationSet = NULL;

	while (true)
	{
		fread_s(&nstrLength, sizeof(BYTE), sizeof(BYTE), 1, pfile);
		fread_s(pstrToken, sizeof(char) * 64, sizeof(char), nstrLength, pfile);
		pstrToken[nstrLength] = '\0';

		if (!strcmp(pstrToken, "<AnimationSets>:"))
		{
			int nAnimations;
			fread_s(&nAnimations, sizeof(int), sizeof(int), 1, pfile);

			pAnimationSet = new CAnimationSet(nAnimations);
		}
		else if (!strcmp(pstrToken, "<FrameNames>:"))
		{
			int nFrames;
			fread_s(&nFrames, sizeof(int), sizeof(int), 1, pfile);
			pAnimationSet->SetAnimationFrames(nFrames);

			CModel **ppCacheModels = NULL;
			ppCacheModels = new CModel*[nFrames];

			for (int i = 0; i < nFrames; i++)
			{
				fread_s(&nstrLength, sizeof(BYTE), sizeof(BYTE), 1, pfile);
				fread_s(pstrToken, sizeof(char) * 64, sizeof(char), nstrLength, pfile);
				pstrToken[nstrLength] = '\0';

				ppCacheModels[i] = pModel->FindFrame(pstrToken);
			}

			pAnimationSet->SetAnimationFrameCaches(ppCacheModels);

			for (int i = 0; i < pAnimationSet->GetAnimationCount(); i++)
			{
				fread_s(&nstrLength, sizeof(BYTE), sizeof(BYTE), 1, pfile);
				fread_s(pstrToken, sizeof(char) * 64, sizeof(char), nstrLength, pfile); //<AnimationSet>:
				pstrToken[nstrLength] = '\0';

				pAnimationSet->GetAnimation(i)->LoadAnimationFromFile(pfile, pAnimationSet->GetAnimationFrames());
			}
		}
		else if (!strcmp(pstrToken, "</AnimationSets>"))
		{
			break;
		}
	}

	return pAnimationSet;
}

void CModel::CacheSkinningBoneFrames(CModel *pRootModel)
{
	if (m_pMesh && (m_pMesh->GetMeshType() & TYPE_SKINNED_MESH))
	{
		CSkinnedMesh *pSkinnedMesh = (CSkinnedMesh*)m_pMesh;
		for (int i = 0; i < pSkinnedMesh->m_nSkinningBones; i++)
			pSkinnedMesh->m_ppSkinningBoneFrameCaches[i] = pRootModel->FindFrame(pSkinnedMesh->m_ppstrSkinningBoneNames[i]);
	}

	if (m_pSibling) m_pSibling->CacheSkinningBoneFrames(pRootModel);
	if (m_pChild) m_pChild->CacheSkinningBoneFrames(pRootModel);
}