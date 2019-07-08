#include "pch.h"
#include "Scene.h"
#include "GameObject.h"


Scene::Scene()
{
}


Scene::~Scene()
{
}

void Scene::InsertObjectFromLoadInfFromBin(char * pstrFileName, int nGroup)
{
	GameObject *pObject = new GameObject();

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
				pObject = new GameObject();
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

void Scene::init(CRepository * pRepository)
{
	robot_mesh_ = pRepository->GetModel("./Resource/PlayerCollisionBox.bin", NULL, NULL);
	bullet_mesh_ = pRepository->GetModel("./Resource/Bullet/Bullet.bin", NULL, NULL);

	for (int i = 0; i < MAX_NUM_OBJECT; ++i)
	{
		Objects_[i] = NULL;
	}

	for (int i = 0; i < MAX_CLIENT; ++i)
	{
		Objects_[i] = new GameObject();
		Objects_[i]->SetObjectType(OBJECT_TYPE_PLAYER);
		Objects_[i]->SetId(i);
		Objects_[i]->SetModel(robot_mesh_);
		Objects_[i]->SetHitPoint(PLAYER_HP);
		Objects_[i]->SetPlay(false);
	}

	Objects_[0]->SetWorldTransf(XMFLOAT4X4{ 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f , 0.0f, 0.0f, 1.0f, 0.0f , 0.0f, 0.0f, -150.0f, 1.0f });
	Objects_[1]->SetWorldTransf(XMFLOAT4X4{ 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f , 0.0f, 0.0f, 1.0f, 0.0f , 0.0f, 0.0f, 150.0f, 1.0f });
	Objects_[2]->SetWorldTransf(XMFLOAT4X4{ 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f , 0.0f, 0.0f, 1.0f, 0.0f , 100.0f, 0.0f, -150.0f, 1.0f });
	Objects_[3]->SetWorldTransf(XMFLOAT4X4{ 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f , 0.0f, 0.0f, 1.0f, 0.0f , -100.0f, 0.0f, 150.0f, 1.0f });
	Objects_[4]->SetWorldTransf(XMFLOAT4X4{ 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f , 0.0f, 0.0f, 1.0f, 0.0f , 0.0f, 0.0f, -200.0f, 1.0f });
	Objects_[5]->SetWorldTransf(XMFLOAT4X4{ 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f , 0.0f, 0.0f, 1.0f, 0.0f , 0.0f, 0.0f, 200.0f, 1.0f });
	Objects_[6]->SetWorldTransf(XMFLOAT4X4{ 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f , 0.0f, 0.0f, 1.0f, 0.0f , -100.0f, 0.0f, -150.0f, 1.0f });
	Objects_[7]->SetWorldTransf(XMFLOAT4X4{ 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f , 0.0f, 0.0f, 1.0f, 0.0f , 100.0f, 0.0f, 150.0f, 1.0f });

	for (int i = 0; i < 24; ++i)
	{
		BeamsaberCollisionmesh_[i].SetUse(false);
		BeamsaberCollisionmesh_[i].SetModel(robot_mesh_);
		BeamsaberCollisionmesh_[i].SetIndex(i);
		BeamsaberCollisionmesh_[i].SetHitPoint(3);
	}
}

void Scene::InsertObject(GameObject * pObject, int nGroup, bool bPrepareRotate, void * pContext)
{
	pObject->SetModel(models_[nGroup]);
	if (bPrepareRotate) pObject->AddPrepareRotate(0, 180, 0);

	Obstacles_.emplace_back(pObject);
}

int Scene::GetIndex()
{
	for (int i = MAX_CLIENT; i < MAX_NUM_OBJECT; ++i)
		if (Objects_[i] == NULL)
			return i;
	return -1;
}

void Scene::AddObject(GameObject* object)
{
	int index = GetIndex();
	OBJECT_TYPE type = object->GetObjectType();
	if (index != -1)
	{
		Objects_[index] = object;
		Objects_[index]->SetIndex(index);
		if (type == OBJECT_TYPE_MACHINE_BULLET
			|| type == OBJECT_TYPE_BZK_BULLET
			|| type == OBJECT_TYPE_BEAM_BULLET)
		{
			Objects_[index]->SetModel(bullet_mesh_);
		}
		else if (type == OBJECT_TYPE_ITEM_HEALING
			|| type == OBJECT_TYPE_ITEM_AMMO)
		{
			Objects_[index]->SetModel(bullet_mesh_);
		}
	}
}

void Scene::releaseObject(int index)
{
	delete Objects_[index];
	Objects_[index] = NULL;
}

XMFLOAT4X4 Scene::get_player_worldmatrix(int id)
{
	return Objects_[id]->GetWorldTransf();
}

void Scene::set_player_is_play(int id, bool play)
{
	Objects_[id]->SetPlay(play);
}

void Scene::set_object_id(int id)
{
	Objects_[id]->SetId(id);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GroundScene::BuildObjects(CRepository * pRepository)
{
	init(pRepository);

	models_.emplace_back(pRepository->GetModel("./Resource/Buildings/Hangar.bin", NULL, NULL));
	InsertObjectFromLoadInfFromBin("./Resource/Buildings/B_HangarSelfData.bin", 0);

	models_.emplace_back(pRepository->GetModel("./Resource/Buildings/Building_Double_Square.bin", NULL, NULL));
	InsertObjectFromLoadInfFromBin("./Resource/Buildings/B_DoubleSquareSelfData.bin", 1);

	models_.emplace_back(pRepository->GetModel("./Resource/Buildings/Building_Octagon.bin", NULL, NULL));
	InsertObjectFromLoadInfFromBin("./Resource/Buildings/B_OctagonSelfData.bin", 2);

	models_.emplace_back(pRepository->GetModel("./Resource/Buildings/Building_OctagonLongTier.bin", NULL, NULL));
	InsertObjectFromLoadInfFromBin("./Resource/Buildings/B_OctagonLongTierSelfData.bin", 3);

	models_.emplace_back(pRepository->GetModel("./Resource/Buildings/Building_Slope_top.bin", NULL, NULL));
	InsertObjectFromLoadInfFromBin("./Resource/Buildings/B_Slope_TopSelfData.bin", 4);

	models_.emplace_back(pRepository->GetModel("./Resource/Buildings/Building_Square.bin", NULL, NULL));
	InsertObjectFromLoadInfFromBin("./Resource/Buildings/B_SquareSelfData.bin", 5);

	models_.emplace_back(pRepository->GetModel("./Resource/Buildings/Building_Steeple_top.bin", NULL, NULL));
	InsertObjectFromLoadInfFromBin("./Resource/Buildings/B_Steeple_TopSelfData.bin", 6);

	models_.emplace_back(pRepository->GetModel("./Resource/Buildings/Wall.bin", NULL, NULL));
	InsertObjectFromLoadInfFromBin("./Resource/Buildings/WallSelfData.bin", 7);

	models_.emplace_back(pRepository->GetModel("./Resource/Buildings/fence.bin", NULL, NULL));
	InsertObjectFromLoadInfFromBin("./Resource/Buildings/FenceSelfData.bin", 8);
}

void GroundScene::AnimateObjects(float fTimeElapsed)
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void SpaceScene::BuildObjects(CRepository * pRepository)
{
	init(pRepository);

	models_.emplace_back(pRepository->GetModel("./Resource/Buildings/Space/Astroids1.bin", NULL, NULL));
	InsertObjectFromLoadInfFromBin("./Resource/Buildings/Space/S_Astroid_1SelfData.bin", 0);

	models_.emplace_back(pRepository->GetModel("./Resource/Buildings/Space/Astroids2.bin", NULL, NULL));
	InsertObjectFromLoadInfFromBin("./Resource/Buildings/Space/S_Astroid_2SelfData.bin", 1);

	models_.emplace_back(pRepository->GetModel("./Resource/Buildings/Space/Astroids3_1.bin", NULL, NULL));
	InsertObjectFromLoadInfFromBin("./Resource/Buildings/Space/S_Astroid_3SelfData.bin", 2);

	models_.emplace_back(pRepository->GetModel("./Resource/Buildings/Space/Astroids3_2.bin", NULL, NULL));
	InsertObjectFromLoadInfFromBin("./Resource/Buildings/Space/S_Astroid_3-1SelfData.bin", 3);

	models_.emplace_back(pRepository->GetModel("./Resource/Buildings/Space/Astroids3_3.bin", NULL, NULL));
	InsertObjectFromLoadInfFromBin("./Resource/Buildings/Space/S_Astroid_3-2SelfData.bin", 4);

	models_.emplace_back(pRepository->GetModel("./Resource/Buildings/Space/Astroids4.bin", NULL, NULL));
	InsertObjectFromLoadInfFromBin("./Resource/Buildings/Space/S_Astroid_4SelfData.bin", 5);

	models_.emplace_back(pRepository->GetModel("./Resource/Buildings/Space/Astroids5.bin", NULL, NULL));
	InsertObjectFromLoadInfFromBin("./Resource/Buildings/Space/S_Astroid_5SelfData.bin", 6);

	models_.emplace_back(pRepository->GetModel("./Resource/Buildings/Space/SpaceShip.bin", NULL, NULL));
	InsertObjectFromLoadInfFromBin("./Resource/Buildings/Space/S_SpaceShipSelfData.bin", 7);

	models_.emplace_back(pRepository->GetModel("./Resource/Buildings/Space/StarShip_Light.bin", NULL, NULL));
	InsertObjectFromLoadInfFromBin("./Resource/Buildings/Space/S_StarShipSelfData.bin", 8);
}

void SpaceScene::AnimateObjects(float fTimeElapsed)
{
}
