#include "pch.h"
#include "Scene.h"

CScene::CScene()
{
	for (int i = 0; i < MAX_NUM_OBJECT; i++)
	{
		m_pObjects[i] = NULL;
		//m_pPlayers[i] = NULL;
		//m_pBulletObjects[i] = NULL;
	}
}


CScene::~CScene()
{
}

void CScene::BuildObjects()
{
	m_pObjects[0] = new CGameObject();
	m_pObjects[0]->SetPosition(XMFLOAT3(0.0f, 0.0f, -50.0f));
	m_pObjects[0]->SetLook(XMFLOAT3(0.0f, 0.0f, 1.0f));
	m_pObjects[0]->SetUp(XMFLOAT3(0.0, 1.0f, 0.0f));
	m_pObjects[0]->SetRight(Vector3::CrossProduct(m_pObjects[0]->GetLook(), m_pObjects[0]->GetUp()));
	m_pObjects[0]->m_Object_Type = OBJECT_TYPE_PLAYER;

	m_pObjects[1] = new CGameObject();
	m_pObjects[1]->SetPosition(XMFLOAT3(0.0f, 0.0f, 50.0f));
	m_pObjects[1]->SetLook(XMFLOAT3(0.0f, 0.0f, -1.0f));
	m_pObjects[1]->SetUp(XMFLOAT3(0.0, 1.0f, 0.0f));
	m_pObjects[1]->SetRight(Vector3::CrossProduct(m_pObjects[1]->GetLook(), m_pObjects[1]->GetUp()));
	m_pObjects[1]->m_Object_Type = OBJECT_TYPE_PLAYER;
}

void CScene::CheckCollision()
{
	CGameObject	**ppBuildingObjects;
	UINT nBuildingObjects;

	std::vector<CGameObject*> vEnemyObjects;
	std::vector<CGameObject*> vBulletObjects;
	std::vector<CGameObject*> vEnemyBulletObjects;



	for (const auto& Bullet : vBulletObjects)
	{
		for (UINT nIndexBuilding = 0; nIndexBuilding < nBuildingObjects; nIndexBuilding++)
		{
			for (UINT i = 0; i < ppBuildingObjects[nIndexBuilding]->GetNumMeshes(); i++)
			{
				for (UINT j = 0; j < Bullet->GetNumMeshes(); j++)
				{
					if (Bullet->GetOOBB(j).Intersects(ppBuildingObjects[nIndexBuilding]->GetOOBB(i)))
					{
						Bullet->DeleteObject();
					}
				}
			}
		}

		for (const auto& Enemy : vEnemyObjects)
		{
			for (UINT i = 0; i < Enemy->GetNumMeshes(); i++)
			{
				for (UINT j = 0; j < Bullet->GetNumMeshes(); j++)
				{
					if (Bullet->GetOOBB(j).Intersects(Enemy->GetOOBB(i)))
					{
						Bullet->DeleteObject();
					}
				}
			}
		}
	}

	for (const auto& Bullet : vEnemyBulletObjects)
	{
		for (UINT nIndexBuilding = 0; nIndexBuilding < nBuildingObjects; nIndexBuilding++)
		{
			for (UINT i = 0; i < ppBuildingObjects[nIndexBuilding]->GetNumMeshes(); i++)
			{
				for (UINT j = 0; j < Bullet->GetNumMeshes(); j++)
				{
					if (Bullet->GetOOBB(j).Intersects(ppBuildingObjects[nIndexBuilding]->GetOOBB(i)))
					{
						Bullet->DeleteObject();
					}
				}
			}
		}
	}
	

	for (const auto& Enemy : vEnemyObjects)
	{
		for (UINT nIndexBuilding = 0; nIndexBuilding < nBuildingObjects; nIndexBuilding++)
		{
			for (UINT i = 0; i < ppBuildingObjects[nIndexBuilding]->GetNumMeshes(); i++)
			{
				for (UINT j = 0; j < Enemy->GetNumMeshes(); j++)
				{
					BoundingOrientedBox EnemysOOBB = Enemy->GetOOBB(j);
					BoundingOrientedBox BuildingsOOBB = ppBuildingObjects[nIndexBuilding]->GetOOBB(i);

					if (EnemysOOBB.Intersects(BuildingsOOBB))
					{
						XMFLOAT3 BuildingMeshCenter = BuildingsOOBB.Center;
						XMFLOAT3 BuildingMeshExtents = BuildingsOOBB.Extents;

						XMFLOAT3 EnemyMeshCenter = EnemysOOBB.Center;
						XMFLOAT3 EnemyMeshExtents = EnemysOOBB.Extents;

						//if(EnemyPosition.x > BuildingMeshExtents.x + BuildingMeshCenter.x)
						// 충돌처리
					}
				}
			}
		}
	}
}

void CScene::AnimateObjects(float fTimeElapsed)
{
	for (int i = 0; i < MAX_NUM_OBJECT; i++)
	{
		//m_pObjects[i]->Animate(fTimeElapsed);
	}
	//CheckCollision();
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