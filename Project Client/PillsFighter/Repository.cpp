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
			delete *Model;
			Model = m_vModels.erase(Model);
		}
	}
}

CModel* CRepository::GetModel(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, char *pstrName)
{
	for (const auto& Model : m_vModels)
	{
		if (Model->IsName(pstrName))
		{
			return Model;
		}
	}

	CModel *pModel = new CModel(pd3dDevice, pd3dCommandList, pstrName);
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