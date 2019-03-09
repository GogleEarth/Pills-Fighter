#include "pch.h"
#include "Scene.h"
#include "Model.h"

CScene::CScene()
{
	for (int i = 0; i < MAX_NUM_OBJECT; i++)
	{
		m_pObjects[i] = NULL;
		m_pObstacles[i] = NULL;
	}
}


CScene::~CScene()
{
}

void CScene::BuildObjects()
{
	m_pBulletMesh = new CModel("./Resource/Bullet/Bullet.FBX");
	m_pObstacleMesh = new CModel("./Resource/Hangar/Hangar.FBX");
	m_pGMMesh = new CModel("./Resource/GM/GM.FBX");

	for (int i = 0; i < MAX_CLIENT; ++i)
	{
		m_pObjects[i] = new CGameObject();
		m_pObjects[i]->m_Object_Type = OBJECT_TYPE_PLAYER;
		m_pObjects[i]->m_iId = i;
		m_pObjects[i]->SetPrepareRotate(-90.0f, 0.0f, 0.0f);
		m_pObjects[i]->SetModel(m_pGMMesh);
	}
	m_pObjects[0]->m_xmf4x4World = XMFLOAT4X4{ 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f , 0.0f, 0.0f, 1.0f, 0.0f , 0.0f, 0.0f, -150.0f, 1.0f };
	m_pObjects[1]->m_xmf4x4World = XMFLOAT4X4{ 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f , 0.0f, 0.0f, 1.0f, 0.0f , 0.0f, 0.0f, 150.0f, 1.0f };
	m_pObjects[2]->m_xmf4x4World = XMFLOAT4X4{ 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f , 0.0f, 0.0f, 1.0f, 0.0f , 100.0f, 0.0f, -150.0f, 1.0f };
	m_pObjects[3]->m_xmf4x4World = XMFLOAT4X4{ 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f , 0.0f, 0.0f, 1.0f, 0.0f , -100.0f, 0.0f, 150.0f, 1.0f };
	m_pObjects[4]->m_xmf4x4World = XMFLOAT4X4{ 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f , 0.0f, 0.0f, 1.0f, 0.0f , 0.0f, 0.0f, -200.0f, 1.0f };
	m_pObjects[5]->m_xmf4x4World = XMFLOAT4X4{ 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f , 0.0f, 0.0f, 1.0f, 0.0f , 0.0f, 0.0f, 200.0f, 1.0f };
	m_pObjects[6]->m_xmf4x4World = XMFLOAT4X4{ 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f , 0.0f, 0.0f, 1.0f, 0.0f , -100.0f, 0.0f, -150.0f, 1.0f };
	m_pObjects[7]->m_xmf4x4World = XMFLOAT4X4{ 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f , 0.0f, 0.0f, 1.0f, 0.0f , 100.0f, 0.0f, 150.0f, 1.0f };


	m_pObstacles[0] = new CGameObject();
	m_pObstacles[0]->SetPrepareRotate(-90.0f, 90.0f, 0.0f);
	m_pObstacles[0]->SetWorldTransf(XMFLOAT4X4{ 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f , 0.0f, 0.0f, 1.0f, 0.0f , -200.0f, 0.0f, 0.0f, 1.0f });
	m_pObstacles[0]->m_Object_Type = OBJECT_TYPE_OBSTACLE;
	m_pObstacles[0]->index = 0;
	m_pObstacles[0]->SetModel(m_pObstacleMesh);

	m_pObstacles[1] = new CGameObject();
	m_pObstacles[1]->SetPrepareRotate(-90.0f, -90.0f, 0.0f);
	m_pObstacles[1]->SetWorldTransf(XMFLOAT4X4{ 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f , 0.0f, 0.0f, 1.0f, 0.0f , 200.0f, 0.0f, 0.0f, 1.0f });
	m_pObstacles[1]->index = 1;	
	m_pObstacles[1]->m_Object_Type = OBJECT_TYPE_OBSTACLE;
	m_pObstacles[1]->SetModel(m_pObstacleMesh);

	m_pObstacles[2] = new CGameObject();
	m_pObstacles[2]->SetPrepareRotate(-90.0f, 0, 0.0f);
	m_pObstacles[2]->SetWorldTransf(XMFLOAT4X4{ 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f , 0.0f, 0.0f, 1.0f, 0.0f , 0.0f, 0.0f, 200.0f, 1.0f });
	m_pObstacles[2]->index = 2;	
	m_pObstacles[2]->m_Object_Type = OBJECT_TYPE_OBSTACLE;
	m_pObstacles[2]->SetModel(m_pObstacleMesh);

	m_pObstacles[3] = new CGameObject();
	m_pObstacles[3]->SetPrepareRotate(-90.0f, 0, 0.0f);
	m_pObstacles[3]->SetWorldTransf(XMFLOAT4X4{ 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f , 0.0f, 0.0f, 1.0f, 0.0f , 0.0f, 0.0f, -200.0f, 1.0f });
	m_pObstacles[3]->index = 3;
	m_pObstacles[3]->m_Object_Type = OBJECT_TYPE_OBSTACLE;
	m_pObstacles[3]->SetModel(m_pObstacleMesh);
}

void CScene::AnimateObjects(float fTimeElapsed)
{
	for (int i = 0; i < MAX_NUM_OBJECT; i++)
	{
		if (m_pObjects[i] != NULL)
		{
			if (!m_pObjects[i]->IsDelete())
				m_pObjects[i]->Animate(fTimeElapsed);
			else if (m_pObjects[i]->IsDelete())
				releaseObject(i);
		}
		if (m_pObstacles[i] != NULL)
			m_pObstacles[i]->Animate(fTimeElapsed);
	}
}

void CScene::InsertObject(PKT_CREATE_OBJECT CreateObjectInfo)
{
	CGameObject* pGameObject = new CGameObject();
	pGameObject->SetWorldTransf(CreateObjectInfo.WorldMatrix);
	if (m_pObjects[CreateObjectInfo.Object_Index])
	{
		m_pObjects[CreateObjectInfo.Object_Index]->Delete();
	}

	m_pObjects[CreateObjectInfo.Object_Index] = pGameObject;
}

void CScene::DeleteObject(PKT_DELETE_OBJECT DeleteObjectInfo)
{
	m_pObjects[DeleteObjectInfo.Object_Index]->Delete();
	m_pObjects[DeleteObjectInfo.Object_Index] = NULL;
}

int CScene::GetIndex()
{
	for (int i = MAX_CLIENT; i < MAX_NUM_OBJECT; ++i)
		if (m_pObjects[i] == NULL)
			return i;
	return -1;
}

void CScene::AddObject(CGameObject* object)
{
	int index = GetIndex();
	if (index != -1)
	{
		m_pObjects[index] = object;
		m_pObjects[index]->index = index;
		if (object->m_Object_Type == OBJECT_TYPE_MACHINE_BULLET 
			|| object->m_Object_Type == OBJECT_TYPE_BZK_BULLET 
			|| object->m_Object_Type == OBJECT_TYPE_BEAM_BULLET)
		{
			m_pObjects[index]->SetModel(m_pBulletMesh);
		}
		else if (object->m_Object_Type == OBJECT_TYPE_ITEM_HEALING 
			|| object->m_Object_Type == OBJECT_TYPE_ITEM_AMMO)
		{
			m_pObjects[index]->SetModel(m_pBulletMesh);
		}
	}
}

void CScene::releaseObject(int index)
{
	delete m_pObjects[index];
	m_pObjects[index] = NULL;
}
