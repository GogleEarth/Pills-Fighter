#include "stdafx.h"
#include "Repository.h"

CRepository::CRepository()
{
}

CRepository::~CRepository()
{
	if (m_vModels.size())
	{
		for (auto& Model = m_vModels.begin(); Model != m_vModels.end();)
		{
			(*Model)->Release();
			Model = m_vModels.erase(Model);
		}
	}
}

CModel* CRepository::GetModel(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, char *pstrFileName, char *pstrUpperAniFileName, char *pstrUnderAniFileName)
{
	for (const auto& Model : m_vModels)
	{
		if (!strcmp(Model->GetFileName(), pstrFileName))
		{
			return Model;
		}
	}

	CModel *pModel = CModel::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pstrFileName, pstrUpperAniFileName, pstrUnderAniFileName);
	pModel->AddRef();
	m_vModels.emplace_back(pModel);
	
	return pModel;
}

void CRepository::ReleaseUploadBuffers()
{
	for (const auto& Model : m_vModels)
	{
		Model->ReleaseUploadBuffers();
	}
}