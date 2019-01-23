#include "stdafx.h"
#include "Model.h"
#include "Mesh.h"
#include "Shader.h"
#include "fbxsdk.h"
#include "Scene.h"

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

std::string GetFileName(const std::string strFileName)
{
	std::string strPathSeparator("\\");
	std::string strSuffixSeparator(".");

	int nPathpos = 1 + (int)(strFileName.find_last_of(strPathSeparator));
	if (nPathpos == 0)
	{
		strPathSeparator = "/";
		nPathpos = 1 + (int)(strFileName.find_last_of(strPathSeparator));
	}

	int nSuffixpos = (int)(strFileName.find_last_of(strSuffixSeparator));

	return strFileName.substr(nPathpos, nSuffixpos - nPathpos);
}

std::string GetFilePath(const std::string strFileName)
{
	std::string strPathSeparator("\\");

	int nPathpos = 1 + (int)(strFileName.find_last_of(strPathSeparator));
	if (nPathpos == 0)
	{
		strPathSeparator = "/";
		nPathpos = 1 + (int)(strFileName.find_last_of(strPathSeparator));
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
	pcbMappedGameObject->m_Material.m_xmf4Diffuse = m_xmf4AlbedoColor;
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

void CMaterial::LoadMaterialFromFBX(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, FbxSurfaceMaterial *pfbxMaterial, const char *pstrFilePath)
{
	if (pfbxMaterial->GetClassId().Is(FbxSurfacePhong::ClassId))
	{

		FbxDouble3 fbxd3Ambient = ((FbxSurfacePhong*)pfbxMaterial)->Ambient;
		m_xmf4AmbientColor.x = static_cast<float>(fbxd3Ambient.mData[0]);
		m_xmf4AmbientColor.y = static_cast<float>(fbxd3Ambient.mData[1]);
		m_xmf4AmbientColor.z = static_cast<float>(fbxd3Ambient.mData[2]);
		printf("Ambient(%f, %f, %f)\n", m_xmf4AmbientColor.x, m_xmf4AmbientColor.y, m_xmf4AmbientColor.z);

		FbxDouble3 fbxd3Diffuse = ((FbxSurfacePhong*)pfbxMaterial)->Diffuse;
		m_xmf4AlbedoColor.x = static_cast<float>(fbxd3Diffuse.mData[0]);
		m_xmf4AlbedoColor.y = static_cast<float>(fbxd3Diffuse.mData[1]);
		m_xmf4AlbedoColor.z = static_cast<float>(fbxd3Diffuse.mData[2]);
		printf("Diffuse(%f, %f, %f)\n", m_xmf4AlbedoColor.x, m_xmf4AlbedoColor.y, m_xmf4AlbedoColor.z);

		FbxDouble3 fbxd3Specular = ((FbxSurfacePhong*)pfbxMaterial)->Specular;
		m_xmf4SpecularColor.x = static_cast<float>(fbxd3Specular.mData[0]);
		m_xmf4SpecularColor.y = static_cast<float>(fbxd3Specular.mData[1]);
		m_xmf4SpecularColor.z = static_cast<float>(fbxd3Specular.mData[2]);

		FbxDouble fbxd3SpecularFactor = ((FbxSurfacePhong*)pfbxMaterial)->SpecularFactor;
		m_xmf4SpecularColor.w = static_cast<float>(fbxd3SpecularFactor);

		printf("Specular(%f, %f, %f, %f)\n", m_xmf4SpecularColor.x, m_xmf4SpecularColor.y, m_xmf4SpecularColor.z, m_xmf4SpecularColor.w);

		FbxDouble3 fbxd3Emissive = ((FbxSurfacePhong*)pfbxMaterial)->Emissive;
		m_xmf4EmissiveColor.x = static_cast<float>(fbxd3Emissive.mData[0]);
		m_xmf4EmissiveColor.y = static_cast<float>(fbxd3Emissive.mData[1]);
		m_xmf4EmissiveColor.z = static_cast<float>(fbxd3Emissive.mData[2]);
		printf("Emissive(%f, %f, %f)\n", m_xmf4EmissiveColor.x, m_xmf4EmissiveColor.y, m_xmf4EmissiveColor.z);

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

		//FbxDouble fbxd3ReflectionFactor = ((FbxSurfacePhong*)pfbxMaterial)->ReflectionFactor;
		//printf("Reflection Factor : %f\n",
			//	fbxd3ReflectionFactor);
	}
	else if (pfbxMaterial->GetClassId().Is(FbxSurfaceLambert::ClassId))
	{
		std::cout << "Is Lambert\n";

		FbxDouble3 fbxd3Ambient = ((FbxSurfaceLambert*)pfbxMaterial)->Ambient;
		m_xmf4AmbientColor.x = static_cast<float>(fbxd3Ambient.mData[0]);
		m_xmf4AmbientColor.y = static_cast<float>(fbxd3Ambient.mData[1]);
		m_xmf4AmbientColor.z = static_cast<float>(fbxd3Ambient.mData[2]);
		printf("Ambient(%f, %f, %f)\n", m_xmf4AmbientColor.x, m_xmf4AmbientColor.y, m_xmf4AmbientColor.z);

		//FbxDouble3 fbxd3Diffuse = ((FbxSurfaceLambert*)pfbxMaterial)->Diffuse;
		//printf("Diffuse Color : %f, %f, %f\n",
		//	fbxd3Diffuse.mData[0],
		//	fbxd3Diffuse.mData[1],
		//	fbxd3Diffuse.mData[2]);

		FbxDouble3 fbxd3Emissive = ((FbxSurfaceLambert*)pfbxMaterial)->Emissive;
		m_xmf4EmissiveColor.x = static_cast<float>(fbxd3Emissive.mData[0]);
		m_xmf4EmissiveColor.y = static_cast<float>(fbxd3Emissive.mData[1]);
		m_xmf4EmissiveColor.z = static_cast<float>(fbxd3Emissive.mData[2]);
		printf("Emissive(%f, %f, %f)\n", m_xmf4EmissiveColor.x, m_xmf4EmissiveColor.y, m_xmf4EmissiveColor.z);

		//// 재질 투명도
		//FbxDouble fbxd3TransparencyFactor = ((FbxSurfaceLambert*)pfbxMaterial)->TransparencyFactor;
		//printf("Transparency Factor : %f\n",
		//	fbxd3TransparencyFactor);
	}

	int nTextureIndex;

	FBXSDK_FOR_EACH_TEXTURE(nTextureIndex)
	{
		FbxProperty fbxProperty = pfbxMaterial->FindProperty(FbxLayerElement::sTextureChannelNames[nTextureIndex]);
		
		if (fbxProperty.IsValid())
		{
			int nTextureCount = fbxProperty.GetSrcObjectCount<FbxTexture>();

			for (int i = 0; i < nTextureCount; i++)
			{
				FbxLayeredTexture *pfbxLayeredTexture = fbxProperty.GetSrcObject<FbxLayeredTexture>(i);

				if (pfbxLayeredTexture)
				{
					std::cout << "Current Material is Layered\n";
				}
				else
				{
					FbxTexture *pfbxTexture = fbxProperty.GetSrcObject<FbxTexture>(i);

					if (pfbxTexture)
					{
						FbxFileTexture *pfbxFileTexture = fbxProperty.GetSrcObject<FbxFileTexture>(i);
						std::string strFileName = GetFileName(pfbxFileTexture->GetFileName());

						char strFile[256] = { 0 };
						strcpy_s(strFile, GetFilePath(pstrFilePath).c_str());
						strcat_s(strFile, strFileName.c_str());
						strcat_s(strFile, ".dds");

						INT nLen = (int)(strlen(strFile)) + 1;
						WCHAR* pstrFileName = (LPWSTR)new WCHAR[sizeof(WCHAR)*nLen];
						MultiByteToWideChar(949, 0, strFile, -1, pstrFileName, nLen);

						CTexture *pTexture = new CTexture(1, RESOURCE_TEXTURE2D, 0);

						const char *pstrName = fbxProperty.GetNameAsCStr();

						if (!strcmp(pstrName, "DiffuseColor"))
						{
							m_nType |= MATERIAL_ALBEDO_MAP;

							pTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, pstrFileName, 0);
							CScene::CreateShaderResourceViews(pd3dDevice, pTexture, ROOT_PARAMETER_INDEX_DIFFUSE_TEXTURE_ARRAY, false);
							printf("Is Diffuse Map : %s\n", strFileName.c_str());
						}
						else if (!strcmp(pstrName, "NormalMap"))
						{
							m_nType |= MATERIAL_NORMAL_MAP;

							pTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, pstrFileName, 0);
							CScene::CreateShaderResourceViews(pd3dDevice, pTexture, ROOT_PARAMETER_INDEX_NORMAL_TEXTURE, false);
							printf("Is Normal Map : %s\n", strFileName.c_str());
						}
						else if (!strcmp(pstrName, "SpecularMap"))
						{
							m_nType |= MATERIAL_SPECULAR_MAP;

							pTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, pstrFileName, 0);
							CScene::CreateShaderResourceViews(pd3dDevice, pTexture, ROOT_PARAMETER_INDEX_SPECULAR_TEXTURE, false);
							printf("Is Normal Map : %s\n", strFileName.c_str());
						}
						else
						{
							printf("Do Not Support This Type[%s]\n", pstrName);
							delete pTexture;

							continue;
						}

						m_vTextures.emplace_back(pTexture);
					}
				}
			}
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

	//m_pStandardShader = new CStandardShader();
	//m_pStandardShader->CreateShader(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
	//m_pStandardShader->CreateShaderVariables(pd3dDevice, pd3dCommandList);

	//m_pSkinnedAnimationShader = new CSkinnedAnimationShader();
	//m_pSkinnedAnimationShader->CreateShader(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
	//m_pSkinnedAnimationShader->CreateShaderVariables(pd3dDevice, pd3dCommandList);
}

void CMaterial::ReleaseShaders()
{
	if (m_pStandardShader) delete m_pStandardShader;

	//m_pStandardShader = new CStandardShader();
	//m_pStandardShader->CreateShader(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
	//m_pStandardShader->CreateShaderVariables(pd3dDevice, pd3dCommandList);

	//m_pSkinnedAnimationShader = new CSkinnedAnimationShader();
	//m_pSkinnedAnimationShader->CreateShader(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
	//m_pSkinnedAnimationShader->CreateShaderVariables(pd3dDevice, pd3dCommandList);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void DisplayMaterial(FbxNode* pfbxNode)
{
	printf("Node Name : %s\n", pfbxNode->GetName());

	int nMaterialCount = pfbxNode->GetMaterialCount();
	int nSurfaceMaterialCount = pfbxNode->GetSrcObjectCount<FbxSurfaceMaterial>();

	printf("Node Surface Material Count : %d\n", nSurfaceMaterialCount);

	for (int i = 0; i < nSurfaceMaterialCount; i++)
	{
		FbxSurfaceMaterial *pfbxMaterial = pfbxNode->GetSrcObject<FbxSurfaceMaterial>(i);

		printf("\tMaterial Name : %s\n", pfbxMaterial->GetName());

		int nTextureIndex;
		FBXSDK_FOR_EACH_TEXTURE(nTextureIndex)
		{
			FbxProperty fbxProperty = pfbxMaterial->FindProperty(FbxLayerElement::sTextureChannelNames[nTextureIndex]);

			if (fbxProperty.IsValid())
			{
				int nTextureCount = fbxProperty.GetSrcObjectCount<FbxTexture>();

				for (int j = 0; j < nTextureCount; j++)
				{
					FbxLayeredTexture *pfbxLayeredTexture = fbxProperty.GetSrcObject<FbxLayeredTexture>(j);

					if (pfbxLayeredTexture)
					{
						printf("\t\t[%d]Material's Texture is Layered\n", i);

						int nTextureCount2 = pfbxLayeredTexture->GetSrcObjectCount<FbxTexture>();

						for (int k = 0; k < nTextureCount2; k++)
						{
							FbxTexture *pfbxTexture = pfbxLayeredTexture->GetSrcObject<FbxTexture>(k);

							if (pfbxTexture)
							{
								FbxFileTexture *pfbxFileTexture = pfbxLayeredTexture->GetSrcObject<FbxFileTexture>(k);
								const char *pstrName = fbxProperty.GetNameAsCStr();

								printf("\t\t\tTextures Connected to [%d]Material\n", i);
								printf("\t\t\tTexture Name : %s\n", pfbxTexture->GetName());
								printf("\t\t\tTexture File Name : %s\n", pfbxFileTexture->GetName());
								printf("\t\t\tTexture Type : %s\n", pstrName);
							}
						}
					}
					else
					{
						printf("\t\t[%d]Material's Texture is Not Layered\n", i);

						FbxTexture *pfbxTexture = fbxProperty.GetSrcObject<FbxTexture>(j);
						FbxFileTexture *pfbxFileTexture = fbxProperty.GetSrcObject<FbxFileTexture>(j);
						const char *pstrName = fbxProperty.GetNameAsCStr();

						if (pfbxTexture)
						{
							printf("\t\t\tTextures Connected to [%d]Material\n", i);
							printf("\t\t\tTexture Name : %s\n", pfbxTexture->GetName());
							printf("\t\t\tTexture File Name : %s\n", pfbxFileTexture->GetName());
							printf("\t\t\tTexture Type : %s\n", pstrName);
						}
					}
				
				}
			}
		}
	}

	int nChild = pfbxNode->GetChildCount();

	for (int i = 0; i < nChild; i++)
	{
		DisplayMaterial(pfbxNode->GetChild(i));
	}

	printf("\n");
}

CModel::CModel()
{
	m_xmf4x4ToParent = Matrix4x4::Identity();
}

CModel::CModel(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, char *pFileName)
{
	m_xmf4x4ToParent = Matrix4x4::Identity();

	m_pstrFileName = pFileName;

	FbxManager *pFbxManager = FbxManager::Create();
	FbxIOSettings *pFbxIOS = FbxIOSettings::Create(pFbxManager, "IOSetting");
	pFbxManager->SetIOSettings(pFbxIOS);

	FbxScene *pFbxScene = FbxScene::Create(pFbxManager, "Scene");
	FbxImporter *pFbxImporter = FbxImporter::Create(pFbxManager, "Importer");

	pFbxImporter->Initialize(pFileName, -1, pFbxManager->GetIOSettings());
	pFbxImporter->Import(pFbxScene);
	pFbxImporter->Destroy();

	//DisplayMaterial(pFbxScene->GetRootNode());

	// Find Mesh Node
	FbxNode *pfbxNode = GetMeshNode(pFbxScene->GetRootNode());
	pfbxNode = GetMeshNode(pfbxNode);
	
	auto pfbxNodeAttribute = pfbxNode->GetNodeAttribute();
	auto fbxAttributeType = pfbxNodeAttribute->GetAttributeType();

	switch (fbxAttributeType)
	{
	case FbxNodeAttribute::eMesh:
	{
		FbxMesh *pFbxMesh = pfbxNode->GetMesh();
		m_pMesh = new CStandardMesh();
		((CStandardMesh*)m_pMesh)->LoadMeshFromFBX(pd3dDevice, pd3dCommandList, pFbxMesh);
		m_pCubeMesh = new CCubeMesh(pd3dDevice, pd3dCommandList, m_pMesh->GetCenter(), m_pMesh->GetExtents());

		int nMaterialCount = m_nMaterials = pfbxNode->GetMaterialCount();

		m_ppMaterials = new CMaterial*[m_nMaterials];

		for (int i = 0; i < m_nMaterials; i++)
		{
			auto pfbxMaterial = pfbxNode->GetMaterial(i);

			m_ppMaterials[i] = new CMaterial();
			m_ppMaterials[i]->LoadMaterialFromFBX(pd3dDevice, pd3dCommandList, pfbxMaterial, pFileName);
			m_ppMaterials[i]->SetStandardShader();
		}
	}
	}

	pFbxScene->Destroy();
	pFbxIOS->Destroy();
	pFbxManager->Destroy();
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
}

void CModel::UpdateWorldTransform(XMFLOAT4X4 *pxmf4x4World)
{
	m_xmf4x4World = (pxmf4x4World) ? Matrix4x4::Multiply(m_xmf4x4ToParent, *pxmf4x4World) : m_xmf4x4ToParent;
}


void CModel::UpdateCollisionBox(BoundingBox *pxmAABB)
{
	if (m_pMesh)
	{
		m_pMesh->m_xmAABB.Transform(*pxmAABB, XMLoadFloat4x4(&m_xmf4x4World));
	}
}

void CModel::RenderWire(ID3D12GraphicsCommandList *pd3dCommandList, CCamera* pCamera)
{
	if (m_pCubeMesh) m_pCubeMesh->Render(pd3dCommandList);
}

void CModel::Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera* pCamera, CB_GAMEOBJECT_INFO* pcbMappedGameObject)
{
	if (m_nMaterials > 0)
	{
		for (int i = 0; i < m_nMaterials; i++)
		{
			if (m_ppMaterials[i])
			{
				if (m_ppMaterials[i]->m_pShader) m_ppMaterials[i]->m_pShader->Render(pd3dCommandList, pCamera);
				m_ppMaterials[i]->UpdateShaderVariable(pd3dCommandList, pcbMappedGameObject);
				m_ppMaterials[i]->UpdateTextureShaderVariable(pd3dCommandList);
			}

			if (m_pMesh) m_pMesh->Render(pd3dCommandList);
		}
	}
}
