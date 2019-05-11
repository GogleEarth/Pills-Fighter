#pragma once
#include "GameObject.h"
#include "Repository.h"

class CScene
{
public:
	CScene();
	~CScene();

	void BuildObjects(CRepository* pRepository);
	void AnimateObjects(float fTimeElapsed);


protected:

	CGameObject					*m_pTerrain = NULL;
	CModel*						m_pBulletMesh = NULL;
	CModel*						m_pObstacleMesh = NULL;
	CModel*						m_pGMMesh = NULL;
	std::vector<CModel*>		m_vpModels;
public:
	void InsertObjectFromLoadInfFromBin(char *pstrFileName, int nGroup);

	float ReadFloatFromFile(FILE *pInFile) {
		float fValue = 0;
		UINT nReads = (UINT)::fread(&fValue, sizeof(float), 1, pInFile);
		return(fValue);
	}
	BYTE LeftByteFromFile(FILE *pInFile, int byte) {
		UINT nReads = 0;
		char waste[64] = { '\0' };
		nReads = (UINT)::fread(waste, sizeof(char), byte, pInFile);

		return(nReads);
	}
	BYTE ReadPosrotFromFile(FILE *pInFile, char *pstrToken) {
		BYTE nStrLength = 41;
		BYTE m_value = 7;
		UINT nReads = 0;
		nReads = (UINT)::fread(pstrToken, sizeof(char), nStrLength, pInFile);
		nReads = (UINT)::fread(pstrToken, sizeof(char), m_value, pInFile);
		pstrToken[m_value] = '\0';

		return(nReads);
	}

public: // For Network
	CGameObject* m_pObjects[MAX_NUM_OBJECT];
	std::vector<CGameObject*> Obstacles;
	CGameObject m_BeamsaberCollisionmesh[24];
	//CGameObject* m_pBulletObjects[MAX_NUM_OBJECT];
	void InsertObject(CGameObject* pObject, int nGroup, bool bPrepareRotate, void *pContext);
	void InsertObject(PKT_CREATE_OBJECT CreateObjectInfo);
	void DeleteObject(PKT_DELETE_OBJECT DeleteObjectInfo);
	int GetIndex();
	void AddObject(CGameObject* objcet);
	void releaseObject(int index);
	CGameObject** GetObjects(OBJECT_TYPE type) 
	{
		//if (type == OBJECT_TYPE_OBSTACLE) return m_pObjects; 
		//else if (type == OBJECT_TYPE_BULLET) return m_pBulletObjects;
		//else return m_pPlayers;
		return m_pObjects;
	}
};
