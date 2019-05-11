#pragma once

#include"Model.h"


class CRepository
{
public:
	CRepository();
	virtual ~CRepository();

	CModel* GetModel(char *pstrFileName, char *pstrUpperAniFileName, char *pstrUnderAniFileName);

protected:
	std::vector<CModel*> m_vModels;
};


