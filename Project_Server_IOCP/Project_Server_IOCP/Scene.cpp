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
		Objects_[i].Animate(fTimeElapsed, 0);
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
	saber_mesh_ = pRepository->GetModel("./Resource/SaberCollisionBox.bin", NULL, NULL);

	for (int i = 0; i < MAX_NUM_OBJECT; ++i)
	{
		Objects_[i].SetUse(false);
	}

	for (int i = 0; i < MAX_CLIENT; ++i)
	{
		Objects_[i].SetObjectType(OBJECT_TYPE_PLAYER);
		Objects_[i].SetId(i);
		Objects_[i].SetModel(robot_mesh_);
		Objects_[i].SetMaxHitPoint(PLAYER_HP);
		Objects_[i].SetHitPoint(PLAYER_HP);
		Objects_[i].SetPlay(false);
		Objects_[i].set_is_player(true);
		Objects_[i].set_is_die(false);
	}

	Objects_[0].SetWorldTransf(XMFLOAT4X4{ 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f , 0.0f, 0.0f, 1.0f, 0.0f , 0.0f, 0.0f, -150.0f, 1.0f });
	Objects_[1].SetWorldTransf(XMFLOAT4X4{ 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f , 0.0f, 0.0f, 1.0f, 0.0f , 0.0f, 0.0f, 150.0f, 1.0f });
	Objects_[2].SetWorldTransf(XMFLOAT4X4{ 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f , 0.0f, 0.0f, 1.0f, 0.0f , 100.0f, 0.0f, -150.0f, 1.0f });
	Objects_[3].SetWorldTransf(XMFLOAT4X4{ 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f , 0.0f, 0.0f, 1.0f, 0.0f , -100.0f, 0.0f, 150.0f, 1.0f });
	Objects_[4].SetWorldTransf(XMFLOAT4X4{ 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f , 0.0f, 0.0f, 1.0f, 0.0f , 0.0f, 0.0f, -200.0f, 1.0f });
	Objects_[5].SetWorldTransf(XMFLOAT4X4{ 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f , 0.0f, 0.0f, 1.0f, 0.0f , 0.0f, 0.0f, 200.0f, 1.0f });
	Objects_[6].SetWorldTransf(XMFLOAT4X4{ 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f , 0.0f, 0.0f, 1.0f, 0.0f , -100.0f, 0.0f, -150.0f, 1.0f });
	Objects_[7].SetWorldTransf(XMFLOAT4X4{ 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f , 0.0f, 0.0f, 1.0f, 0.0f , 100.0f, 0.0f, 150.0f, 1.0f });

	for (auto obstacle : Obstacles_)
		obstacle->Animate(0.016f, 0);

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
		Objects_[i].set_elapsed_time_to_zero();
	}

	for (int i = 0; i < MAX_CLIENT; ++i)
	{
		Objects_[i].SetHitPoint(PLAYER_HP);
		Objects_[i].SetPlay(false);
		Objects_[i].set_is_die(false);
		Objects_[i].set_elapsed_time_to_zero();
	}

	Objects_[0].SetWorldTransf(XMFLOAT4X4{ 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f , 0.0f, 0.0f, 1.0f, 0.0f , 0.0f, 0.0f, -150.0f, 1.0f });
	Objects_[1].SetWorldTransf(XMFLOAT4X4{ 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f , 0.0f, 0.0f, 1.0f, 0.0f , 0.0f, 0.0f, 150.0f, 1.0f });
	Objects_[2].SetWorldTransf(XMFLOAT4X4{ 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f , 0.0f, 0.0f, 1.0f, 0.0f , 100.0f, 0.0f, -150.0f, 1.0f });
	Objects_[3].SetWorldTransf(XMFLOAT4X4{ 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f , 0.0f, 0.0f, 1.0f, 0.0f , -100.0f, 0.0f, 150.0f, 1.0f });
	Objects_[4].SetWorldTransf(XMFLOAT4X4{ 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f , 0.0f, 0.0f, 1.0f, 0.0f , 0.0f, 0.0f, -200.0f, 1.0f });
	Objects_[5].SetWorldTransf(XMFLOAT4X4{ 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f , 0.0f, 0.0f, 1.0f, 0.0f , 0.0f, 0.0f, 200.0f, 1.0f });
	Objects_[6].SetWorldTransf(XMFLOAT4X4{ 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f , 0.0f, 0.0f, 1.0f, 0.0f , -100.0f, 0.0f, -150.0f, 1.0f });
	Objects_[7].SetWorldTransf(XMFLOAT4X4{ 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f , 0.0f, 0.0f, 1.0f, 0.0f , 100.0f, 0.0f, 150.0f, 1.0f });

	while (item_queue_.size() > 0)
		item_queue_.pop();
	while (player_life_queue_.size() > 0)
		player_life_queue_.pop();
	while (score_queue_.size() > 0)
		score_queue_.pop();
	while (create_effect_queue_.size() > 0)
		create_effect_queue_.pop();
	while (player_die_queue_.size() > 0)
		player_die_queue_.pop();

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

