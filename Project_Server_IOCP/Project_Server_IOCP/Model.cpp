#include "pch.h"
#include "Model.h"
#include "Mesh.h"

///////////////////////////////////////////////////////////////////////////////
CModel::CModel()
{
	m_xmf4x4World = Matrix4x4::Identity();
}

CModel::~CModel()
{
	if (m_pMesh)
	{
		delete m_pMesh;
		if (m_pCubeMesh) delete m_pCubeMesh;
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

void CModel::SetChild(CModel * pChild, bool bAddReference)
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
		m_nMeshes++;
	}

	m_pMesh = pMesh;
	m_pCubeMesh = pCubeMesh;
}

void CModel::GetMeshes(int *pnMeshes, int *pnSkinnedMeshes)
{
	(*pnMeshes) += m_nMeshes;

	if (m_pSibling) m_pSibling->GetMeshes(pnMeshes, pnSkinnedMeshes);
	if (m_pChild) m_pChild->GetMeshes(pnMeshes, pnSkinnedMeshes);
}

void CModel::UpdateCollisionBox(std::vector<BoundingBox>& vxmAABB, int *pnIndex)
{
	if ((*pnIndex) == vxmAABB.size())
		return;

	if (m_pMesh) m_pMesh->m_xmAABB.Transform(vxmAABB[(*pnIndex)++], XMLoadFloat4x4(&m_xmf4x4World));

	if (m_pSibling) m_pSibling->UpdateCollisionBox(vxmAABB, pnIndex);
	if (m_pChild) m_pChild->UpdateCollisionBox(vxmAABB, pnIndex);
}

void CModel::UpdateWorldTransform(XMFLOAT4X4 *pxmf4x4Parent)
{
	m_xmf4x4World = (pxmf4x4Parent) ? Matrix4x4::Multiply(m_xmf4x4ToParent, *pxmf4x4Parent) : m_xmf4x4ToParent;

	if (m_pSibling) m_pSibling->UpdateWorldTransform(pxmf4x4Parent);
	if (m_pChild) m_pChild->UpdateWorldTransform(&m_xmf4x4World);
}

CModel* CModel::LoadGeometryAndAnimationFromFile(char *pstrFileName, char *pstrUpperAniFileName, char *pstrUnderAniFileName)
{
	FILE *pFile;
	fopen_s(&pFile, pstrFileName, "rb");
	printf("File : %s\n", pstrFileName);

	std::string pstrFilePath = ::GetFilePath(pstrFileName);
	printf("File Path : %s\n", pstrFilePath.c_str());

	CModel *pRootModel = CModel::LoadModelFromFile(pFile, pstrFileName, pstrFilePath.c_str());
	int nMeshes = 0, nSkinnedMeshes = 0;
	pRootModel->GetMeshes(&nMeshes, &nSkinnedMeshes);
	pRootModel->SetModelMeshCount(nMeshes, nSkinnedMeshes);

	return pRootModel;
}

CModel* CModel::LoadModelFromFile(FILE *pfile, const char *pstrFileName, const char *pstrFilePath)
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
		else if (!strcmp(pstrToken, "<Mesh>:"))
		{
			CStandardMesh *pMesh = new CStandardMesh();
			pMesh->LoadMeshFromFile(pfile);
			CCubeMesh *pCubeMesh = new CCubeMesh(pMesh->GetCenter(), pMesh->GetExtents());
			pModel->SetMesh(pMesh, pCubeMesh, false);
		}
		else if (!strcmp(pstrToken, "<Materials>:"))
		{
			fread_s(&pModel->m_nMaterials, sizeof(int), sizeof(int), 1, pfile);

			if (pModel->m_nMaterials > 0)
			{

				for (int i = 0; i < pModel->m_nMaterials; i++)
				{
					fread_s(&nstrLength, sizeof(BYTE), sizeof(BYTE), 1, pfile);
					fread_s(pstrToken, sizeof(char) * 64, sizeof(char), nstrLength, pfile);
					pstrToken[nstrLength] = '\0';

					if (!strcmp(pstrToken, "<Material>:"))
					{
						XMFLOAT4						m_xmf4DiffuseColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
						XMFLOAT4						m_xmf4EmissiveColor = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
						XMFLOAT4						m_xmf4SpecularColor = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
						XMFLOAT4						m_xmf4AmbientColor = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
						XMFLOAT4						m_xmf4ReflectionColor = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
						float							m_fShininess = 0.0f;
						float							m_fTransparency = 0.0f;

						float							m_fSmoothness = 0.0f;
						float							m_fSpecularHighlight = 0.0f;
						float							m_fMetallic = 0.0f;
						float							m_fGlossyReflection = 0.0f;
						while (true)
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

							}
							else if (!strcmp(pstrToken, "<NormalMap>:"))
							{
								char pstrFileName[64] = { 0 };
								fread_s(&nstrLength, sizeof(BYTE), sizeof(BYTE), 1, pfile);
								fread_s(pstrFileName, sizeof(char) * 64, sizeof(char), nstrLength, pfile);

								if (!strcmp(pstrFileName, "null")) continue;

							}
							else if (!strcmp(pstrToken, "<SpecularFactorMap>:"))
							{
								char pstrFileName[64] = { 0 };
								fread_s(&nstrLength, sizeof(BYTE), sizeof(BYTE), 1, pfile);
								fread_s(pstrFileName, sizeof(char) * 64, sizeof(char), nstrLength, pfile);

								if (!strcmp(pstrFileName, "null")) continue;

							}
							else if (!strcmp(pstrToken, "<SpecularMap>:"))
							{
								char pstrFileName[64] = { 0 };
								fread_s(&nstrLength, sizeof(BYTE), sizeof(BYTE), 1, pfile);
								fread_s(pstrFileName, sizeof(char) * 64, sizeof(char), nstrLength, pfile);

								if (!strcmp(pstrFileName, "null")) continue;
							}
							else if (!strcmp(pstrToken, "</Material>"))
							{
								break;
							}
						}
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
					CModel *pChild = CModel::LoadModelFromFile(pfile, pstrFileName, pstrFilePath);
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
