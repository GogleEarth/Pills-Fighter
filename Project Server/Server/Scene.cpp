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
	m_ppBulletCubeMesh = new CCubeMesh*[1];
	m_ppBulletMesh[0] = new CMesh("./Resource/Bullet/Bullet.FBX");
	XMFLOAT3 extend = m_ppBulletMesh[0]->GetExtents();
	m_ppBulletCubeMesh[0] = new CCubeMesh(m_ppBulletMesh[0]->GetCenter(), extend.x, extend.y, extend.z);

	m_pObjects[0] = new CGameObject();
	m_pObjects[0]->m_xmf4x4World = XMFLOAT4X4{ 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f , 0.0f, 0.0f, 1.0f, 0.0f , 0.0f, 0.0f, -150.0f, 1.0f };
	m_pObjects[0]->m_Object_Type = OBJECT_TYPE_PLAYER;
	m_pObjects[0]->m_iId = 0;
	m_pObjects[0]->SetMesh(ppMesh, ppCubeMesh, nMeshes);

	m_pObjects[1] = new CGameObject();
	m_pObjects[1]->m_xmf4x4World = XMFLOAT4X4{ 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f , 0.0f, 0.0f, 1.0f, 0.0f , 0.0f, 0.0f, 150.0f, 1.0f };
	m_pObjects[1]->m_Object_Type = OBJECT_TYPE_PLAYER;
	m_pObjects[1]->m_iId = 1;
	m_pObjects[1]->SetMesh(ppMesh, ppCubeMesh, nMeshes);

	m_pObstacles[0] = new CGameObject();
	m_pObstacles[0]->SetPosition(XMFLOAT3(-200.0f, 0.0f, 100.0f));

	m_pObstacles[1] = new CGameObject();
	m_pObstacles[1]->SetPosition(XMFLOAT3(200.0f, 0.0f, 100.0f));

	m_pObstacles[2] = new CGameObject();
	m_pObstacles[2]->SetPosition(XMFLOAT3(0.0f, 0.0f, 100.0f));
}

void CScene::AnimateObjects(float fTimeElapsed)
{
	for (int i = 0; i < MAX_NUM_OBJECT; i++)
	{
		if(m_pObjects[i] != NULL)
			if(!m_pObjects[i]->IsDelete())
				m_pObjects[i]->Animate(fTimeElapsed);
		if (m_pObjects[i] != NULL)
			if (m_pObjects[i]->IsDelete())
				releaseObject(i);
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
	m_pObjects[index] = new CGameObject(objcet);
	m_pObjects[index]->index = index;
	if (objcet.m_Object_Type == OBJECT_TYPE_BULLET)
	{
		m_pObjects[index]->SetMesh(m_ppBulletMesh, m_ppBulletCubeMesh, 1);
	}
}

void CScene::releaseObject(int index)
{
	delete m_pObjects[index];
	m_pObjects[index] = NULL;
}
