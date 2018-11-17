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


public:
	// 충돌 체크를 검사한다.
	void CheckCollision();


public: // For Network
	CGameObject* m_pObjects[MAX_NUM_OBJECT];
	//CGameObject* m_pPlayers[MAX_NUM_OBJECT];
	//CGameObject* m_pBulletObjects[MAX_NUM_OBJECT];
	void InsertObject(PKT_CREATE_OBJECT CreateObjectInfo);
	void DeleteObject(PKT_DELETE_OBJECT DeleteObjectInfo);
	CGameObject** GetObjects(OBJECT_TYPE type) 
	{
		//if (type == OBJECT_TYPE_OBSTACLE) return m_pObjects; 
		//else if (type == OBJECT_TYPE_BULLET) return m_pBulletObjects;
		//else return m_pPlayers;
		return m_pObjects;
	}
};
