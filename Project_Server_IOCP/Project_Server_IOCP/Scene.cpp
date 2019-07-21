#include "pch.h"
#include "Scene.h"
#include "GameObject.h"


Scene::Scene()
{
}


Scene::~Scene()
{
}

void Scene::AnimateObjects(float fTimeElapsed)
{
	elapsed_game_time_ += fTimeElapsed;

	for (int i = 0; i < MAX_CLIENT; ++i)
	{
		Objects_[i].Animate(fTimeElapsed);
	}
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
		Objects_[i].SetUse(false);
	}

	for (int i = 0; i < MAX_CLIENT; ++i)
	{
		Objects_[i].SetObjectType(OBJECT_TYPE_PLAYER);
		Objects_[i].SetId(i);
		Objects_[i].SetModel(robot_mesh_);
		Objects_[i].SetHitPoint(PLAYER_HP);
		Objects_[i].SetPlay(false);
	}

	Objects_[0].SetWorldTransf(XMFLOAT4X4{ 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f , 0.0f, 0.0f, 1.0f, 0.0f , 0.0f, 0.0f, -150.0f, 1.0f });
	Objects_[1].SetWorldTransf(XMFLOAT4X4{ 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f , 0.0f, 0.0f, 1.0f, 0.0f , 0.0f, 0.0f, 150.0f, 1.0f });
	Objects_[2].SetWorldTransf(XMFLOAT4X4{ 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f , 0.0f, 0.0f, 1.0f, 0.0f , 100.0f, 0.0f, -150.0f, 1.0f });
	Objects_[3].SetWorldTransf(XMFLOAT4X4{ 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f , 0.0f, 0.0f, 1.0f, 0.0f , -100.0f, 0.0f, 150.0f, 1.0f });
	Objects_[4].SetWorldTransf(XMFLOAT4X4{ 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f , 0.0f, 0.0f, 1.0f, 0.0f , 0.0f, 0.0f, -200.0f, 1.0f });
	Objects_[5].SetWorldTransf(XMFLOAT4X4{ 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f , 0.0f, 0.0f, 1.0f, 0.0f , 0.0f, 0.0f, 200.0f, 1.0f });
	Objects_[6].SetWorldTransf(XMFLOAT4X4{ 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f , 0.0f, 0.0f, 1.0f, 0.0f , -100.0f, 0.0f, -150.0f, 1.0f });
	Objects_[7].SetWorldTransf(XMFLOAT4X4{ 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f , 0.0f, 0.0f, 1.0f, 0.0f , 100.0f, 0.0f, 150.0f, 1.0f });

	for (int i = 0; i < 24; ++i)
	{
		BeamsaberCollisionmesh_[i].SetUse(false);
		BeamsaberCollisionmesh_[i].SetModel(robot_mesh_);
		BeamsaberCollisionmesh_[i].SetIndex(i);
		BeamsaberCollisionmesh_[i].SetHitPoint(3);
	}

	for (auto obstacle : Obstacles_)
		obstacle->Animate(0.016f);

	elapsed_game_time_ = 0.0f;
	event_time_ = 0.0f;
	is_being_event_ = false;
	alert_ = false;
}

void Scene::init()
{
	for (int i = MAX_CLIENT; i < MAX_NUM_OBJECT; ++i)
	{
		Objects_[i].Delete();
		Objects_[i].SetUse(false);
	}

	for (int i = 0; i < MAX_CLIENT; ++i)
	{
		Objects_[i].SetHitPoint(PLAYER_HP);
		Objects_[i].SetPlay(false);
	}

	Objects_[0].SetWorldTransf(XMFLOAT4X4{ 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f , 0.0f, 0.0f, 1.0f, 0.0f , 0.0f, 0.0f, -150.0f, 1.0f });
	Objects_[1].SetWorldTransf(XMFLOAT4X4{ 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f , 0.0f, 0.0f, 1.0f, 0.0f , 0.0f, 0.0f, 150.0f, 1.0f });
	Objects_[2].SetWorldTransf(XMFLOAT4X4{ 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f , 0.0f, 0.0f, 1.0f, 0.0f , 100.0f, 0.0f, -150.0f, 1.0f });
	Objects_[3].SetWorldTransf(XMFLOAT4X4{ 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f , 0.0f, 0.0f, 1.0f, 0.0f , -100.0f, 0.0f, 150.0f, 1.0f });
	Objects_[4].SetWorldTransf(XMFLOAT4X4{ 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f , 0.0f, 0.0f, 1.0f, 0.0f , 0.0f, 0.0f, -200.0f, 1.0f });
	Objects_[5].SetWorldTransf(XMFLOAT4X4{ 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f , 0.0f, 0.0f, 1.0f, 0.0f , 0.0f, 0.0f, 200.0f, 1.0f });
	Objects_[6].SetWorldTransf(XMFLOAT4X4{ 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f , 0.0f, 0.0f, 1.0f, 0.0f , -100.0f, 0.0f, -150.0f, 1.0f });
	Objects_[7].SetWorldTransf(XMFLOAT4X4{ 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f , 0.0f, 0.0f, 1.0f, 0.0f , 100.0f, 0.0f, 150.0f, 1.0f });

	for (int i = 0; i < 24; ++i)
	{
		BeamsaberCollisionmesh_[i].SetUse(false);
		BeamsaberCollisionmesh_[i].SetHitPoint(3);
	}

	elapsed_game_time_ = 0.0f;
	event_time_ = 0.0f;
	is_being_event_ = false;
	alert_ = false;
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
		if (Objects_[i].GetUse() == false)
			return i;
	return -1;
}

