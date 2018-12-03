#pragma once
#include "GameObject.h"

class CScene
{
public:
	CScene();
	~CScene();

	void BuildObjects();
	void AnimateObjects(float fTimeElapsed);


protected:

	CGameObject					*m_pTerrain = NULL;
	CMesh**						m_ppBulletMesh = NULL;
	CMesh**						m_ppObstacleMesh = NULL;
	CCubeMesh**					m_ppBulletCubeMesh = NULL;
	CCubeMesh**					m_ppObstacleCubeMesh = NULL;

public:


public: // For Network
	CGameObject* m_pObjects[MAX_NUM_OBJECT];
	CGameObject* m_pObstacles[MAX_NUM_OBJECT];
	//CGameObject* m_pBulletObjects[MAX_NUM_OBJECT];
	void InsertObject(PKT_CREATE_OBJECT CreateObjectInfo);
	void DeleteObject(PKT_DELETE_OBJECT DeleteObjectInfo);
	int GetIndex();
	void AddObject(CGameObject objcet);
	void releaseObject(int index);
	CGameObject** GetObjects(OBJECT_TYPE type) 
	{
		//if (type == OBJECT_TYPE_OBSTACLE) return m_pObjects; 
		//else if (type == OBJECT_TYPE_BULLET) return m_pBulletObjects;
		//else return m_pPlayers;
		return m_pObjects;
	}
};
