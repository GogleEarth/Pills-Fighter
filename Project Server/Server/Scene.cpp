#include "pch.h"
#include "Scene.h"
#include "Model.h"

CScene::CScene()
{
	for (int i = 0; i < MAX_NUM_OBJECT; i++)
	{
		m_pObjects[i] = NULL;
	}
}


CScene::~CScene()
{
}

void CScene::BuildObjects(CRepository* pRepository)
{
	m_vpModels.emplace_back(pRepository->GetModel("./Resource/Buildings/Hangar.bin", NULL, NULL));
	InsertObjectFromLoadInfFromBin("./Resource/Buildings/B_HangarSelfData.bin", 0);

	m_vpModels.emplace_back(pRepository->GetModel("./Resource/Buildings/Building_Double_Square.bin", NULL, NULL));
	InsertObjectFromLoadInfFromBin("./Resource/Buildings/B_DoubleSquareSelfData.bin", 1);

	m_vpModels.emplace_back(pRepository->GetModel("./Resource/Buildings/Building_Octagon.bin", NULL, NULL));
	InsertObjectFromLoadInfFromBin("./Resource/Buildings/B_OctagonSelfData.bin", 2);

	m_vpModels.emplace_back(pRepository->GetModel("./Resource/Buildings/Building_OctagonLongTier.bin", NULL, NULL));
	InsertObjectFromLoadInfFromBin("./Resource/Buildings/B_OctagonLongTierSelfData.bin", 3);

	m_vpModels.emplace_back(pRepository->GetModel("./Resource/Buildings/Building_Slope_top.bin", NULL, NULL));
	InsertObjectFromLoadInfFromBin("./Resource/Buildings/B_Slope_TopSelfData.bin", 4);

	m_vpModels.emplace_back(pRepository->GetModel("./Resource/Buildings/Building_Square.bin", NULL, NULL));
	InsertObjectFromLoadInfFromBin("./Resource/Buildings/B_SquareSelfData.bin", 5);

	m_vpModels.emplace_back(pRepository->GetModel("./Resource/Buildings/Building_Steeple_top.bin", NULL, NULL));
	InsertObjectFromLoadInfFromBin("./Resource/Buildings/B_Steeple_TopSelfData.bin", 6);

	m_vpModels.emplace_back(pRepository->GetModel("./Resource/Buildings/Wall.bin", NULL, NULL));
	InsertObjectFromLoadInfFromBin("./Resource/Buildings/WallSelfData.bin", 7);

	m_vpModels.emplace_back(pRepository->GetModel("./Resource/Buildings/Brick_Garage.bin", NULL, NULL));
	InsertObjectFromLoadInfFromBin("./Resource/Buildings/B_BrickGarageSelfData.bin", 8);

	m_vpModels.emplace_back(pRepository->GetModel("./Resource/Buildings/fence.bin", NULL, NULL));
	InsertObjectFromLoadInfFromBin("./Resource/Buildings/FenceSelfData.bin", 9);

	m_pGMMesh = pRepository->GetModel("./Resource/PlayerCollisionBox.bin", NULL, NULL);
	m_pBulletMesh = pRepository->GetModel("./Resource/Bullet/Bullet.bin", NULL, NULL);

	for (int i = 0; i < MAX_CLIENT; ++i)
	{
		m_pObjects[i] = new CGameObject();
		m_pObjects[i]->m_Object_Type = OBJECT_TYPE_PLAYER;
		m_pObjects[i]->m_iId = i;
		//m_pObjects[i]->SetPrepareRotate(-90.0f, 0.0f, 0.0f);
		m_pObjects[i]->SetModel(m_pGMMesh);
		m_pObjects[i]->hp = PLAYER_HP;
		m_pObjects[i]->m_bPlay = false;
	}
	m_pObjects[0]->m_xmf4x4World = XMFLOAT4X4{ 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f , 0.0f, 0.0f, 1.0f, 0.0f , 0.0f, 0.0f, -150.0f, 1.0f };
	m_pObjects[1]->m_xmf4x4World = XMFLOAT4X4{ 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f , 0.0f, 0.0f, 1.0f, 0.0f , 0.0f, 0.0f, 150.0f, 1.0f };
	m_pObjects[2]->m_xmf4x4World = XMFLOAT4X4{ 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f , 0.0f, 0.0f, 1.0f, 0.0f , 100.0f, 0.0f, -150.0f, 1.0f };
	m_pObjects[3]->m_xmf4x4World = XMFLOAT4X4{ 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f , 0.0f, 0.0f, 1.0f, 0.0f , -100.0f, 0.0f, 150.0f, 1.0f };
	m_pObjects[4]->m_xmf4x4World = XMFLOAT4X4{ 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f , 0.0f, 0.0f, 1.0f, 0.0f , 0.0f, 0.0f, -200.0f, 1.0f };
	m_pObjects[5]->m_xmf4x4World = XMFLOAT4X4{ 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f , 0.0f, 0.0f, 1.0f, 0.0f , 0.0f, 0.0f, 200.0f, 1.0f };
	m_pObjects[6]->m_xmf4x4World = XMFLOAT4X4{ 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f , 0.0f, 0.0f, 1.0f, 0.0f , -100.0f, 0.0f, -150.0f, 1.0f };
	m_pObjects[7]->m_xmf4x4World = XMFLOAT4X4{ 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f , 0.0f, 0.0f, 1.0f, 0.0f , 100.0f, 0.0f, 150.0f, 1.0f };

	for (int i = 0; i < 24; ++i)
	{
		m_BeamsaberCollisionmesh[i].in_used = false;
		//m_BeamsaberCollisionmesh[i].SetPrepareRotate(-90.0f, 0.0f, 0.0f);
		m_BeamsaberCollisionmesh[i].SetModel(m_pGMMesh);
		m_BeamsaberCollisionmesh[i].index = i;
		m_BeamsaberCollisionmesh[i].hp = 3;
	}
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
	}

	for (auto obstacle : Obstacles)
	{
		obstacle->Animate(fTimeElapsed);
	}

	for (int i = 0; i < 24; ++i)
	{
		if (m_BeamsaberCollisionmesh[i].in_used)
			m_BeamsaberCollisionmesh[i].Animate(fTimeElapsed);
	}
}

