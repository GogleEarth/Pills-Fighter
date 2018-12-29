#include "pch.h"
#include "Scene.h"

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
	CMesh** ppMesh;
	CCubeMesh** ppCubeMesh;
	UINT nMeshes;
	CreateRobotObjectMesh(ppMesh, ppCubeMesh, nMeshes);

	m_ppBulletMesh = new CMesh*[1];
	m_ppBulletMesh[0] = new CMesh("./Resource/Bullet/Bullet.FBX");
	XMFLOAT3 extend = m_ppBulletMesh[0]->GetExtents();
	std::cout << m_ppBulletMesh[0]->GetCenter().x << ", " << m_ppBulletMesh[0]->GetCenter().y << ", " << m_ppBulletMesh[0]->GetCenter().z << std::endl;
	std::cout << extend.x << ", " << extend.y << ", " << extend.z << std::endl;

	m_ppObstacleMesh = new CMesh*[1];
	m_ppObstacleMesh[0] = new CMesh("./Resource/hangar.FBX");
	XMFLOAT3 obstacleextend = m_ppObstacleMesh[0]->GetExtents();
	std::cout << m_ppObstacleMesh[0]->GetCenter().x << ", " << m_ppObstacleMesh[0]->GetCenter().y << ", " << m_ppObstacleMesh[0]->GetCenter().z << std::endl;
	std::cout << obstacleextend.x << ", " << obstacleextend.y << ", " << obstacleextend.z << std::endl;

	for (int i = 0; i < MAX_CLIENT; ++i)
	{
		m_pObjects[i] = new CGameObject();
		m_pObjects[i]->m_Object_Type = OBJECT_TYPE_PLAYER;
		m_pObjects[i]->m_iId = i;
		m_pObjects[i]->SetMesh(ppMesh, ppCubeMesh, nMeshes);
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
	m_pObstacles[0]->SetPrepareRotate(0, 0, 0);
	m_pObstacles[0]->SetWorldTransf(XMFLOAT4X4{ 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f , 0.0f, 0.0f, 1.0f, 0.0f , -200.0f, 0.0f, 100.0f, 1.0f });
	m_pObstacles[0]->m_Object_Type = OBJECT_TYPE_OBSTACLE;
	m_pObstacles[0]->index = 0;
	m_pObstacles[0]->SetMesh(m_ppObstacleMesh, NULL, 1);

	m_pObstacles[1] = new CGameObject();
	m_pObstacles[1]->SetPrepareRotate(0, 0, 0);
	m_pObstacles[1]->SetWorldTransf(XMFLOAT4X4{ 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f , 0.0f, 0.0f, 1.0f, 0.0f , 200.0f, 0.0f, 100.0f, 1.0f });
	m_pObstacles[1]->index = 1;	
	m_pObstacles[1]->m_Object_Type = OBJECT_TYPE_OBSTACLE;
	m_pObstacles[1]->SetMesh(m_ppObstacleMesh, NULL, 1);

	m_pObstacles[2] = new CGameObject();
	m_pObstacles[2]->SetPrepareRotate(0, 0, 0);
	m_pObstacles[2]->SetWorldTransf(XMFLOAT4X4{ 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f , 0.0f, 0.0f, 1.0f, 0.0f , 0.0f, 0.0f, 100.0f, 1.0f });
	m_pObstacles[2]->index = 2;	
	m_pObstacles[2]->m_Object_Type = OBJECT_TYPE_OBSTACLE;
	m_pObstacles[2]->SetMesh(m_ppObstacleMesh, NULL, 1);
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
		m_pObjects[CreateObjectInfo.Object_Index]->DeleteObject();
	}

	m_pObjects[CreateObjectInfo.Object_Index] = pGameObject;
}

void CScene::DeleteObject(PKT_DELETE_OBJECT DeleteObjectInfo)
{
	m_pObjects[DeleteObjectInfo.Object_Index]->DeleteObject();
	m_pObjects[DeleteObjectInfo.Object_Index] = NULL;
}

int CScene::GetIndex()
{
	for (int i = 2; i < MAX_NUM_OBJECT; ++i)
		if (m_pObjects[i] == NULL)
			return i;
	return -1;
}

void CScene::AddObject(CGameObject objcet)
{
	int index = GetIndex();
	if (index != -1)
	{
		m_pObjects[index] = new CGameObject(objcet);
		m_pObjects[index]->index = index;
		if (objcet.m_Object_Type == OBJECT_TYPE_BULLET)
		{
			m_pObjects[index]->SetMesh(m_ppBulletMesh, NULL, 1);
		}
	}
}

void CScene::releaseObject(int index)
{
	delete m_pObjects[index];
	m_pObjects[index] = NULL;
}