int Scene::AddObject(OBJECT_TYPE type, int hp, float life_time, float speed, XMFLOAT4X4 matrix, int id)
{
	obj_lock.lock();
	int index = GetIndex();
	Objects_[index].SetObjectType(type);
	Objects_[index].SetMaxHitPoint(hp);
	Objects_[index].SetHitPoint(hp);
	XMFLOAT4X4 worldmatrix = matrix;
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
			|| type == OBJECT_TYPE_ITEM_AMMO
			|| type == OBJECT_TYPE_ITEM_AMMO_1
			|| type == OBJECT_TYPE_ITEM_AMMO_2)
		{
			Objects_[index].SetModel(bullet_mesh_);
		}
		else if (type == OBJECT_TYPE_METEOR)
		{
			Objects_[index].SetModel(bullet_mesh_);
			Objects_[index].set_life(life_time);
			Objects_[index].set_speed(speed);
		}
		else if (type == OBJECT_TYPE_SABER)
		{
			Objects_[index].SetModel(saber_mesh_);
			Objects_[index].set_life(life_time);
			Objects_[index].set_speed(speed);
			XMFLOAT3 position = XMFLOAT3{ matrix._41, matrix._42, matrix._43 };
			XMFLOAT3 look = XMFLOAT3{ matrix._31, matrix._32, matrix._33 };
			position = Vector3::Add(position, Vector3::ScalarProduct(look, 11.0f, false));
			worldmatrix._41 = position.x;
			worldmatrix._42 = position.y;
			worldmatrix._43 = position.z;
		}
	}
	Objects_[index].set_owner_id(id);
	Objects_[index].SetWorldTransf(worldmatrix);
	Objects_[index].SetPrevPosition(XMFLOAT3{ worldmatrix._41, worldmatrix._42, worldmatrix._43 });
	Objects_[index].SetUse(true);
	obj_lock.unlock();
	return index;
}

void Scene::set_player_team(int id, char team)
{
	//std::cout << id << "ÀÇ ÆÀ : " << (int)team << "\n";
	Objects_[id].set_team(team);
}

bool Scene::check_collision_obstacles(int object)
{
	FXMVECTOR origin = XMLoadFloat3(&Objects_[object].GetPrevPosition());
	FXMVECTOR direction = XMLoadFloat3(&Objects_[object].GetLook());
	float distance;

	for (auto obstacle : Obstacles_)
	{
		for (auto playeraabb : obstacle->GetAABB())
		{
			if (playeraabb.Intersects(origin, direction, distance))
			{
				XMFLOAT3 length = Vector3::Subtract(Objects_[object].GetPosition(), Objects_[object].GetPrevPosition());
				float len = Vector3::Length(length);
				if (distance <= len)
				{
					return true;
				}
			}
		}
	}

	return false;
}

