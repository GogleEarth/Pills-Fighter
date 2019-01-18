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
			delete *Model;
			Model = m_vModels.erase(Model);
		}
	}
}

CModel* CRepository::GetModel(char *pstrName)
{
	for (const auto& Model : m_vModels)
	{
		if (Model->IsName(pstrName))
		{
			return Model;
		}
	}

	CModel *pModel = new CModel(pstrName);
	m_vModels.emplace_back(pModel);

	return pModel;
}