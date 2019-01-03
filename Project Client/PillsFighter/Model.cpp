#include "stdafx.h"
#include "Model.h"
#include "Mesh.h"
//#include "Shader.h"
#include "fbxsdk.h"

CTexture::CTexture(int nTextures, UINT nTextureType, int nSamplers)
{
	m_nTextureType = nTextureType;
	m_nTextures = nTextures;

	if (m_nTextures > 0)
	{
		m_pRootArgumentInfos = new SRVROOTARGUMENTINFO[m_nTextures];
		m_ppd3dTextureUploadBuffers = new ID3D12Resource*[m_nTextures];
		m_ppd3dTextures = new ID3D12Resource*[m_nTextures];
	}

	m_nSamplers = nSamplers;
	if (m_nSamplers > 0) m_pd3dSamplerGpuDescriptorHandles = new D3D12_GPU_DESCRIPTOR_HANDLE[m_nSamplers];
}

CTexture::~CTexture()
{
	if (m_ppd3dTextures)
	{
		for (int i = 0; i < m_nTextures; i++) if (m_ppd3dTextures[i]) m_ppd3dTextures[i]->Release();
		delete[] m_ppd3dTextures;
	}

	if (m_pRootArgumentInfos) delete[] m_pRootArgumentInfos;

	if (m_ppd3dTextureUploadBuffers)
	{
		for (int i = 0; i < m_nTextures; i++) if (m_ppd3dTextureUploadBuffers[i]) m_ppd3dTextureUploadBuffers[i]->Release();
		delete[] m_ppd3dTextureUploadBuffers;
		m_ppd3dTextureUploadBuffers = NULL;
	}

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

void CTexture::LoadTextureFromFile(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, const wchar_t *pszFileName, UINT nIndex)
{
	m_ppd3dTextures[nIndex] = ::CreateTextureResourceFromFile(pd3dDevice, pd3dCommandList, pszFileName, &m_ppd3dTextureUploadBuffers[nIndex], D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
}

ID3D12Resource *CTexture::CreateTexture(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, UINT nWidth, UINT nHeight, DXGI_FORMAT dxgiFormat, D3D12_RESOURCE_FLAGS d3dResourceFlags, D3D12_RESOURCE_STATES d3dResourceStates, D3D12_CLEAR_VALUE *pd3dClearValue, UINT nIndex)
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
	for (auto& Texture : m_vpTextures) 
		if (Texture) 
			delete Texture;

	m_vpTextures.empty();
}

void CMaterial::SetTexture(CTexture *pTexture)
{
	m_vpTextures.emplace_back(pTexture);
}

void CMaterial::UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList)
{
	for (auto& Texture : m_vpTextures)
	{
		if (Texture) Texture->UpdateShaderVariable(pd3dCommandList, 0);
	}
}

void CMaterial::ReleaseUploadBuffers()
{
	for (auto& Texture : m_vpTextures)
	{
		if (Texture) 
			Texture->ReleaseUploadBuffers();
	}
}

