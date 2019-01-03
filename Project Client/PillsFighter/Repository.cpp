#include "stdafx.h"
#include "Repository.h"


CRepository::CRepository()
{
}

CRepository::~CRepository()
{
}

GAMEOBJECTINFO* CRepository::CreateObjectInfo(char *pstrFileName)
{
	GAMEOBJECTINFO* pObjectInfo = GetObjectInfo(pstrFileName);

	if (!pObjectInfo)
	{
		GAMEOBJECTINFO objectInfo;
		strcpy(objectInfo.m_pstrFileName, pstrFileName);

		CGameObject *pObject = new CGameObject();
		// File Load

		////////////
		objectInfo.m_pObject = pObject;


		return &objectInfo;
	}
	else
		return pObjectInfo;

	return NULL;
}

GAMEOBJECTINFO* CRepository::GetObjectInfo(char *m_pstrFileName)
{
	for (auto& ObjectInfo : m_vObjectsInfo)
	{
		if (!strcmp(ObjectInfo.m_pstrFileName, m_pstrFileName))
			return &ObjectInfo;
	}

	return NULL;
}



void CRepository::ReleaseUploadBuffers()
{
	for (const auto& ObjectInfo : m_vObjectsInfo)
	{
		ObjectInfo.m_pObject->ReleaseUploadBuffers();
	}
}

void CRepository::ReleaseObjects()
{
	for (const auto& ObjectInfo : m_vObjectsInfo)
	{
		delete ObjectInfo.m_pObject;
	}
}