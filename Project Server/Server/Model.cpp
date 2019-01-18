#include "pch.h"
#include "Model.h"
#include "Mesh.h"
#include "fbxsdk.h"

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

CModel::CModel(char *pFileName)
{
	m_pstrName = pFileName;

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
		((CStandardMesh*)m_pMesh)->LoadMeshFromFBX(pFbxMesh);
		m_pCubeMesh = new CCubeMesh(m_pMesh->GetCenter(), m_pMesh->GetExtents());
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
}

void CModel::UpdateCollisionBox(BoundingBox &xmAABB, XMFLOAT4X4 &xmf4x4World)
{
	m_pMesh->m_xmAABB.Transform(xmAABB, XMLoadFloat4x4(&xmf4x4World));
}