bool Scene::check_saber_collision_player(int object)
{
	for (int i = 0; i < MAX_CLIENT; ++i)
	{
		if (Objects_[i].GetPlay())
		{
			if (!Objects_[i].get_is_die())
			{
				for (auto objaabb : Objects_[object].GetAABB())
				{
					for (auto playeraabb : Objects_[i].GetAABB())
					{
						if (playeraabb.Intersects(objaabb))
						{
							if (Objects_[object].GetObjectType() == OBJECT_TYPE_SABER)
							{
								if (i == Objects_[object].get_owner_id()) continue;
								PKT_PLAYER_LIFE* pkt_pl = new PKT_PLAYER_LIFE;
								pkt_pl->ID = i;
								Objects_[i].SetHitPoint(Objects_[i].GetHitPoint() - Objects_[object].GetHitPoint());
								pkt_pl->HP = Objects_[object].GetHitPoint();
								pkt_pl->PktId = PKT_ID_PLAYER_LIFE;
								pkt_pl->PktSize = sizeof(PKT_PLAYER_LIFE);

								PKT_CREATE_EFFECT* pkt_ce = new PKT_CREATE_EFFECT();
								pkt_ce->PktId = PKT_ID_CREATE_EFFECT;
								pkt_ce->PktSize = sizeof(PKT_CREATE_EFFECT);
								pkt_ce->efType = EFFECT_TYPE_HIT_FONT;
								pkt_ce->EftAnitType = EFFECT_ANIMATION_TYPE_ONE;
								auto position = Objects_[i].GetPosition();
								position.y += 20.0f;
								pkt_ce->xmf3Position = position;
								pkt_ce->id = Objects_[object].get_owner_id();

								effect_lock_.lock();
								create_effect_queue_.push(pkt_ce);
								effect_lock_.unlock();

								if (Objects_[i].GetHitPoint() <= 0)
								{
									if (Objects_[i].get_team() == 0)
										red_score_ -= 5;
									else
										blue_score_ -= 5;
									PKT_SCORE* pkt_sco = new PKT_SCORE;
									pkt_sco->PktId = PKT_ID_SCORE;
									pkt_sco->PktSize = sizeof(PKT_SCORE);
									pkt_sco->RedScore = red_score_;
									pkt_sco->BlueScore = blue_score_;

									Objects_[i].SetHitPoint(Objects_[i].GetMaxHitPoint());

									PKT_PLAYER_DIE* pkt_pd = new PKT_PLAYER_DIE;
									pkt_pd->PktId = PKT_ID_PLAYER_DIE;
									pkt_pd->PktSize = sizeof(PKT_PLAYER_DIE);
									pkt_pd->respawntime = 5.0f;
									pkt_pd->id = i;

									Objects_[i].set_is_die(true);

									die_lock_.lock();
									player_die_queue_.push(pkt_pd);
									die_lock_.unlock();

									score_lock.lock();
									score_queue_.push(pkt_sco);
									score_lock.unlock();
								}

								life_lock.lock();
								player_life_queue_.push(pkt_pl);
								life_lock.unlock();

							}
							return true;
						}
					}
				}
			}
		}
	}

	return false;
}