void CScene::InsertObjectFromLoadInfFromBin(char * pstrFileName, int nGroup)
{
	CGameObject *pObject = new CGameObject();

	FILE *pInFile = NULL;
	::fopen_s(&pInFile, pstrFileName, "rb");
	if (!pInFile) {
		std::cout << "lose bin file" << std::endl;
	}
	::rewind(pInFile);

	char pstrToken[64] = { '\0' };
	UINT nReads = 0;
	float loadedToken = 0;
	UINT cycle = 0;
	XMFLOAT3 posLoader = XMFLOAT3(0, 0, 0);
	XMFLOAT3 rotLoader = XMFLOAT3(0, 0, 0);

	while (feof(pInFile) == 0)
	{
		ReadPosrotFromFile(pInFile, pstrToken);

		if (!strcmp(pstrToken, "m_value"))
		{
			switch (cycle) {
			case 0:
				nReads = LeftByteFromFile(pInFile, 2);
				loadedToken = ReadFloatFromFile(pInFile);
				nReads = LeftByteFromFile(pInFile, 1);
				posLoader.x = loadedToken;
				++cycle;
				break;
			case 1:
				nReads = LeftByteFromFile(pInFile, 2);
				loadedToken = ReadFloatFromFile(pInFile);
				nReads = LeftByteFromFile(pInFile, 1);
				posLoader.y = loadedToken;
				++cycle;
				break;
			case 2:
				nReads = LeftByteFromFile(pInFile, 2);
				loadedToken = ReadFloatFromFile(pInFile);
				nReads = LeftByteFromFile(pInFile, 1);
				posLoader.z = loadedToken;
				++cycle;
				break;
			case 3:
				nReads = LeftByteFromFile(pInFile, 2);
				loadedToken = ReadFloatFromFile(pInFile);
				nReads = LeftByteFromFile(pInFile, 1);
				rotLoader.x = loadedToken;
				++cycle;
				break;
			case 4:
				nReads = LeftByteFromFile(pInFile, 2);
				loadedToken = ReadFloatFromFile(pInFile);
				nReads = LeftByteFromFile(pInFile, 1);
				rotLoader.y = loadedToken;
				++cycle;
				break;
			case 5:
				nReads = LeftByteFromFile(pInFile, 2);
				loadedToken = ReadFloatFromFile(pInFile);
				nReads = LeftByteFromFile(pInFile, 1);
				rotLoader.z = loadedToken;
				pObject = new CGameObject();
				pObject->SetPosition(posLoader);
				pObject->SetPrepareRotate(rotLoader.x, rotLoader.y, rotLoader.z);
				InsertObject(pObject, nGroup, true, NULL);
				cycle = 0;
				break;
			}
		}
		else
		{
			std::cout << "bin file load error" << std::endl;
			break;
		}
	}
}

void CScene::InsertObject(CGameObject * pObject, int nGroup, bool bPrepareRotate, void * pContext)
{
	pObject->SetModel(m_vpModels[nGroup]);
	if (bPrepareRotate) pObject->AddPrepareRotate(0, 180, 0);

	Obstacles.emplace_back(pObject);
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
