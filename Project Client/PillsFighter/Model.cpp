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
	}

	if (m_pRootArgumentInfos) delete[] m_pRootArgumentInfos;

	if (m_pd3dSamplerGpuDescriptorHandles) delete[] m_pd3dSamplerGpuDescriptorHandles;
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
		for (int i = 0; i < m_nTextures; i++) if (m_ppd3dTextureUploadBuffers[i]) m_ppd3dTextureUploadBuffers[i]->Release();
		delete[] m_ppd3dTextureUploadBuffers;
		m_ppd3dTextureUploadBuffers = NULL;
	}
}

void CTexture::LoadTextureFromFile(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, wchar_t *pszFileName, UINT nIndex)
{
	m_ppd3dTextures[nIndex] = ::CreateTextureResourceFromFile(pd3dDevice, pd3dCommandList, pszFileName, &m_ppd3dTextureUploadBuffers[nIndex], D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
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

	if (m_pShader != m_pStandardShader && m_pShader != m_pSkinnedAnimationShader)
		delete m_pShader;
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
	pcbMappedGameObject->m_nTexturesMask = m_nType;
}

void CMaterial::UpdateTextureShaderVariable(ID3D12GraphicsCommandList *pd3dCommandList)
{
	for (auto& Texture : m_vTextures)
	{
		if (Texture) Texture->UpdateShaderVariable(pd3dCommandList, 0);
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

void CMaterial::LoadMaterialFromFile(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, FILE *pFile, const char *pstrFilePath)
{
	char pstrToken[64] = { 0 };

	while(true)
	{
		fscanf_s(pFile, "%s", pstrToken, (int)sizeof(pstrToken));

		if (!strcmp(pstrToken, "<AmbientColor>:"))
		{
			fscanf_s(pFile, "%f %f %f %f", &m_xmf4AmbientColor.x, &m_xmf4AmbientColor.y, &m_xmf4AmbientColor.z, &m_xmf4AmbientColor.w);
		}
		else if (!strcmp(pstrToken, "<DiffuseColor>:"))
		{
			fscanf_s(pFile, "%f %f %f %f", &m_xmf4DiffuseColor.x, &m_xmf4DiffuseColor.y, &m_xmf4DiffuseColor.z, &m_xmf4DiffuseColor.w);
		}
		else if (!strcmp(pstrToken, "<EmissiveColor>:"))
		{
			fscanf_s(pFile, "%f %f %f %f", &m_xmf4EmissiveColor.x, &m_xmf4EmissiveColor.y, &m_xmf4EmissiveColor.z, &m_xmf4EmissiveColor.w);
		}
		else if (!strcmp(pstrToken, "<ReflectionColor>:"))
		{
			fscanf_s(pFile, "%f %f %f %f", &m_xmf4ReflectionColor.x, &m_xmf4ReflectionColor.y, &m_xmf4ReflectionColor.z, &m_xmf4ReflectionColor.w);
		}
		else if (!strcmp(pstrToken, "<SpecularColor>:"))
		{
			fscanf_s(pFile, "%f %f %f %f", &m_xmf4SpecularColor.x, &m_xmf4SpecularColor.y, &m_xmf4SpecularColor.z, &m_xmf4SpecularColor.w);
		}
		else if (!strcmp(pstrToken, "<ReflectionFactor>:"))
		{
			fscanf_s(pFile, "%f", &m_xmf4ReflectionColor.w);
		}
		else if (!strcmp(pstrToken, "<Shininess>:"))
		{
			fscanf_s(pFile, "%f", &m_fShininess);
		}
		else if (!strcmp(pstrToken, "<SpecularFactor>:"))
		{
			fscanf_s(pFile, "%f", &m_xmf4SpecularColor.w);
		}
		else if (!strcmp(pstrToken, "<Transparency>:"))
		{
			fscanf_s(pFile, "%f", &m_fTransparency);
		}
		else if (!strcmp(pstrToken, "<DiffuseMap>:"))
		{
			char pstrFileName[64] = { 0 };
			fscanf_s(pFile, "%s", pstrFileName, (int)sizeof(pstrFileName));
			if (!strcmp(pstrFileName, "null")) continue;

			char pstrDiffuseFile[64] = { 0 };
			strcpy_s(pstrDiffuseFile, pstrFilePath);
			strcat_s(pstrDiffuseFile, pstrFileName);
			strcat_s(pstrDiffuseFile, ".dds");

			INT nLen = (int)(strlen(pstrDiffuseFile)) + 1;
			WCHAR* pwstrFileName = (LPWSTR)new WCHAR[sizeof(WCHAR)*nLen];
			MultiByteToWideChar(949, 0, pstrDiffuseFile, -1, pwstrFileName, nLen);

			CTexture *pTexture = new CTexture(1, RESOURCE_TEXTURE2D, 0);

			m_nType |= MATERIAL_ALBEDO_MAP;

			pTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, pwstrFileName, 0);
			CScene::CreateShaderResourceViews(pd3dDevice, pTexture, ROOT_PARAMETER_INDEX_DIFFUSE_TEXTURE_ARRAY, false);
			printf("Diffuse Map Name : [%s]\n", pstrFileName);
			m_vTextures.emplace_back(pTexture);
		}
		else if (!strcmp(pstrToken, "<NormalMap>:"))
		{
			char pstrFileName[64] = { 0 };
			fscanf_s(pFile, "%s", pstrFileName, (int)sizeof(pstrFileName));
			if (!strcmp(pstrFileName, "null")) continue;

			char pstrDiffuseFile[64] = { 0 };
			strcpy_s(pstrDiffuseFile, pstrFilePath);
			strcat_s(pstrDiffuseFile, pstrFileName);
			strcat_s(pstrDiffuseFile, ".dds");

			INT nLen = (int)(strlen(pstrDiffuseFile)) + 1;
			WCHAR* pwstrFileName = (LPWSTR)new WCHAR[sizeof(WCHAR)*nLen];
			MultiByteToWideChar(949, 0, pstrDiffuseFile, -1, pwstrFileName, nLen);

			CTexture *pTexture = new CTexture(1, RESOURCE_TEXTURE2D, 0);

			m_nType |= MATERIAL_NORMAL_MAP;

			pTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, pwstrFileName, 0);
			CScene::CreateShaderResourceViews(pd3dDevice, pTexture, ROOT_PARAMETER_INDEX_NORMAL_TEXTURE, false);
			printf("Normal Map Name : [%s]\n", pstrFileName);
			m_vTextures.emplace_back(pTexture);
		}
		else if (!strcmp(pstrToken, "<SpecularFactorMap>:"))
		{
			char pstrFileName[64] = { 0 };
			fscanf_s(pFile, "%s", pstrFileName, (int)sizeof(pstrFileName));
			if(!strcmp(pstrFileName, "null")) continue;

			char pstrDiffuseFile[64] = { 0 };
			strcpy_s(pstrDiffuseFile, pstrFilePath);
			strcat_s(pstrDiffuseFile, pstrFileName);
			strcat_s(pstrDiffuseFile, ".dds");

			INT nLen = (int)(strlen(pstrDiffuseFile)) + 1;
			WCHAR* pwstrFileName = (LPWSTR)new WCHAR[sizeof(WCHAR)*nLen];
			MultiByteToWideChar(949, 0, pstrDiffuseFile, -1, pwstrFileName, nLen);

			CTexture *pTexture = new CTexture(1, RESOURCE_TEXTURE2D, 0);

			m_nType |= MATERIAL_SPECULAR_FACTOR_MAP;

			pTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, pwstrFileName, 0);
			CScene::CreateShaderResourceViews(pd3dDevice, pTexture, ROOT_PARAMETER_INDEX_SPECULAR_TEXTURE, false);
			printf("Specular Factor Map Name : [%s]\n", pstrFileName);
			m_vTextures.emplace_back(pTexture);
		}
		else if (!strcmp(pstrToken, "<SpecularMap>:"))
		{
			char pstrFileName[64] = { 0 };
			fscanf_s(pFile, "%s", pstrFileName, (int)sizeof(pstrFileName));
			if (!strcmp(pstrFileName, "null")) continue;
		}
		else if (!strcmp(pstrToken, "</Material>"))
		{
			break;
		}
	}
}
CShader *CMaterial::m_pSkinnedAnimationShader = NULL;
CShader *CMaterial::m_pStandardShader = NULL;