void CMaterial::LoadMaterialFromFBX(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, FbxSurfaceMaterial *pfbxMaterial, CShader *pShader, const char *pstrFilePath)
{
	if (pfbxMaterial->GetClassId().Is(FbxSurfacePhong::ClassId))
	{
		std::cout << "Is Phong\n";

		FbxDouble3 fbxd3Ambient = ((FbxSurfacePhong*)pfbxMaterial)->Ambient;
		m_xmf4AmbientColor.x = fbxd3Ambient.mData[0];
		m_xmf4AmbientColor.y = fbxd3Ambient.mData[1];
		m_xmf4AmbientColor.z = fbxd3Ambient.mData[2];

		FbxDouble3 fbxd3Diffuse = ((FbxSurfacePhong*)pfbxMaterial)->Diffuse;
		m_xmf4DiffuseColor.x = fbxd3Diffuse.mData[0];
		m_xmf4DiffuseColor.y = fbxd3Diffuse.mData[1];
		m_xmf4DiffuseColor.z = fbxd3Diffuse.mData[2];

		FbxDouble3 fbxd3Specular = ((FbxSurfacePhong*)pfbxMaterial)->Specular;
		m_xmf4SpecularColor.x = fbxd3Specular.mData[0];
		m_xmf4SpecularColor.y = fbxd3Specular.mData[1];
		m_xmf4SpecularColor.z = fbxd3Specular.mData[2];

		FbxDouble3 fbxd3Emissive = ((FbxSurfacePhong*)pfbxMaterial)->Emissive;
		m_xmf4EmissiveColor.x = fbxd3Emissive.mData[0];
		m_xmf4EmissiveColor.y = fbxd3Emissive.mData[1];
		m_xmf4EmissiveColor.z = fbxd3Emissive.mData[2];

		//FbxDouble3 fbxd3Reflection = ((FbxSurfacePhong*)pfbxMaterial)->Reflection;
		//printf("Reflection Color : %f, %f, %f\n",
		//	fbxd3Reflection.mData[0],
		//	fbxd3Reflection.mData[1],
		//	fbxd3Reflection.mData[2]);

		//// 재질 투명도
		//FbxDouble fbxd3TransparencyFactor = ((FbxSurfacePhong*)pfbxMaterial)->TransparencyFactor;
		//printf("Transparency Factor : %f\n",
		//	fbxd3TransparencyFactor);

		//// 반짝임
		//FbxDouble fbxd3Shininess = ((FbxSurfacePhong*)pfbxMaterial)->Shininess;
		//printf("Shininess : %f\n",
		//	fbxd3Shininess);

		FbxDouble fbxd3SpecularFactor = ((FbxSurfacePhong*)pfbxMaterial)->SpecularFactor;
		m_xmf4SpecularColor.w = fbxd3SpecularFactor;

		//FbxDouble fbxd3ReflectionFactor = ((FbxSurfacePhong*)pfbxMaterial)->ReflectionFactor;
		//printf("Reflection Factor : %f\n",
			//	fbxd3ReflectionFactor);
	}
	else if (pfbxMaterial->GetClassId().Is(FbxSurfaceLambert::ClassId))
	{
		std::cout << "Is Lambert\n";

		FbxDouble3 fbxd3Ambient = ((FbxSurfaceLambert*)pfbxMaterial)->Ambient;
		m_xmf4AmbientColor.x = fbxd3Ambient.mData[0];
		m_xmf4AmbientColor.y = fbxd3Ambient.mData[1];
		m_xmf4AmbientColor.z = fbxd3Ambient.mData[2];

		//FbxDouble3 fbxd3Diffuse = ((FbxSurfaceLambert*)pfbxMaterial)->Diffuse;
		//printf("Diffuse Color : %f, %f, %f\n",
		//	fbxd3Diffuse.mData[0],
		//	fbxd3Diffuse.mData[1],
		//	fbxd3Diffuse.mData[2]);

		FbxDouble3 fbxd3Emissive = ((FbxSurfaceLambert*)pfbxMaterial)->Emissive;
		m_xmf4EmissiveColor.x = fbxd3Emissive.mData[0];
		m_xmf4EmissiveColor.y = fbxd3Emissive.mData[1];
		m_xmf4EmissiveColor.z = fbxd3Emissive.mData[2];

		//// 재질 투명도
		//FbxDouble fbxd3TransparencyFactor = ((FbxSurfaceLambert*)pfbxMaterial)->TransparencyFactor;
		//printf("Transparency Factor : %f\n",
		//	fbxd3TransparencyFactor);
	}

	int j;

	FBXSDK_FOR_EACH_TEXTURE(j)
	{
		auto fbxProperty = pfbxMaterial->FindProperty(FbxLayerElement::sTextureChannelNames[j]);

		if (fbxProperty.IsValid())
		{
			FbxLayeredTexture *pfbxLayeredTexture = fbxProperty.GetSrcObject<FbxLayeredTexture>(j);
			if (pfbxLayeredTexture)
			{
				std::cout << "Current Material is Layered\n";
			}
			else
			{
				FbxTexture *pfbxTexture = fbxProperty.GetSrcObject<FbxTexture>(j);
				if (pfbxTexture)
				{
					FbxFileTexture *pFbxFileTexture = FbxCast<FbxFileTexture>(pfbxTexture);
					std::string strFileName = GetFileName(pFbxFileTexture->GetFileName());

					if (pFbxFileTexture)
					{
						std::string strTextureType = fbxProperty.GetNameAsCStr();
						char strFile[256];
						strcpy(strFile, pstrFilePath);
						strcat_s(strFile, "Texture\\");
						strcat_s(strFile, strFileName.c_str());
						strcat_s(strFile, ".dds");

						INT nLen = strlen(strFile) + 1;
						WCHAR* pstrFileName = (LPWSTR)new WCHAR[sizeof(WCHAR)*nLen];
						MultiByteToWideChar(949, 0, strFileName.c_str(), -1, pstrFileName, nLen);

						CTexture *pTexture = new CTexture(1, RESOURCE_TEXTURE2D, 0);

						if (strTextureType == "DiffuseColor")
						{
							m_nType |= MATERIAL_DIFFUSE_MAP;

							printf("Is Diffuse Map : %s\n", strFileName.c_str());
						}
						else if (strTextureType == "SpecularColor")
						{
							m_nType |= MATERIAL_SPECULAR_MAP;

							printf("Is Specular Map : %s\n", strFileName.c_str());
						}
						else if (strTextureType == "Bump")
						{
							m_nType |= MATERIAL_NORMAL_MAP;

							printf("Is Bump Map : %s\n", strFileName.c_str());
						}
						else
						{
							printf("Do Not Support This Texture Type");
							delete pTexture;

							continue;
						}
						pTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, pstrFileName, 0);

						m_vpTextures.emplace_back(pTexture);
					}
				}
			}
		}
	}
}

