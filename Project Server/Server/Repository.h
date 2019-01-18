#pragma once

#include"Model.h"

class CRepository
{
public:
	CRepository();
	virtual ~CRepository();

	CModel* GetModel(char *pstrName);

protected:
	std::vector<CModel*> m_vModels;
};

