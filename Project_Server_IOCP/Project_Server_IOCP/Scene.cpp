#include "pch.h"
#include "Scene.h"
#include "GameObject.h"


Scene::Scene()
{
}


Scene::~Scene()
{
}

void Scene::animate_objects(float time_elapsed)
{
	elapsed_game_time_ += time_elapsed;

	for (int i = 0; i < MAX_CLIENT; ++i)
	{
		Objects_[i].animate(time_elapsed, 0);
	}
}

void Scene::inert_objects_from_file(char* file_name, int group)
{
	GameObject *object = new GameObject();

	FILE *file = NULL;
	::fopen_s(&file, file_name, "rb");
	if (!file) {
		std::cout << "lose bin file" << std::endl;
	}
	::rewind(file);

	char token[64] = { '\0' };
	UINT reads = 0;
	float loaded_token = 0;
	UINT cycle = 0;
	XMFLOAT3 pos_loader = XMFLOAT3(0, 0, 0);
	XMFLOAT3 rot_loader = XMFLOAT3(0, 0, 0);

	while (feof(file) == 0)
	{
		read_posrot_from_file(file, token);

		if (!strcmp(token, "m_value"))
		{
			switch (cycle) {
			case 0:
				reads = left_byte_from_file(file, 2);
				loaded_token = read_float_from_file(file);
				reads = left_byte_from_file(file, 1);
				pos_loader.x = loaded_token;
				++cycle;
				break;
			case 1:
				reads = left_byte_from_file(file, 2);
				loaded_token = read_float_from_file(file);
				reads = left_byte_from_file(file, 1);
				pos_loader.y = loaded_token;
				++cycle;
				break;
			case 2:
				reads = left_byte_from_file(file, 2);
				loaded_token = read_float_from_file(file);
				reads = left_byte_from_file(file, 1);
				pos_loader.z = loaded_token;
				++cycle;
				break;
			case 3:
				reads = left_byte_from_file(file, 2);
				loaded_token = read_float_from_file(file);
				reads = left_byte_from_file(file, 1);
				rot_loader.x = loaded_token;
				++cycle;
				break;
			case 4:
				reads = left_byte_from_file(file, 2);
				loaded_token = read_float_from_file(file);
				reads = left_byte_from_file(file, 1);
				rot_loader.y = loaded_token;
				++cycle;
				break;
			case 5:
				reads = left_byte_from_file(file, 2);
				loaded_token = read_float_from_file(file);
				reads = left_byte_from_file(file, 1);
				rot_loader.z = loaded_token;
				object = new GameObject();
				object->set_position(pos_loader);
				object->set_prepare_rotate(rot_loader.x, rot_loader.y, rot_loader.z);
				insert_object(object, group, true, NULL);
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

void Scene::init(Repository * repository)
{
	robot_mesh_ = repository->get_model("./Resource/PlayerCollisionBox.bin");
	bullet_mesh_ = repository->get_model("./Resource/Bullet/Bullet.bin");
	saber_mesh_ = repository->get_model("./Resource/SaberCollisionBox.bin");

	for (int i = 0; i < MAX_NUM_OBJECT; ++i)
	{
		Objects_[i].set_use(false);
	}

	for (int i = 0; i < MAX_CLIENT; ++i)
	{
		Objects_[i].set_object_type(OBJECT_TYPE_PLAYER);
		Objects_[i].set_id(i);
		Objects_[i].set_model(robot_mesh_);
		Objects_[i].set_max_hp(PLAYER_HP);
		Objects_[i].set_hp(PLAYER_HP);
		Objects_[i].set_play(false);
		Objects_[i].set_is_player(true);
		Objects_[i].set_is_die(false);
	}

	Objects_[0].set_world_matrix(XMFLOAT4X4{ 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f , 0.0f, 0.0f, 1.0f, 0.0f , 0.0f, 0.0f, -150.0f, 1.0f });
	Objects_[1].set_world_matrix(XMFLOAT4X4{ 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f , 0.0f, 0.0f, 1.0f, 0.0f , 0.0f, 0.0f, 150.0f, 1.0f });
	Objects_[2].set_world_matrix(XMFLOAT4X4{ 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f , 0.0f, 0.0f, 1.0f, 0.0f , 100.0f, 0.0f, -150.0f, 1.0f });
	Objects_[3].set_world_matrix(XMFLOAT4X4{ 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f , 0.0f, 0.0f, 1.0f, 0.0f , -100.0f, 0.0f, 150.0f, 1.0f });
	Objects_[4].set_world_matrix(XMFLOAT4X4{ 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f , 0.0f, 0.0f, 1.0f, 0.0f , 0.0f, 0.0f, -200.0f, 1.0f });
	Objects_[5].set_world_matrix(XMFLOAT4X4{ 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f , 0.0f, 0.0f, 1.0f, 0.0f , 0.0f, 0.0f, 200.0f, 1.0f });
	Objects_[6].set_world_matrix(XMFLOAT4X4{ 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f , 0.0f, 0.0f, 1.0f, 0.0f , -100.0f, 0.0f, -150.0f, 1.0f });
	Objects_[7].set_world_matrix(XMFLOAT4X4{ 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f , 0.0f, 0.0f, 1.0f, 0.0f , 100.0f, 0.0f, 150.0f, 1.0f });

	for (auto obstacle : Obstacles_)
		obstacle->animate(0.016f, 0);

	elapsed_game_time_ = 0.0f;
	event_time_ = 0.0f;
	is_being_event_ = false;
	alert_ = false;
}

void Scene::init()
{
	for (int i = MAX_CLIENT; i < MAX_NUM_OBJECT; ++i)
	{
		Objects_[i].object_delete();
		Objects_[i].set_use(false);
		Objects_[i].set_elapsed_time_to_zero();
	}

	for (int i = 0; i < MAX_CLIENT; ++i)
	{
		Objects_[i].set_hp(PLAYER_HP);
		Objects_[i].set_play(false);
		Objects_[i].set_is_die(false);
		Objects_[i].set_elapsed_time_to_zero();
	}

	Objects_[0].set_world_matrix(XMFLOAT4X4{ 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f , 0.0f, 0.0f, 1.0f, 0.0f , 0.0f, 0.0f, -150.0f, 1.0f });
	Objects_[1].set_world_matrix(XMFLOAT4X4{ 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f , 0.0f, 0.0f, 1.0f, 0.0f , 0.0f, 0.0f, 150.0f, 1.0f });
	Objects_[2].set_world_matrix(XMFLOAT4X4{ 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f , 0.0f, 0.0f, 1.0f, 0.0f , 100.0f, 0.0f, -150.0f, 1.0f });
	Objects_[3].set_world_matrix(XMFLOAT4X4{ 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f , 0.0f, 0.0f, 1.0f, 0.0f , -100.0f, 0.0f, 150.0f, 1.0f });
	Objects_[4].set_world_matrix(XMFLOAT4X4{ 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f , 0.0f, 0.0f, 1.0f, 0.0f , 0.0f, 0.0f, -200.0f, 1.0f });
	Objects_[5].set_world_matrix(XMFLOAT4X4{ 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f , 0.0f, 0.0f, 1.0f, 0.0f , 0.0f, 0.0f, 200.0f, 1.0f });
	Objects_[6].set_world_matrix(XMFLOAT4X4{ 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f , 0.0f, 0.0f, 1.0f, 0.0f , -100.0f, 0.0f, -150.0f, 1.0f });
	Objects_[7].set_world_matrix(XMFLOAT4X4{ 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f , 0.0f, 0.0f, 1.0f, 0.0f , 100.0f, 0.0f, 150.0f, 1.0f });

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
	while (kill_queue_.size() > 0)
		kill_queue_.pop();

	elapsed_game_time_ = 0.0f;
	event_time_ = 0.0f;
	is_being_event_ = false;
	alert_ = false;
}

void Scene::insert_object(GameObject * object, int group, bool prepare_rotate, void * context)
{
	object->set_model(models_[group]);
	if (prepare_rotate) object->add_prepare_rotate(0, 180, 0);

	Obstacles_.emplace_back(object);
}

int Scene::get_index()
{
	for (int i = MAX_CLIENT; i < MAX_NUM_OBJECT; ++i)
		if (Objects_[i].get_use() == false)
			return i;
	return -1;
}

int Scene::add_object(OBJECT_TYPE type, int hp, float life_time, float speed, XMFLOAT4X4 matrix, int id)
{
	obj_lock.lock();
	int index = get_index();
	Objects_[index].set_object_type(type);
	Objects_[index].set_max_hp(hp);
	Objects_[index].set_hp(hp);
	XMFLOAT4X4 worldmatrix = matrix;
	if (index != -1)
	{
		Objects_[index].set_index(index);
		if (type == OBJECT_TYPE_MACHINE_BULLET
			|| type == OBJECT_TYPE_BZK_BULLET
			|| type == OBJECT_TYPE_BEAM_BULLET)
		{
			Objects_[index].set_model(bullet_mesh_);
			Objects_[index].set_life(life_time);
			Objects_[index].set_speed(speed);
		}
		else if (type == OBJECT_TYPE_ITEM_HEALING
			|| type == OBJECT_TYPE_ITEM_AMMO
			|| type == OBJECT_TYPE_ITEM_AMMO_1
			|| type == OBJECT_TYPE_ITEM_AMMO_2)
		{
			Objects_[index].set_model(bullet_mesh_);
		}
		else if (type == OBJECT_TYPE_METEOR)
		{
			Objects_[index].set_model(bullet_mesh_);
			Objects_[index].set_life(life_time);
			Objects_[index].set_speed(speed);
		}
		else if (type == OBJECT_TYPE_SABER)
		{
			Objects_[index].set_model(saber_mesh_);
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
	Objects_[index].set_world_matrix(worldmatrix);
	Objects_[index].set_prev_position(XMFLOAT3{ worldmatrix._41, worldmatrix._42, worldmatrix._43 });
	Objects_[index].set_use(true);
	obj_lock.unlock();
	return index;
}

void Scene::set_player_team(int id, char team)
{
	Objects_[id].set_team(team);
}

bool Scene::check_collision_obstacles(int object)
{
	FXMVECTOR origin = XMLoadFloat3(&Objects_[object].get_prev_position());
	FXMVECTOR direction = XMLoadFloat3(&Objects_[object].get_look());
	float distance;

	for (auto obstacle : Obstacles_)
	{
		for (auto playeraabb : obstacle->get_aabbs())
		{
			if (playeraabb.Intersects(origin, direction, distance))
			{
				XMFLOAT3 length = Vector3::Subtract(Objects_[object].get_position(), Objects_[object].get_prev_position());
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
		if (Objects_[i].get_play())
		{
			if (!Objects_[i].get_is_die())
			{
				for (auto objaabb : Objects_[object].get_aabbs())
				{
					for (auto playeraabb : Objects_[i].get_aabbs())
					{
						if (playeraabb.Intersects(objaabb))
						{
							if (Objects_[object].get_object_type() == OBJECT_TYPE_SABER)
							{
								if (i == Objects_[object].get_owner_id()) continue;
								PKT_PLAYER_LIFE* pkt_pl = new PKT_PLAYER_LIFE;
								pkt_pl->ID = i;
								Objects_[i].set_hp(Objects_[i].get_hp() - Objects_[object].get_hp());
								pkt_pl->HP = Objects_[object].get_hp();
								pkt_pl->PktId = PKT_ID_PLAYER_LIFE;
								pkt_pl->PktSize = sizeof(PKT_PLAYER_LIFE);

								PKT_CREATE_EFFECT* pkt_ce = new PKT_CREATE_EFFECT();
								pkt_ce->PktId = PKT_ID_CREATE_EFFECT;
								pkt_ce->PktSize = sizeof(PKT_CREATE_EFFECT);
								pkt_ce->efType = EFFECT_TYPE_HIT_FONT;
								pkt_ce->EftAnitType = EFFECT_ANIMATION_TYPE_ONE;
								auto position = Objects_[i].get_position();
								position.y += 20.0f;
								pkt_ce->xmf3Position = position;
								pkt_ce->id = Objects_[object].get_owner_id();

								effect_lock_.lock();
								create_effect_queue_.push(pkt_ce);
								effect_lock_.unlock();

								if (Objects_[i].get_hp() <= 0)
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

									Objects_[i].set_hp(Objects_[i].get_max_hp());

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

									KILL_MESSAGE* km = new KILL_MESSAGE;
									km->die_id = i;
									km->kill_id = Objects_[object].get_owner_id();

									kill_lock_.lock();
									kill_queue_.push(km);
									kill_lock_.unlock();
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
	// 오브젝트의 이전위치
	FXMVECTOR origin = XMLoadFloat3(&Objects_[object].get_prev_position());
	// 오브젝트의 진행방향
	FXMVECTOR direction = XMLoadFloat3(&Objects_[object].get_look());
	float distance;

	for (int i = 0; i < MAX_CLIENT; ++i)
	{
		if (Objects_[i].get_play())
		{
			if (!Objects_[i].get_is_die())
			{
				for (auto playeraabb : Objects_[i].get_aabbs())
				{
					// 오브젝트의 이전위치에서 진행방향으로 광선을 쏘아 플레이어의 AABB와 충돌 검사를 한다.
					// 충돌검사 결과가 true이면 distance에 광선이 충돌된 위치까지의 거리가 반환된다.
					if (playeraabb.Intersects(origin, direction, distance))
					{
						// 오브젝트의 이전위치와 현재위치사이의 거리(len)를 구한다.
						XMFLOAT3 length = Vector3::Subtract(Objects_[object].get_position(), Objects_[object].get_prev_position());
						float len = Vector3::Length(length);

						// len이 distance보다 크다면 오브젝트의 이전위치와 현재위치 사이에 플레이어가 있었던것이므로 충돌처리를 한다.
						// distance가 len보다 크다면 오브젝트는 아직 플레이어와 충돌하지 않은 상태이다.
						if (distance <= len)
						{
							if (i != Objects_[object].get_owner_id())
							{
								if (Objects_[object].get_object_type() == OBJECT_TYPE_ITEM_AMMO_1 ||
									Objects_[object].get_object_type() == OBJECT_TYPE_ITEM_AMMO_2)
								{
									PKT_PICK_ITEM* pkt_pi = new PKT_PICK_ITEM;
									pkt_pi->PktId = PKT_ID_PICK_ITEM;
									pkt_pi->PktSize = sizeof(PKT_PICK_ITEM);
									pkt_pi->ID = i;
									pkt_pi->AMMO = Objects_[object].get_hp();
									if (Objects_[object].get_object_type() == OBJECT_TYPE_ITEM_AMMO_1)
										pkt_pi->Item_type = ITEM_TYPE_AMMO1;
									else
										pkt_pi->Item_type = ITEM_TYPE_AMMO2;
									item_lock.lock();
									item_queue_.push(pkt_pi);
									item_lock.unlock();
								}
								else if (Objects_[object].get_object_type() == OBJECT_TYPE_ITEM_HEALING)
								{
									PKT_PICK_ITEM* pkt_pi = new PKT_PICK_ITEM;
									pkt_pi->PktId = PKT_ID_PICK_ITEM;
									pkt_pi->PktSize = sizeof(PKT_PICK_ITEM);
									pkt_pi->ID = i;
									pkt_pi->HP = Objects_[object].get_hp();
									pkt_pi->Item_type = ITEM_TYPE_HEALING;

									Objects_[i].set_hp(Objects_[i].get_hp() + 50);

									item_lock.lock();
									item_queue_.push(pkt_pi);
									item_lock.unlock();
								}
								else if (Objects_[object].get_object_type() == OBJECT_TYPE_MACHINE_BULLET ||
									Objects_[object].get_object_type() == OBJECT_TYPE_BEAM_BULLET ||
									Objects_[object].get_object_type() == OBJECT_TYPE_BZK_BULLET)
								{
									PKT_PLAYER_LIFE* pkt_pl = new PKT_PLAYER_LIFE;
									pkt_pl->ID = i;
									Objects_[i].set_hp(Objects_[i].get_hp() - Objects_[object].get_hp());
									pkt_pl->HP = Objects_[object].get_hp();
									pkt_pl->PktId = PKT_ID_PLAYER_LIFE;
									pkt_pl->PktSize = sizeof(PKT_PLAYER_LIFE);

									PKT_CREATE_EFFECT* pkt_ce = new PKT_CREATE_EFFECT();
									pkt_ce->PktId = PKT_ID_CREATE_EFFECT;
									pkt_ce->PktSize = sizeof(PKT_CREATE_EFFECT);
									pkt_ce->efType = EFFECT_TYPE_HIT_FONT;
									pkt_ce->EftAnitType = EFFECT_ANIMATION_TYPE_ONE;
									auto position = Objects_[i].get_position();
									position.y += 20.0f;
									pkt_ce->xmf3Position = position;
									pkt_ce->id = Objects_[object].get_owner_id();

									effect_lock_.lock();
									create_effect_queue_.push(pkt_ce);
									effect_lock_.unlock();

									if (Objects_[i].get_hp() <= 0)
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

										Objects_[i].set_hp(Objects_[i].get_max_hp());

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

										KILL_MESSAGE* km = new KILL_MESSAGE;
										km->die_id = i;
										km->kill_id = Objects_[object].get_owner_id();

										kill_lock_.lock();
										kill_queue_.push(km);
										kill_lock_.unlock();
									}

									life_lock.lock();
									player_life_queue_.push(pkt_pl);
									life_lock.unlock();

								}
								else if (Objects_[object].get_object_type() == OBJECT_TYPE_METEOR)
								{
									PKT_PLAYER_LIFE* pkt_pl = new PKT_PLAYER_LIFE;
									pkt_pl->ID = i;
									Objects_[i].set_hp(Objects_[i].get_hp() - Objects_[object].get_hp());
									pkt_pl->HP = Objects_[object].get_hp();
									pkt_pl->PktId = PKT_ID_PLAYER_LIFE;
									pkt_pl->PktSize = sizeof(PKT_PLAYER_LIFE);

									if (Objects_[i].get_hp() <= 0)
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

										Objects_[i].set_hp(Objects_[i].get_max_hp());

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

										KILL_MESSAGE* km = new KILL_MESSAGE;
										km->die_id = i;
										km->kill_id = Objects_[object].get_owner_id();

										kill_lock_.lock();
										kill_queue_.push(km);
										kill_lock_.unlock();
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
	FXMVECTOR origin = XMLoadFloat3(&Objects_[object].get_position());
	FXMVECTOR direction = XMLoadFloat3(&Objects_[object].get_look());
	float distance = 1000.0f;
	for (int i = 0; i < MAX_CLIENT; ++i)
	{
		if (Objects_[i].get_play())
		{
			if (!Objects_[i].get_is_die())
			{
				for (auto playeraabb : Objects_[i].get_aabbs())
				{
					if (playeraabb.Intersects(origin, direction, distance))
					{
						if (Objects_[object].get_object_type() == OBJECT_TYPE_BEAM_BULLET)
						{
							if (distance <= len)
							{
								*dis = distance;

								PKT_PLAYER_LIFE* pkt_pl = new PKT_PLAYER_LIFE;
								pkt_pl->ID = i;
								Objects_[i].set_hp(Objects_[i].get_hp() - Objects_[object].get_hp());
								pkt_pl->HP = Objects_[object].get_hp();
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
								auto position = Objects_[i].get_position();
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
								auto position2 = Objects_[i].get_position();
								position2.y += 9.0f;
								pkt_ce2->xmf3Position = position2;
								pkt_ce2->id = Objects_[object].get_owner_id();

								effect_lock_.lock();
								create_effect_queue_.push(pkt_ce2);
								effect_lock_.unlock();

								if (Objects_[i].get_hp() <= 0)
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

									Objects_[i].set_hp(Objects_[i].get_max_hp());

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

									KILL_MESSAGE* km = new KILL_MESSAGE;
									km->die_id = i;
									km->kill_id = Objects_[object].get_owner_id();

									kill_lock_.lock();
									kill_queue_.push(km);
									kill_lock_.unlock();
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

KILL_MESSAGE* Scene::kill_dequeue()
{
	kill_lock_.lock();
	if (kill_queue_.empty())
	{
		kill_lock_.unlock();
		return nullptr;
	}
	auto kill = kill_queue_.front();
	kill_queue_.pop();
	kill_lock_.unlock();
	return kill;
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

void GroundScene::build_objects(Repository * repository)
{
	models_.emplace_back(repository->get_model("./Resource/Buildings/Hangar.bin"));
	inert_objects_from_file("./Resource/Buildings/B_HangarSelfData.bin", 0);

	models_.emplace_back(repository->get_model("./Resource/Buildings/Building_Double_Square.bin"));
	inert_objects_from_file("./Resource/Buildings/B_DoubleSquareSelfData.bin", 1);

	models_.emplace_back(repository->get_model("./Resource/Buildings/Building_Octagon.bin"));
	inert_objects_from_file("./Resource/Buildings/B_OctagonSelfData.bin", 2);

	models_.emplace_back(repository->get_model("./Resource/Buildings/Building_OctagonLongTier.bin"));
	inert_objects_from_file("./Resource/Buildings/B_OctagonLongTierSelfData.bin", 3);

	models_.emplace_back(repository->get_model("./Resource/Buildings/Building_Slope_top.bin"));
	inert_objects_from_file("./Resource/Buildings/B_Slope_TopSelfData.bin", 4);

	models_.emplace_back(repository->get_model("./Resource/Buildings/Building_Square.bin"));
	inert_objects_from_file("./Resource/Buildings/B_SquareSelfData.bin", 5);

	models_.emplace_back(repository->get_model("./Resource/Buildings/Building_Steeple_top.bin"));
	inert_objects_from_file("./Resource/Buildings/B_Steeple_TopSelfData.bin", 6);

	models_.emplace_back(repository->get_model("./Resource/Buildings/Wall.bin"));
	inert_objects_from_file("./Resource/Buildings/WallSelfData.bin", 7);

	models_.emplace_back(repository->get_model("./Resource/Buildings/fence.bin"));
	inert_objects_from_file("./Resource/Buildings/FenceSelfData.bin", 8);

	init(repository);
	gravity_ = -9.8f;
}

void GroundScene::animate_objects(float time_elapsed)
{
	Scene::animate_objects(time_elapsed);
}

void GroundScene::scene_event(float time_elapsed)
{
	event_time_ += time_elapsed;
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

void SpaceScene::build_objects(Repository * repository)
{
	models_.emplace_back(repository->get_model("./Resource/Buildings/Space/Astroids1.bin"));
	inert_objects_from_file("./Resource/Buildings/Space/S_Astroid_1SelfData.bin", 0);

	models_.emplace_back(repository->get_model("./Resource/Buildings/Space/Astroids2.bin"));
	inert_objects_from_file("./Resource/Buildings/Space/S_Astroid_2SelfData.bin", 1);

	models_.emplace_back(repository->get_model("./Resource/Buildings/Space/Astroids3_1.bin"));
	inert_objects_from_file("./Resource/Buildings/Space/S_Astroid_3SelfData.bin", 2);

	models_.emplace_back(repository->get_model("./Resource/Buildings/Space/Astroids3_2.bin"));
	inert_objects_from_file("./Resource/Buildings/Space/S_Astroid_3-1SelfData.bin", 3);

	models_.emplace_back(repository->get_model("./Resource/Buildings/Space/Astroids3_3.bin"));
	inert_objects_from_file("./Resource/Buildings/Space/S_Astroid_3-2SelfData.bin", 4);

	models_.emplace_back(repository->get_model("./Resource/Buildings/Space/Astroids4.bin"));
	inert_objects_from_file("./Resource/Buildings/Space/S_Astroid_4SelfData.bin", 5);

	models_.emplace_back(repository->get_model("./Resource/Buildings/Space/Astroids5.bin"));
	inert_objects_from_file("./Resource/Buildings/Space/S_Astroid_5SelfData.bin", 6);

	models_.emplace_back(repository->get_model("./Resource/Buildings/Space/SpaceShip.bin"));
	inert_objects_from_file("./Resource/Buildings/Space/S_SpaceShipSelfData.bin", 7);

	models_.emplace_back(repository->get_model("./Resource/Buildings/Space/StarShip_Light.bin"));
	inert_objects_from_file("./Resource/Buildings/Space/S_StarShipSelfData.bin", 8);

	init(repository);
	gravity_ = 0.0f;
}

void SpaceScene::animate_objects(float time_elapsed)
{
	Scene::animate_objects(time_elapsed);
}

void SpaceScene::scene_event(float time_elapsed)
{
	event_time_ += time_elapsed;
	meteor_cooltime_duration_ += time_elapsed;
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