bool Scene::check_collision_player(int object)
{
	FXMVECTOR origin = XMLoadFloat3(&Objects_[object].GetPrevPosition());
	FXMVECTOR direction = XMLoadFloat3(&Objects_[object].GetLook());
	float distance;

	for (int i = 0; i < MAX_CLIENT; ++i)
	{
		if (Objects_[i].GetPlay())
		{
			if (!Objects_[i].get_is_die())
			{
				for (auto playeraabb : Objects_[i].GetAABB())
				{
					if (playeraabb.Intersects(origin, direction, distance))
					{
						XMFLOAT3 length = Vector3::Subtract(Objects_[object].GetPosition(), Objects_[object].GetPrevPosition());
						float len = Vector3::Length(length);
						if (distance <= len)
						{
							if (i != Objects_[object].get_owner_id())
							{
								if (Objects_[object].GetObjectType() == OBJECT_TYPE_ITEM_AMMO_1 ||
									Objects_[object].GetObjectType() == OBJECT_TYPE_ITEM_AMMO_2)
								{
									PKT_PICK_ITEM* pkt_pi = new PKT_PICK_ITEM;
									pkt_pi->PktId = PKT_ID_PICK_ITEM;
									pkt_pi->PktSize = sizeof(PKT_PICK_ITEM);
									pkt_pi->ID = i;
									pkt_pi->AMMO = Objects_[object].GetHitPoint();
									if (Objects_[object].GetObjectType() == OBJECT_TYPE_ITEM_AMMO_1)
										pkt_pi->Item_type = ITEM_TYPE_AMMO1;
									else
										pkt_pi->Item_type = ITEM_TYPE_AMMO2;
									item_lock.lock();
									item_queue_.push(pkt_pi);
									item_lock.unlock();
								}
								else if (Objects_[object].GetObjectType() == OBJECT_TYPE_ITEM_HEALING)
								{
									PKT_PICK_ITEM* pkt_pi = new PKT_PICK_ITEM;
									pkt_pi->PktId = PKT_ID_PICK_ITEM;
									pkt_pi->PktSize = sizeof(PKT_PICK_ITEM);
									pkt_pi->ID = i;
									pkt_pi->HP = Objects_[object].GetHitPoint();
									pkt_pi->Item_type = ITEM_TYPE_HEALING;

									Objects_[i].SetHitPoint(Objects_[i].GetHitPoint() + 50);

									item_lock.lock();
									item_queue_.push(pkt_pi);
									item_lock.unlock();
								}
								else if (Objects_[object].GetObjectType() == OBJECT_TYPE_MACHINE_BULLET ||
									Objects_[object].GetObjectType() == OBJECT_TYPE_BEAM_BULLET ||
									Objects_[object].GetObjectType() == OBJECT_TYPE_BZK_BULLET)
								{
									PKT_PLAYER_LIFE* pkt_pl = new PKT_PLAYER_LIFE;
									pkt_pl->ID = i;
									Objects_[i].SetHitPoint(Objects_[i].GetHitPoint() - Objects_[object].GetHitPoint());
									pkt_pl->HP = Objects_[object].GetHitPoint();
									pkt_pl->PktId = PKT_ID_PLAYER_LIFE;
									pkt_pl->PktSize = sizeof(PKT_PLAYER_LIFE);

									PKT_CREATE_EFFECT* pkt_ce = new PKT_CREATE_EFFECT();
									pkt_ce->PktId = PKT_ID_CREATE_EFFECT;
									pkt_ce->PktSize = sizeof(PKT_CREATE_EFFECT);
									pkt_ce->efType = EFFECT_TYPE_HIT_FONT;
									pkt_ce->EftAnitType = EFFECT_ANIMATION_TYPE_ONE;
									auto position = Objects_[i].GetPosition();
									position.y += 20.0f;
									pkt_ce->xmf3Position = position;
									pkt_ce->id = Objects_[object].get_owner_id();

									effect_lock_.lock();
									create_effect_queue_.push(pkt_ce);
									effect_lock_.unlock();

									if (Objects_[i].GetHitPoint() <= 0)
									{
										if (Objects_[i].get_team() == 0)
											red_score_ -= 5;
										else
											blue_score_ -= 5;
										PKT_SCORE* pkt_sco = new PKT_SCORE;
										pkt_sco->PktId = PKT_ID_SCORE;
										pkt_sco->PktSize = sizeof(PKT_SCORE);
										pkt_sco->RedScore = red_score_;
										pkt_sco->BlueScore = blue_score_;

										Objects_[i].SetHitPoint(Objects_[i].GetMaxHitPoint());

										PKT_PLAYER_DIE* pkt_pd = new PKT_PLAYER_DIE;
										pkt_pd->PktId = PKT_ID_PLAYER_DIE;
										pkt_pd->PktSize = sizeof(PKT_PLAYER_DIE);
										pkt_pd->respawntime = 5.0f;
										pkt_pd->id = i;

										die_lock_.lock();
										player_die_queue_.push(pkt_pd);
										die_lock_.unlock();

										Objects_[i].set_is_die(true);

										score_lock.lock();
										score_queue_.push(pkt_sco);
										score_lock.unlock();
									}

									life_lock.lock();
									player_life_queue_.push(pkt_pl);
									life_lock.unlock();

								}
								else if (Objects_[object].GetObjectType() == OBJECT_TYPE_METEOR)
								{
									PKT_PLAYER_LIFE* pkt_pl = new PKT_PLAYER_LIFE;
									pkt_pl->ID = i;
									Objects_[i].SetHitPoint(Objects_[i].GetHitPoint() - Objects_[object].GetHitPoint());
									pkt_pl->HP = Objects_[object].GetHitPoint();
									pkt_pl->PktId = PKT_ID_PLAYER_LIFE;
									pkt_pl->PktSize = sizeof(PKT_PLAYER_LIFE);

									if (Objects_[i].GetHitPoint() <= 0)
									{
										if (Objects_[i].get_team() == 0)
											red_score_ -= 5;
										else
											blue_score_ -= 5;
										PKT_SCORE* pkt_sco = new PKT_SCORE;
										pkt_sco->PktId = PKT_ID_SCORE;
										pkt_sco->PktSize = sizeof(PKT_SCORE);
										pkt_sco->RedScore = red_score_;
										pkt_sco->BlueScore = blue_score_;

										Objects_[i].SetHitPoint(Objects_[i].GetMaxHitPoint());

										PKT_PLAYER_DIE* pkt_pd = new PKT_PLAYER_DIE;
										pkt_pd->PktId = PKT_ID_PLAYER_DIE;
										pkt_pd->PktSize = sizeof(PKT_PLAYER_DIE);
										pkt_pd->respawntime = 5.0f;
										pkt_pd->id = i;

										Objects_[i].set_is_die(true);

										die_lock_.lock();
										player_die_queue_.push(pkt_pd);
										die_lock_.unlock();

										score_lock.lock();
										score_queue_.push(pkt_sco);
										score_lock.unlock();
									}

									life_lock.lock();
									player_life_queue_.push(pkt_pl);
									life_lock.unlock();
								}
								return true;
							}
						}
					}
				}
			}
		}
	}
	return false;
}

