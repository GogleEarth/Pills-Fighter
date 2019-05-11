#include "pch.h"
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

CModel* CRepository::GetModel(char *pstrFileName, char *pstrUpperAniFileName, char *pstrUnderAniFileName)
{
	for (const auto& Model : m_vModels)
	{
		if (!strcmp(Model->GetFileName(), pstrFileName))
		{
			return Model;
		}
	}

	CModel *pModel = CModel::LoadGeometryAndAnimationFromFile(pstrFileName, pstrUpperAniFileName, pstrUnderAniFileName);
	pModel->AddRef();
	m_vModels.emplace_back(pModel);

	return pModel;
}