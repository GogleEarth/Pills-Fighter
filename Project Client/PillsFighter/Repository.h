#pragma once

#include"GameObject.h"

struct GAMEOBJECTINFO
{
	char m_pstrFileName[64] = "\0";
	CGameObject *m_pObject = NULL;
};

class CRepository
{
public:
	CRepository();
	virtual ~CRepository();

	GAMEOBJECTINFO *CreateObjectInfo(char *pstrFileName);
	GAMEOBJECTINFO *GetObjectInfo(char *pstrFileName);

	void ReleaseUploadBuffers();
	void ReleaseObjects();

protected:
	std::vector<GAMEOBJECTINFO> m_vObjectsInfo;
};