int Scene::AddObject(OBJECT_TYPE type, int hp, float life_time, float speed, XMFLOAT4X4 matrix)
{
	obj_lock.lock();
	int index = GetIndex();
	Objects_[index].SetObjectType(type);
	Objects_[index].SetMaxHitPoint(hp);
	Objects_[index].SetHitPoint(hp);
	if (index != -1)
	{
		Objects_[index].SetIndex(index);
		if (type == OBJECT_TYPE_MACHINE_BULLET
			|| type == OBJECT_TYPE_BZK_BULLET
			|| type == OBJECT_TYPE_BEAM_BULLET)
		{
			Objects_[index].SetModel(bullet_mesh_);
			Objects_[index].set_life(life_time);
			Objects_[index].set_speed(speed);
		}
		else if (type == OBJECT_TYPE_ITEM_HEALING
			|| type == OBJECT_TYPE_ITEM_AMMO)
		{
			Objects_[index].SetModel(bullet_mesh_);
		}
		else if (type == OBJECT_TYPE_METEOR)
		{
			Objects_[index].SetModel(bullet_mesh_);
			Objects_[index].set_life(life_time);
			Objects_[index].set_speed(speed);
		}
	}
	Objects_[index].SetWorldTransf(matrix);
	Objects_[index].SetUse(true);
	obj_lock.unlock();
	return index;
}

bool Scene::check_collision_obstacles(int object)
{
	for (auto obstacle : Obstacles_)
	{
		for (auto objaabb : Objects_[object].GetAABB())
		{
			for (auto playeraabb : obstacle->GetAABB())
			{
				if (objaabb.Intersects(playeraabb))
				{
					std::cout << "건탄충\n";
					return true;
				}
			}
		}
	}

	return false;
}

bool Scene::check_collision_player(int object)
{
	for (int i = 0; i < MAX_CLIENT; ++i)
	{
		if (Objects_[i].GetPlay())
		{
			for (auto objaabb : Objects_[object].GetAABB())
			{
				for (auto playeraabb : Objects_[i].GetAABB())
				{
					if (objaabb.Intersects(playeraabb))
					{
						if (Objects_[object].GetObjectType() == OBJECT_TYPE_ITEM_AMMO_1 ||
							Objects_[object].GetObjectType() == OBJECT_TYPE_ITEM_AMMO_2)
						{
							std::cout << "아플충\n";

							PKT_PICK_ITEM* pkt_pi = new PKT_PICK_ITEM;
							pkt_pi->PktId = PKT_ID_PICK_ITEM;
							pkt_pi->PktSize = sizeof(PKT_PICK_ITEM);
							pkt_pi->ID = i;
							pkt_pi->AMMO = Objects_[object].GetHitPoint();
							if (Objects_[object].GetObjectType() == OBJECT_TYPE_ITEM_AMMO_1)
								pkt_pi->Item_type = ITEM_TYPE_AMMO1;
							else
								pkt_pi->Item_type = ITEM_TYPE_AMMO2;
							item_queue_.push(pkt_pi);
						}
						else if (Objects_[object].GetObjectType() == OBJECT_TYPE_ITEM_HEALING)
						{
							std::cout << "아플충\n";

							PKT_PICK_ITEM* pkt_pi = new PKT_PICK_ITEM;
							pkt_pi->PktId = PKT_ID_PICK_ITEM;
							pkt_pi->PktSize = sizeof(PKT_PICK_ITEM);
							pkt_pi->ID = i;
							pkt_pi->HP = Objects_[object].GetHitPoint();
							pkt_pi->Item_type = ITEM_TYPE_HEALING;

							Objects_[i].SetHitPoint(Objects_[i].GetHitPoint() + 50);

							item_queue_.push(pkt_pi);
						}
						else if (Objects_[object].GetObjectType() == OBJECT_TYPE_MACHINE_BULLET ||
							Objects_[object].GetObjectType() == OBJECT_TYPE_BEAM_BULLET || 
							Objects_[object].GetObjectType() == OBJECT_TYPE_BZK_BULLET)
						{
							std::cout << "플탄충\n";

						}
						return true;
					}
				}
			}
		}
	}
	return false;
}

PKT_PICK_ITEM* Scene::item_dequeue()
{
	if (item_queue_.empty()) return nullptr;
	auto packet = item_queue_.front();
	item_queue_.pop();
	return packet;
}

void Scene::start_event()
{
	std::cout << "이벤트 시작\n";
	is_being_event_ = true;
}

void Scene::end_event()
{
	std::cout << "이벤트 끝\n";
	elapsed_game_time_ = 0.0f;
	event_time_ = 0.0f;
	is_being_event_ = false;
	alert_ = false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GroundScene::BuildObjects(CRepository * pRepository)
{
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

	init(pRepository);
	gravity_ = -9.8f;
}

void GroundScene::AnimateObjects(float fTimeElapsed)
{
	Scene::AnimateObjects(fTimeElapsed);
}

void GroundScene::SceneEvent(float fTimeElapsed)
{
	event_time_ += fTimeElapsed;
}

void GroundScene::start_event()
{
	Scene::start_event();
	gravity_ = ((float)((rand() % 196) - 98)) / 10.0f;
}

void GroundScene::end_event()
{
	Scene::end_event();
	gravity_ = -9.8f;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void SpaceScene::BuildObjects(CRepository * pRepository)
{
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

	init(pRepository);
	gravity_ = 0.0f;
}

void SpaceScene::AnimateObjects(float fTimeElapsed)
{
	Scene::AnimateObjects(fTimeElapsed);
}

void SpaceScene::SceneEvent(float fTimeElapsed)
{
	event_time_ += fTimeElapsed;
	meteor_cooltime_duration_ += fTimeElapsed;
}

void SpaceScene::start_event()
{
	Scene::start_event();
	meteor_cooltime_ = 0.05f;
}

void SpaceScene::end_event()
{
	Scene::end_event();
}