bool Scene::check_collision_player_to_vector(int object, float len, float* dis)
{
	FXMVECTOR origin = XMLoadFloat3(&Objects_[object].GetPosition());
	FXMVECTOR direction = XMLoadFloat3(&Objects_[object].GetLook());
	float distance = 1000.0f;
	for (int i = 0; i < MAX_CLIENT; ++i)
	{
		if (Objects_[i].GetPlay())
		{
			if (!Objects_[i].get_is_die())
			{
				for (auto playeraabb : Objects_[i].GetAABB())
				{
					if (playeraabb.Intersects(origin, direction, distance))
					{
						if (Objects_[object].GetObjectType() == OBJECT_TYPE_BEAM_BULLET)
						{
							if (distance <= len)
							{
								*dis = distance;

								PKT_PLAYER_LIFE* pkt_pl = new PKT_PLAYER_LIFE;
								pkt_pl->ID = i;
								Objects_[i].SetHitPoint(Objects_[i].GetHitPoint() - Objects_[object].GetHitPoint());
								pkt_pl->HP = Objects_[object].GetHitPoint();
								pkt_pl->PktId = PKT_ID_PLAYER_LIFE;
								pkt_pl->PktSize = sizeof(PKT_PLAYER_LIFE);

								life_lock.lock();
								player_life_queue_.push(pkt_pl);
								life_lock.unlock();

								PKT_CREATE_EFFECT* pkt_ce = new PKT_CREATE_EFFECT();
								pkt_ce->PktId = PKT_ID_CREATE_EFFECT;
								pkt_ce->PktSize = sizeof(PKT_CREATE_EFFECT);
								pkt_ce->efType = EFFECT_TYPE_HIT_FONT;
								pkt_ce->EftAnitType = EFFECT_ANIMATION_TYPE_ONE;
								auto position = Objects_[i].GetPosition();
								position.y += 20.0f;
								pkt_ce->xmf3Position = position;
								pkt_ce->id = Objects_[object].get_owner_id();

								effect_lock_.lock();
								create_effect_queue_.push(pkt_ce);
								effect_lock_.unlock();

								PKT_CREATE_EFFECT* pkt_ce2 = new PKT_CREATE_EFFECT();
								pkt_ce2->PktId = PKT_ID_CREATE_EFFECT;
								pkt_ce2->PktSize = sizeof(PKT_CREATE_EFFECT);
								pkt_ce2->efType = EFFECT_TYPE_BEAM_HIT;
								pkt_ce2->EftAnitType = EFFECT_ANIMATION_TYPE_ONE;
								auto position2 = Objects_[i].GetPosition();
								position2.y += 9.0f;
								pkt_ce2->xmf3Position = position2;
								pkt_ce2->id = Objects_[object].get_owner_id();

								effect_lock_.lock();
								create_effect_queue_.push(pkt_ce2);
								effect_lock_.unlock();

								if (Objects_[i].GetHitPoint() <= 0)
								{
									if (Objects_[i].get_team() == 0)
										red_score_ -= 5;
									else
										blue_score_ -= 5;
									PKT_SCORE* pkt_sco = new PKT_SCORE;
									pkt_sco->PktId = PKT_ID_SCORE;
									pkt_sco->PktSize = sizeof(PKT_SCORE);
									pkt_sco->RedScore = red_score_;
									pkt_sco->BlueScore = blue_score_;

									Objects_[i].SetHitPoint(Objects_[i].GetMaxHitPoint());

									PKT_PLAYER_DIE* pkt_pd = new PKT_PLAYER_DIE;
									pkt_pd->PktId = PKT_ID_PLAYER_DIE;
									pkt_pd->PktSize = sizeof(PKT_PLAYER_DIE);
									pkt_pd->respawntime = 5.0f;
									pkt_pd->id = i;

									Objects_[i].set_is_die(true);

									die_lock_.lock();
									player_die_queue_.push(pkt_pd);
									die_lock_.unlock();

									score_lock.lock();
									score_queue_.push(pkt_sco);
									score_lock.unlock();
								}
								return true;
							}
						}
					}
				}
			}
		}
	}
	return false;
}