void CMaterial::CreateShaderResourceViewsInMaterial(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, CShader *pShader)
{
	for (auto& Texture : m_vpTextures)
	{
		//UINT nType = Texture->GetType();

		//if (nType == DIFFUSE_MAP)
		{
			//pShader->CreateShaderResourceViews(pd3dDevice, pd3dCommandList, Texture, ROOT_PARAMETER_INDEX_DIFFUSE_TEXTURE_ARRAY, false);
		}
		//else if (nType == SPECULAR_MAP)
		{
			//pShader->CreateShaderResourceViews(pd3dDevice, pd3dCommandList, Texture, ROOT_PARAMETER_INDEX_SPECULAR_TEXTURE, false);
		}
		//else if (nType == NORMAL_MAP)
		{
			//pShader->CreateShaderResourceViews(pd3dDevice, pd3dCommandList, Texture, ROOT_PARAMETER_INDEX_NORMAL_TEXTURE, false);
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string GetFileName(const std::string strFileName)
{
	std::string strPathSeparator("\\");
	std::string strSuffixSeparator(".");

	int nPathpos = 1 + strFileName.find_last_of(strPathSeparator);
	if (nPathpos == 0)
	{
		strPathSeparator = "/";
		nPathpos = 1 + strFileName.find_last_of(strPathSeparator);
	}

	int nSuffixpos = strFileName.find_last_of(strSuffixSeparator);

	return strFileName.substr(nPathpos, nSuffixpos - nPathpos);
}

std::string GetFilePath(const std::string strFileName)
{
	std::string strPathSeparator("\\");

	int nPathpos = 1 + strFileName.find_last_of(strPathSeparator);
	if (nPathpos == 0)
	{
		strPathSeparator = "/";
		nPathpos = 1 + strFileName.find_last_of(strPathSeparator);
	}
	std::cout << nPathpos << std::endl;

	return strFileName.substr(0, nPathpos);
}

FbxNode* GetMeshNode(FbxNode* pfbxNode)
{
	auto fbxNodeAttribute = pfbxNode->GetNodeAttribute();

	if (fbxNodeAttribute)
	{
		if (fbxNodeAttribute->GetAttributeType() == FbxNodeAttribute::eMesh)
		{
			return pfbxNode;
		}
	}

	for (int i = 0; i < pfbxNode->GetChildCount(); i++)
	{
		auto pNode = GetMeshNode(pfbxNode->GetChild(i));

		if (pNode)
			return pNode;
	}

	return NULL;
}

CModel::CModel(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature, const char *pFileName, CShader *pShader)
{
	FbxManager *pFbxManager = FbxManager::Create();
	pFbxManager->SetIOSettings(FbxIOSettings::Create(pFbxManager, "IOSetting"));

	FbxScene *pFbxScene = FbxScene::Create(pFbxManager, "Scene");
	FbxImporter *pFbxImporter = FbxImporter::Create(pFbxManager, "Importer");

	pFbxImporter->Initialize(pFileName, -1, pFbxManager->GetIOSettings());
	pFbxImporter->Import(pFbxScene);
	pFbxImporter->Destroy();

	const char *pstrFilePath = GetFilePath(pFileName).c_str();

	// Find Mesh Node
	FbxNode *pfbxNode = GetMeshNode(pFbxScene->GetRootNode());

	auto pfbxNodeAttribute = pfbxNode->GetNodeAttribute();
	auto fbxAttributeType = pfbxNodeAttribute->GetAttributeType();

	switch (fbxAttributeType)
	{
	case FbxNodeAttribute::eMesh:
	{
		FbxMesh *pFbxMesh = pfbxNode->GetMesh();
		m_pMesh = new CStandardMesh();
		//m_pMesh->LoadMeshFromFBX(pd3dDevice, pd3dCommandList, pFbxMesh);

		// 재질 임포트[ 하나의 재질 ]
		int nMaterialCount = m_nMaterials = pfbxNode->GetMaterialCount();

		m_ppMaterials = new CMaterial*[m_nMaterials];

		int nTextureCount = 0;

		for (int i = 0; i < m_nMaterials; i++)
		{
			auto pfbxMaterial = pfbxNode->GetMaterial(i);
			std::cout << "Material Count : " << i << "\n";

			m_ppMaterials[i] = new CMaterial();
			m_ppMaterials[i]->LoadMaterialFromFBX(pd3dDevice, pd3dCommandList, pfbxMaterial, pShader, pstrFilePath);

			nTextureCount += m_ppMaterials[i]->GetTextureCount();
		}

		//pShader->CreateDescriptorHeaps(pd3dDevice, pd3dCommandList, nTextureCount);

		for (int i = 0; i < m_nMaterials; i++)
		{

		}
	}
	}
}


CModel::~CModel()
{
}