void CMaterial::PrepareShaders(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature)
{
	m_pStandardShader = new CShader();
	m_pStandardShader->CreateShader(pd3dDevice, pd3dGraphicsRootSignature);
	m_pStandardShader->CreateShaderVariables(pd3dDevice, pd3dCommandList);

	m_pSkinnedAnimationShader = new CSkinnedAnimationShader();
	m_pSkinnedAnimationShader->CreateShader(pd3dDevice, pd3dGraphicsRootSignature);
	m_pSkinnedAnimationShader->CreateShaderVariables(pd3dDevice, pd3dCommandList);
}

void CMaterial::ReleaseShaders()
{
	if (m_pStandardShader) delete m_pStandardShader;
	if (m_pSkinnedAnimationShader) delete m_pSkinnedAnimationShader;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CModel::CModel()
{
	m_xmf4x4ToParent = Matrix4x4::Identity();
}

CModel::~CModel()
{
	if (m_pMesh)
	{
		delete m_pMesh;
		if (m_pCubeMesh) delete m_pCubeMesh;
	}

	if (m_ppMaterials)
	{
		for (int i = 0; i < m_nMaterials; i++)
			if (m_ppMaterials[i]) delete m_ppMaterials[i];
		delete[] m_ppMaterials;
	}

	if (m_pAnimationSet) delete m_pAnimationSet;
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
		else m_nStandardMeshes++;
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

void CModel::GetMeshes(int *pnStandardMeshes, int *pnSkinnedMeshes)
{
	(*pnStandardMeshes) += m_nStandardMeshes;
	(*pnSkinnedMeshes) += m_nSkinnedMeshes;

	if (m_pSibling) m_pSibling->GetMeshes(pnStandardMeshes, pnSkinnedMeshes);
	if (m_pChild) m_pChild->GetMeshes(pnStandardMeshes, pnSkinnedMeshes);
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

void CModel::UpdateCollisionBox(std::vector<BoundingBox> vxmAABB, int *pnIndex)
{
	if ((*pnIndex) == vxmAABB.size())
		return;

	if (m_pMesh)
	{
		m_pMesh->m_xmAABB.Transform(vxmAABB[*pnIndex], XMLoadFloat4x4(&m_xmf4x4World));
	}

	(*pnIndex)++;

	if (m_pSibling) m_pSibling->UpdateCollisionBox(vxmAABB, pnIndex);
	if (m_pChild) m_pChild->UpdateCollisionBox(vxmAABB, pnIndex);
}

void CModel::RenderWire(ID3D12GraphicsCommandList *pd3dCommandList, CCamera* pCamera)
{
	if (m_pCubeMesh) m_pCubeMesh->Render(pd3dCommandList, 0);

	if (m_pSibling) m_pSibling->RenderWire(pd3dCommandList, pCamera);
	if (m_pChild) m_pChild->RenderWire(pd3dCommandList, pCamera);
}

void CModel::UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList, ID3D12Resource* pd3dcbGameObject, CB_GAMEOBJECT_INFO* pcbMappedGameObject)
{
	D3D12_GPU_VIRTUAL_ADDRESS d3dGpuVirtualAddress = pd3dcbGameObject->GetGPUVirtualAddress();
	pd3dCommandList->SetGraphicsRootConstantBufferView(ROOT_PARAMETER_INDEX_OBJECT, d3dGpuVirtualAddress);

	XMStoreFloat4x4(&pcbMappedGameObject->m_xmf4x4World, XMMatrixTranspose(XMLoadFloat4x4(&m_xmf4x4World)));
}

void CModel::Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera* pCamera, std::vector<ID3D12Resource*>& vd3dcbGameObject, std::vector<CB_GAMEOBJECT_INFO*>& vcbMappedGameObject, int *pnIndex)
{
	if ((*pnIndex) == vd3dcbGameObject.size())
		return;

	if (m_pMesh)
	{
		if(m_nStandardMeshes > 0) UpdateShaderVariables(pd3dCommandList, vd3dcbGameObject[*pnIndex], vcbMappedGameObject[*pnIndex]);

		if (m_nMaterials > 0)
		{
			for (int i = 0; i < m_nMaterials; i++)
			{
				if (m_ppMaterials[i])
				{
					if (m_ppMaterials[i]->m_pShader) m_ppMaterials[i]->m_pShader->Render(pd3dCommandList, pCamera);
					m_ppMaterials[i]->UpdateShaderVariable(pd3dCommandList, vcbMappedGameObject[*pnIndex]);
					m_ppMaterials[i]->UpdateTextureShaderVariable(pd3dCommandList);
				}

				m_pMesh->Render(pd3dCommandList, i);
			}
		}

		(*pnIndex)++;
	}


	if (m_pSibling) m_pSibling->Render(pd3dCommandList, pCamera, vd3dcbGameObject, vcbMappedGameObject, pnIndex);
	if (m_pChild) m_pChild->Render(pd3dCommandList, pCamera, vd3dcbGameObject, vcbMappedGameObject, pnIndex);
}

CModel* CModel::LoadGeometryAndAnimationFromFile(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, char *pstrFileName, bool bHasAnimation)
{
	FILE *pFile;
	fopen_s(&pFile, pstrFileName, "rt");
	printf("File : %s\n", pstrFileName);

	std::string pstrFilePath = ::GetFilePath(pstrFileName);
	printf("File Path : %s\n", pstrFilePath.c_str());

	CModel *pModel = CModel::LoadModelFromFile(pd3dDevice, pd3dCommandList, pFile, pstrFileName, pstrFilePath.c_str());
	int nStandardMeshes = 0, nSkinnedMeshes = 0;
	pModel->GetMeshes(&nStandardMeshes, &nSkinnedMeshes);
	pModel->SetModelMeshCount(nStandardMeshes, nSkinnedMeshes);
	if (bHasAnimation) pModel->m_pAnimationSet = ::CModel::LoadAnimationFromFile(pFile, pModel);
	pModel->CacheSkinningBoneFrames(pModel);

	return pModel;
}

CModel* CModel::LoadModelFromFile(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, FILE *pFile, const char *pstrFileName, const char *pstrFilePath)
{
	CModel *pModel = NULL;

	while (true)
	{
		char pstrToken[64] = { 0 };
		fscanf_s(pFile, "%s", pstrToken, (int)sizeof(pstrToken));

		if (!strcmp(pstrToken, "<Frame>:"))
		{
			pModel = new CModel();
			pModel->SetFileName(pstrFileName);

			fscanf_s(pFile, "%s", pModel->m_pstrModelName, (int)sizeof(pModel->m_pstrModelName));
		}
		else if (!strcmp(pstrToken, "<TransformMatrix>:"))
		{
			fscanf_s(pFile, "%f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f",
				&pModel->m_xmf4x4ToParent._11, &pModel->m_xmf4x4ToParent._12, &pModel->m_xmf4x4ToParent._13, &pModel->m_xmf4x4ToParent._14,
				&pModel->m_xmf4x4ToParent._21, &pModel->m_xmf4x4ToParent._22, &pModel->m_xmf4x4ToParent._23, &pModel->m_xmf4x4ToParent._24,
				&pModel->m_xmf4x4ToParent._31, &pModel->m_xmf4x4ToParent._32, &pModel->m_xmf4x4ToParent._33, &pModel->m_xmf4x4ToParent._34,
				&pModel->m_xmf4x4ToParent._41, &pModel->m_xmf4x4ToParent._42, &pModel->m_xmf4x4ToParent._43, &pModel->m_xmf4x4ToParent._44);
		}
		else if (!strcmp(pstrToken, "<SkinningInfo>:"))
		{
			CSkinnedMesh *pSkinnedMesh = new CSkinnedMesh(pd3dDevice, pd3dCommandList);
			pSkinnedMesh->LoadSkinInfoFromFile(pd3dDevice, pd3dCommandList, pFile);
			pSkinnedMesh->CreateShaderVariables(pd3dDevice, pd3dCommandList);

			fscanf_s(pFile, "%s", pstrToken, (int)sizeof(pstrToken)); //<Mesh>:
			pSkinnedMesh->LoadMeshFromFile(pd3dDevice, pd3dCommandList, pFile);

			CCubeMesh *pCubeMesh = new CCubeMesh(pd3dDevice, pd3dCommandList, pSkinnedMesh->GetCenter(), pSkinnedMesh->GetExtents());
			pModel->SetMesh(pSkinnedMesh, pCubeMesh, true);
		}
		else if (!strcmp(pstrToken, "<Mesh>:"))
		{
			CStandardMesh *pMesh = new CStandardMesh(pd3dDevice, pd3dCommandList);
			pMesh->LoadMeshFromFile(pd3dDevice, pd3dCommandList, pFile);
			CCubeMesh *pCubeMesh = new CCubeMesh(pd3dDevice, pd3dCommandList, pMesh->GetCenter(), pMesh->GetExtents());
			pModel->SetMesh(pMesh, pCubeMesh, false);
		}
		else if (!strcmp(pstrToken, "<Materials>:"))
		{
			fscanf_s(pFile, "%d", &pModel->m_nMaterials);

			if (pModel->m_nMaterials > 0)
			{
				pModel->m_ppMaterials = new CMaterial*[pModel->m_nMaterials];

				for (int i = 0; i < pModel->m_nMaterials; i++)
				{
					fscanf_s(pFile, "%s", pstrToken, (int)sizeof(pstrToken));
					if (!strcmp(pstrToken, "<Material>:"))
					{
						int nIndex = 0;
						fscanf_s(pFile, "%d", &nIndex);

						pModel->m_ppMaterials[nIndex] = new CMaterial();
						pModel->m_ppMaterials[nIndex]->LoadMaterialFromFile(pd3dDevice, pd3dCommandList, pFile, pstrFilePath);

						int nMeshType = pModel->m_pMesh->GetMeshType();
						if (nMeshType & TYPE_STANDARD_MESH)
						{
							if (nMeshType & TYPE_SKINNED_MESH)
								pModel->m_ppMaterials[nIndex]->SetSkinnedAnimationShader();
							else
								pModel->m_ppMaterials[nIndex]->SetStandardShader();
						}
					}
				}
			}
		}
		else if (!strcmp(pstrToken, "<Children>:"))
		{
			int nChild = 0;
			fscanf_s(pFile, "%d", &nChild);

			if (nChild > 0)
			{
				for (int i = 0; i < nChild; i++)
				{
					CModel *pChild = CModel::LoadModelFromFile(pd3dDevice, pd3dCommandList, pFile, pstrFileName, pstrFilePath);
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

CAnimationSet* CModel::LoadAnimationFromFile(FILE *pFile, CModel *pModel)
{
	char pstrToken[64] = { 0 };

	CAnimationSet *pAnimationSet = NULL;

	while (true)
	{
		fscanf_s(pFile, "%s", pstrToken, (int)sizeof(pstrToken));

		if (!strcmp(pstrToken, "<AnimationSets>:"))
		{
			int nAnimations;
			fscanf_s(pFile, "%d", &nAnimations);

			pAnimationSet = new CAnimationSet(nAnimations);
		}
		else if (!strcmp(pstrToken, "<FrameNames>:"))
		{
			int nFrames;
			fscanf_s(pFile, "%d", &nFrames);
			pAnimationSet->SetAnimationFrames(nFrames);

			CModel **ppCacheModels = NULL;
			ppCacheModels = new CModel*[nFrames];
			for (int i = 0; i < nFrames; i++)
			{
				fscanf_s(pFile, "%s", pstrToken, (int)sizeof(pstrToken));
				ppCacheModels[i] = pModel->FindFrame(pstrToken);
			}

			pAnimationSet->SetAnimationFrameCaches(ppCacheModels);

			for (int i = 0; i < pAnimationSet->GetAnimationCount(); i++)
			{
				fscanf_s(pFile, "%s", pstrToken, (int)sizeof(pstrToken)); // <AnimationSet>:

				pAnimationSet->GetAnimation(i)->LoadAnimationFromFile(pFile, pAnimationSet->GetAnimationFrames());
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