PKT_PICK_ITEM* Scene::item_dequeue()
{
	item_lock.lock();
	if (item_queue_.empty())
	{
		item_lock.unlock();
		return nullptr;
	}
	auto packet = item_queue_.front();
	item_queue_.pop();
	item_lock.unlock();
	return packet;
}

PKT_PLAYER_LIFE * Scene::player_life_dequeue()
{
	life_lock.lock();
	if (player_life_queue_.empty())
	{
		life_lock.unlock();
		return nullptr;
	}
	auto packet = player_life_queue_.front();
	player_life_queue_.pop();
	life_lock.unlock();
	return packet;
}

PKT_SCORE * Scene::score_dequeue()
{
	score_lock.lock();
	if (score_queue_.empty())
	{
		score_lock.unlock();
		return nullptr;
	}
	auto packet = score_queue_.front();
	score_queue_.pop();
	score_lock.unlock();
	return packet;
}

PKT_CREATE_EFFECT* Scene::create_effect_dequeue()
{
	effect_lock_.lock();
	if (create_effect_queue_.empty())
	{
		effect_lock_.unlock();
		return nullptr;
	}
	auto packet = create_effect_queue_.front();
	create_effect_queue_.pop();
	effect_lock_.unlock();
	return packet;
}

PKT_PLAYER_DIE * Scene::player_die_dequeue()
{
	die_lock_.lock();
	if (player_die_queue_.empty())
	{
		die_lock_.unlock();
		return nullptr;
	}
	auto packet = player_die_queue_.front();
	player_die_queue_.pop();
	die_lock_.unlock();
	return packet;
}

void Scene::start_event()
{
	is_being_event_ = true;
}

void Scene::end_event()
{
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
	//-30~0.5
	gravity_ = ((float)((rand() % 306) - 300)) / 10.0f;
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
	meteor_cooltime_ = 1.0f;
}

void SpaceScene::end_event()
{
	Scene::end_event();
}
