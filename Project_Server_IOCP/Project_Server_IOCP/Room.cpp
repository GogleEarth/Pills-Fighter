#include "pch.h"
#include "Room.h"
#include "Protocol.h"
#include "Scene.h"


Room::Room()
{
}


Room::~Room()
{
}

void Room::set_player_slot(int id, char slot)
{
	if (!slots_[slot])
	{
		players_[id].set_slot(slot);
		slots_[slot] = true;
	}
	else
	{
		int empty_slot = get_empty_slot();
		players_[id].set_slot(empty_slot);
		slots_[empty_slot] = true;
	}
}

int Room::get_empty_slot()
{
	for (int i = 0; i < 8; ++i)
	{
		if (!slots_[i])
		{
			return i;
		}
	}

	return -1;
}

int Room::get_empty_slot_red_team()
{
	if (slots_[0] == false)
		return 0;
	if (slots_[2] == false)
		return 2;
	if (slots_[4] == false)
		return 4;
	if (slots_[6] == false)
		return 6;
	return -1;
}

int Room::get_empty_slot_blue_team()
{
	if (slots_[1] == false)
		return 1;
	if (slots_[3] == false)
		return 3;
	if (slots_[5] == false)
		return 5;
	if (slots_[7] == false)
		return 7;
	return -1;
}

XMFLOAT3 Room::get_respawn_point(int id)
{
	XMFLOAT3 point;

	if (using_scene_ == 0)
	{
		switch (players_[id].get_slot())
		{
		case 0:
			point = XMFLOAT3{ -700, 0, -725 };
			break;
		case 1:
			point = XMFLOAT3{ 610, 0, 800 };
			break;
		case 2:
			point = XMFLOAT3{ -735, 0, -725 };
			break;
		case 3:
			point = XMFLOAT3{ 690, 0, 800 };
			break;
		case 4:
			point = XMFLOAT3{ -635, 0, -725 };
			break;
		case 5:
			point = XMFLOAT3{ 530, 0, 800 };
			break;
		case 6:
			point = XMFLOAT3{ -700, 0, -770 };
			break;
		case 7:
			point = XMFLOAT3{ 610, 0, 850 };
			break;
		default:
			break;
		}
	}
	else
	{
		switch (players_[id].get_slot())
		{
		case 0:
			point = XMFLOAT3{ -525, 0, -800 };
			break;
		case 1:
			point = XMFLOAT3{ 525, 0, 800 };
			break;
		case 2:
			point = XMFLOAT3{ -600, 0, -800 };
			break;
		case 3:
			point = XMFLOAT3{ 615, 0, 800 };
			break;
		case 4:
			point = XMFLOAT3{ -455, 0, -800 };
			break;
		case 5:
			point = XMFLOAT3{ 440, 0, 800 };
			break;
		case 6:
			point = XMFLOAT3{ -525, 0, -850 };
			break;
		case 7:
			point = XMFLOAT3{ 525, 0, 850 };
			break;
		default:
			break;
		}
	}


	return point;
}

XMFLOAT4X4 Room::get_player_worldmatrix(int id)
{
	return 	scenes_[using_scene_]->get_player_worldmatrix(id);
}

XMFLOAT4X4 Room::make_matrix()
{
	XMFLOAT4X4 matrix;

	XMFLOAT3 position;
	float randomangle = (float)(rand() % 36000) / 100.0f;
	position.x = sin(randomangle) * 1000.0f;
	randomangle = (float)(rand() % 36000) / 100.0f;
	position.y = sin(randomangle) * 1000.0f;
	randomangle = (float)(rand() % 36000) / 100.0f;
	position.z = sin(randomangle) * 1000.0f;

	XMFLOAT3 look;
	look = Vector3::Subtract(XMFLOAT3{ 0.0f,0.0f,0.0f }, position);
	look = Vector3::Normalize(look);

	XMFLOAT3 up = { 0.0f, 1.0f, 0.0f };

	XMFLOAT3 right = Vector3::CrossProduct(look, up, true);

	up = Vector3::CrossProduct(look, right, true);

	matrix._11 = right.x; matrix._12 = right.y; matrix._13 = right.z; matrix._34 = 0.0f;
	matrix._21 = up.x; matrix._22 = up.y; matrix._23 = up.z; matrix._24 = 0.0f;
	matrix._31 = look.x; matrix._32 = look.y; matrix._33 = look.z; matrix._14 = 0.0f;
	matrix._41 = position.x; matrix._42 = position.y; matrix._43 = position.z; matrix._44 = 1.0f;
	return matrix;
}

void Room::set_player_worldmatrix(int id, XMFLOAT4X4 matrix)
{
	scenes_[using_scene_]->set_player_worldmatrix(id, matrix);
}

void Room::set_player_is_play(int id, bool play)
{
	scenes_[using_scene_]->set_player_is_play(id, play);
}

void Room::set_object_id(int id)
{
	scenes_[using_scene_]->set_object_id(id);
}

int Room::get_num_player_in_room()
{
	int num = 0;
	for (int i = 0; i < 8; ++i)
	{
		if (players_[i].get_use())
			num += 1;
	}
	return num;
}

void Room::init(Repository* repository)
{
	in_use_ = false;
	is_playing_ = false;
	game_end_ = false;
	using_scene_ = 4;
	blue_score_ = 0;
	red_score_ = 0;

	GroundScene* groundscene = new GroundScene();
	groundscene->build_objects(repository);
	SpaceScene* spacescene = new SpaceScene();
	spacescene->build_objects(repository);

	scenes_[0] = groundscene;
	scenes_[1] = spacescene;

	for (int i = 0; i < 8; ++i)
		players_[i].init();
	for (int i = 0; i < 8; ++i)
		slots_[i] = false;
}

void Room::init()
{
	in_use_ = false;
	is_playing_ = false;
	game_end_ = false;
	using_scene_ = 4;
	blue_score_ = 0;
	red_score_ = 0;

	while (create_object_queue_.size() > 0)
		create_object_queue_.pop();
	while (map_event_queue_.size() > 0)
		map_event_queue_.pop();

	scenes_[0]->init();
	scenes_[1]->init();

	for (int i = 0; i < 8; ++i)
		players_[i].init();

	std::cout << "방 준비 완료\n";
}

void Room::end_game()
{
	is_playing_ = false;
	using_scene_ = 4;
	blue_score_ = 0;
	red_score_ = 0;

	for (int i = 0; i < MAX_CLIENT; ++i)
		players_[i].set_robot(0);

	while (create_object_queue_.size() > 0)
		create_object_queue_.pop();
	while (map_event_queue_.size() > 0)
		map_event_queue_.pop();

	scenes_[0]->init();
	scenes_[1]->init();

	std::cout << "방 준비 완료\n";
}

bool Room::search_client(SOCKET client)
{
	bool ret = false;
	
	for (int i = 0; i < 8; ++i)
	{
		if (!players_[i].get_use()) continue;
		if (players_[i].get_socket() == client) ret = true;
	}

	return ret;
}

void Room::disconnect_client(SOCKET client)
{
	int index = find_player_by_socket(client);
	players_[index].set_use(false);
	slots_[players_[index].get_slot()] = false;
}

int Room::findindex()
{
	int num = -1;
	for (int i = 0; i < 8; ++i)
	{
		if (players_[i].get_use()) continue;
		num = i;
		break;
	}
	return num;
}

void Room::add_player(int id, SOCKET socket, char slot)
{
	int num = findindex();
	players_[num].set_use(true);
	players_[num].set_serverid(id);
	players_[num].set_socket(socket);
	players_[num].set_robot(ROBOT_TYPE_GM);
	players_[num].set_slot(slot);
	players_[num].set_team((char)((int)slot % 2));
	slots_[slot] = true;
}

int Room::add_object(OBJECT_TYPE type, XMFLOAT4X4 matrix, int id)
{
	int hp;
	float lifetime;
	float speed;
	if (type == OBJECT_TYPE_METEOR)
	{
		hp = 30;
		lifetime = 30.0f;
		speed = 300.0f;
	}
	if (type == OBJECT_TYPE_SABER)
	{
		hp = 25;
		lifetime = 0.0f;
		speed = 0.0f;
	}
	return scenes_[using_scene_]->add_object(type, hp, lifetime, speed, matrix, id);
}

void Room::set_player_lobby_info(int id, char selectedrobot, char team, char slot)
{
	players_[id].set_robot(selectedrobot);
	slots_[players_[id].get_slot()] = false;
	players_[id].set_slot(slot);
	slots_[slot] = true;
}

PKT_CREATE_OBJECT* Room::shoot(int id, XMFLOAT4X4 matrix, WEAPON_TYPE weapon)
{
	int hp;
	float life_time;
	float speed;
	OBJECT_TYPE type;
	if (weapon == WEAPON_TYPE_MACHINE_GUN)
	{
		hp = 10;
		life_time = 1.5f;
		speed = 1200.0f;
		type = OBJECT_TYPE_MACHINE_BULLET;
	}
	else if (weapon == WEAPON_TYPE_BAZOOKA)
	{
		hp = 35;
		life_time = 2.5f;
		speed = 800.0f;
		type = OBJECT_TYPE_BZK_BULLET;
	}

	PKT_CREATE_OBJECT* pkt_co = new PKT_CREATE_OBJECT();
	pkt_co->PktId = PKT_ID_CREATE_OBJECT;
	pkt_co->PktSize = sizeof(PKT_CREATE_OBJECT);
	pkt_co->Robot_Type = ROBOT_TYPE_GM;
	pkt_co->WorldMatrix = matrix;
	pkt_co->Object_Type = type;
	pkt_co->Object_Index = scenes_[using_scene_]->add_object(type, hp, life_time, speed, matrix, id);

	return pkt_co;
}

PKT_CREATE_EFFECT * Room::shoot(int id, XMFLOAT4X4 matrix, WEAPON_TYPE weapon, float len, int* index)
{
	int hp;
	float life_time;
	float speed;
	float length;
	OBJECT_TYPE type;
	XMFLOAT3 look = XMFLOAT3{ matrix._31, matrix._32, matrix._33 };
	XMFLOAT3 position = XMFLOAT3{ matrix._41, matrix._42, matrix._43 };
	EFFECT_TYPE eftype;
	if (weapon == WEAPON_TYPE_BEAM_RIFLE)
	{
		hp = 25;
		life_time = 0.001f;
		speed = 600.0;
		type = OBJECT_TYPE_BEAM_BULLET;
		length = 1000.0f;
		eftype = EFFECT_TYPE_BEAM_RIFLE;
	}
	else if (weapon == WEAPON_TYPE_GM_GUN)
	{
		hp = 4;
		life_time = 0.001f;
		speed = 600.0;
		type = OBJECT_TYPE_BEAM_BULLET;
		length = 600.0f;
		eftype = EFFECT_TYPE_GM_GUN;
	}
	else if (weapon == WEAPON_TYPE_BEAM_SNIPER)
	{
		hp = 50;
		life_time = 0.001f;
		speed = 600.0;
		type = OBJECT_TYPE_BEAM_BULLET;
		length = 2000.0f;
		eftype = EFFECT_TYPE_BEAM_SNIPER;
	}

	PKT_CREATE_EFFECT* pkt_ce = new PKT_CREATE_EFFECT();
	pkt_ce->PktId = PKT_ID_CREATE_EFFECT;
	pkt_ce->PktSize = sizeof(PKT_CREATE_EFFECT);
	pkt_ce->EftAnitType = EFFECT_ANIMATION_TYPE_ONE;
	pkt_ce->efType = eftype;
	pkt_ce->xmf3Look = look;
	pkt_ce->xmf3Position = position;
	pkt_ce->fDistance = length;

	*index = scenes_[using_scene_]->add_object(type, hp, life_time, speed, matrix, id);

	return pkt_ce;
}

void Room::player_load_complete(SOCKET socket)
{
	int id = find_player_by_socket(socket);
	players_[id].set_load(true);
}

void Room::player_send_complete(int id)
{
	players_[id].set_send(true);
}

bool Room::all_send_complete()
{
	for (int i = 0; i < 8; ++i)
	{
		if (!players_[i].get_use()) continue;
		if (!players_[i].get_send()) return false;
	}
	return true;
}

bool Room::all_load_complete()
{
	for (int i = 0; i < 8; ++i)
	{
		if (!players_[i].get_use()) continue;
		if (!players_[i].get_load()) return false;
	}
	return true;
}

Player* Room::get_player(int id)
{
	return &players_[id];
}

int Room::find_player_by_socket(SOCKET client)
{
	int ret = -1;

	for (int i = 0; i < 8; ++i)
		if(players_[i].get_use())
			if (players_[i].get_socket() == client) ret = i;

	return ret;
}

void Room::start_game()
{
	is_playing_ = true;
	blue_score_ = MAX_SCORE;
	red_score_ = MAX_SCORE;

	if (get_map() == 4)
		using_scene_ = 0;
	else if (get_map() == 5)
		using_scene_ = 1;

	for (int i = 0; i < MAX_CLIENT; ++i)
	{
		if (!players_[i].get_use()) continue;
		scenes_[using_scene_]->set_player_team(i, players_[i].get_team());
	}

	scenes_[using_scene_]->set_red_score(red_score_);
	scenes_[using_scene_]->set_blue_score(blue_score_);

	for (int i = 0; i < 3; ++i)
	{
		item_cooltime_[i] = 0.0f;
		item_spawn_[i] = false;
	}
}

void Room::room_update(float elapsed_time)
{
	for (int i = 0; i < 3; ++i)
		if(!item_spawn_[i])
			item_cooltime_[i] += elapsed_time;
	
	spawn_healing_item();
	spawn_ammo_item();

	scenes_[using_scene_]->animate_objects(elapsed_time);

	if (using_scene_ == COLONY)
	{
		if (scenes_[using_scene_]->get_elapsed_game_time() >= EVENT_START_INTERVAL_GROUND - 5.0f &&
			!scenes_[using_scene_]->get_alert())
		{
			PKT_MAP_EVENT* pkt_mev = new PKT_MAP_EVENT();
			pkt_mev->PktId = PKT_ID_MAP_EVENT;
			pkt_mev->PktSize = sizeof(PKT_MAP_EVENT);
			pkt_mev->type = MAP_EVENT_TYPE_ALERT;
			map_event_queue_.push(pkt_mev);
			scenes_[using_scene_]->set_alert(true);
		}

		if (scenes_[using_scene_]->get_elapsed_game_time() >= EVENT_START_INTERVAL_GROUND &&
			!scenes_[using_scene_]->get_is_being_event())
		{
			PKT_MAP_EVENT* pkt_mev = new PKT_MAP_EVENT();
			pkt_mev->PktId = PKT_ID_MAP_EVENT;
			pkt_mev->PktSize = sizeof(PKT_MAP_EVENT);
			pkt_mev->type = MAP_EVENT_TYPE_START;
			scenes_[using_scene_]->start_event();
			pkt_mev->gravity = scenes_[using_scene_]->get_gravity();
			map_event_queue_.push(pkt_mev);
		}


		if (scenes_[using_scene_]->get_elapsed_game_time() >= EVENT_START_INTERVAL_GROUND + EVENT_TIME_GROUND &&
			scenes_[using_scene_]->get_is_being_event())
		{
			PKT_MAP_EVENT* pkt_mev = new PKT_MAP_EVENT();
			pkt_mev->PktId = PKT_ID_MAP_EVENT;
			pkt_mev->PktSize = sizeof(PKT_MAP_EVENT);
			pkt_mev->type = MAP_EVENT_TYPE_END;
			scenes_[using_scene_]->end_event();
			pkt_mev->gravity = scenes_[using_scene_]->get_gravity();
			map_event_queue_.push(pkt_mev);
		}
	}
	else
	{
		if (scenes_[using_scene_]->get_elapsed_game_time() >= EVENT_START_INTERVAL_SPACE - 5.0f &&
			!scenes_[using_scene_]->get_alert())
		{
			PKT_MAP_EVENT* pkt_mev = new PKT_MAP_EVENT();
			pkt_mev->PktId = PKT_ID_MAP_EVENT;
			pkt_mev->PktSize = sizeof(PKT_MAP_EVENT);
			pkt_mev->type = MAP_EVENT_TYPE_ALERT;
			map_event_queue_.push(pkt_mev);
			scenes_[using_scene_]->set_alert(true);
		}

		if (scenes_[using_scene_]->get_elapsed_game_time() >= EVENT_START_INTERVAL_SPACE &&
			!scenes_[using_scene_]->get_is_being_event())
		{
			PKT_MAP_EVENT* pkt_mev = new PKT_MAP_EVENT();
			pkt_mev->PktId = PKT_ID_MAP_EVENT;
			pkt_mev->PktSize = sizeof(PKT_MAP_EVENT);
			pkt_mev->type = MAP_EVENT_TYPE_START;
			scenes_[using_scene_]->start_event();
			pkt_mev->gravity = scenes_[using_scene_]->get_gravity();
			map_event_queue_.push(pkt_mev);
		}

		if (scenes_[using_scene_]->get_is_being_event())
		{
			scenes_[using_scene_]->scene_event(elapsed_time);
			if (((SpaceScene*)scenes_[using_scene_])->get_meteor_cooltime_duration() >= ((SpaceScene*)scenes_[using_scene_])->get_meteor_cooltime())
			{
				((SpaceScene*)scenes_[using_scene_])->init_meteor_cooltime_duration();
				PKT_CREATE_OBJECT* pkt_co = new PKT_CREATE_OBJECT();
				pkt_co->PktId = PKT_ID_CREATE_OBJECT;
				pkt_co->PktSize = sizeof(PKT_CREATE_OBJECT);
				pkt_co->Object_Type = OBJECT_TYPE_METEOR;
				pkt_co->WorldMatrix = make_matrix();
				create_object_queue_.push(pkt_co);
			}
		}

		if (scenes_[using_scene_]->get_elapsed_game_time() >= EVENT_START_INTERVAL_SPACE + EVENT_TIME_SPACE &&
			scenes_[using_scene_]->get_is_being_event())
		{
			PKT_MAP_EVENT* pkt_mev = new PKT_MAP_EVENT();
			pkt_mev->PktId = PKT_ID_MAP_EVENT;
			pkt_mev->PktSize = sizeof(PKT_MAP_EVENT);
			pkt_mev->type = MAP_EVENT_TYPE_END;
			scenes_[using_scene_]->end_event();
			pkt_mev->gravity = scenes_[using_scene_]->get_gravity();
			map_event_queue_.push(pkt_mev);
		}
	}

}

void Room::spawn_healing_item()
{
	if (item_cooltime_[0] >= 60.0f && !item_spawn_[0])
	{
		item_spawn_[0] = true;
		int hp = 50;
		XMFLOAT4X4 matrix = XMFLOAT4X4{ 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f , 0.0f, 0.0f, 1.0f, 0.0f , 0.0f, 5.0f, 0.0f, 1.0f };
		PKT_CREATE_OBJECT* pkt_co = new PKT_CREATE_OBJECT();
		pkt_co->PktId = PKT_ID_CREATE_OBJECT;
		pkt_co->PktSize = sizeof(PKT_CREATE_OBJECT);
		pkt_co->WorldMatrix = matrix;
		pkt_co->Object_Type = OBJECT_TYPE_ITEM_HEALING;
		pkt_co->Object_Index = scenes_[using_scene_]->add_object(OBJECT_TYPE_ITEM_HEALING, hp, 0.0f, 0.0f, matrix);

		create_object_queue_.push(pkt_co);
	}
}

void Room::spawn_ammo_item()
{
	if (item_cooltime_[1] >= 20.0f && !item_spawn_[1])
	{
		item_spawn_[1] = true;
		int hp = 100;
		XMFLOAT4X4 matrix = XMFLOAT4X4{ 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f , 0.0f, 0.0f, 1.0f, 0.0f , 100.0f, 5.0f, 0.0f, 1.0f };
		PKT_CREATE_OBJECT* pkt_co = new PKT_CREATE_OBJECT();
		pkt_co->PktId = PKT_ID_CREATE_OBJECT;
		pkt_co->PktSize = sizeof(PKT_CREATE_OBJECT);
		pkt_co->WorldMatrix = matrix;
		pkt_co->Object_Type = OBJECT_TYPE_ITEM_AMMO;
		pkt_co->Object_Index = scenes_[using_scene_]->add_object(OBJECT_TYPE_ITEM_AMMO_1, hp, 0.0f, 0.0f, matrix);

		create_object_queue_.push(pkt_co);
	}

	if (item_cooltime_[2] >= 20.0f && !item_spawn_[2])
	{
		item_spawn_[2] = true;
		int hp = 100;
		XMFLOAT4X4 matrix = XMFLOAT4X4{ 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f , 0.0f, 0.0f, 1.0f, 0.0f , -100.0f, 5.0f, 0.0f, 1.0f };
		PKT_CREATE_OBJECT* pkt_co = new PKT_CREATE_OBJECT();
		pkt_co->PktId = PKT_ID_CREATE_OBJECT;
		pkt_co->PktSize = sizeof(PKT_CREATE_OBJECT);
		pkt_co->WorldMatrix = matrix;
		pkt_co->Object_Type = OBJECT_TYPE_ITEM_AMMO;
		pkt_co->Object_Index = scenes_[using_scene_]->add_object(OBJECT_TYPE_ITEM_AMMO_2, hp, 0.0f, 0.0f, matrix);

		create_object_queue_.push(pkt_co);
	}
}

void Room::change_team(int id, char team)
{
	if (players_[id].get_team() != team)
	{
		if (team == TEAM_TYPE_RED)
		{
			int slot = get_empty_slot_red_team();
			slots_[players_[id].get_slot()] = false;
			slots_[slot] = true;
			players_[id].set_slot(slot);
			players_[id].set_team(0);
		}
		else if (team == TEAM_TYPE_BLUE)
		{
			int slot = get_empty_slot_blue_team();
			slots_[players_[id].get_slot()] = false;
			slots_[slot] = true;
			players_[id].set_slot(slot);
			players_[id].set_team(1);
		}
	}
}

void Room::check_collision_obstacles(int object)
{
	if (scenes_[using_scene_]->check_collision_obstacles(object))
	{
		scenes_[using_scene_]->delete_object(object);
	}
}

void Room::check_collision_player(int object)
{
	if (scenes_[using_scene_]->check_collision_player(object))
	{
		scenes_[using_scene_]->delete_object(object);
	}
}

void Room::check_saber_collision_player(int object)
{
	if (scenes_[using_scene_]->check_saber_collision_player(object))
	{
		scenes_[using_scene_]->delete_object(object);
	}
}

void Room::check_collision_player_to_vector(int object, float len, float* dis)
{
	if (scenes_[using_scene_]->check_collision_player_to_vector(object, len, dis))
	{
		scenes_[using_scene_]->delete_object(object);
	}
}

void Room::player_info_inqueue(char* packet)
{
	player_info_queue_.push(reinterpret_cast<PKT_PLAYER_INFO*>(packet));
}

PKT_PLAYER_INFO* Room::player_info_dequeue()
{
	if (player_info_queue_.empty()) return nullptr;
	auto packet = player_info_queue_.front();
	player_info_queue_.pop();
	return packet;
}

PKT_CREATE_OBJECT* Room::create_object_dequeue()
{
	if (create_object_queue_.empty()) return nullptr;
	auto packet = create_object_queue_.front();
	create_object_queue_.pop();
	return packet;
}

PKT_CREATE_EFFECT* Room::create_effect_dequeue()
{
	return scenes_[using_scene_]->create_effect_dequeue();
}

PKT_MAP_EVENT* Room::map_event_dequeue()
{
	if (map_event_queue_.empty()) return nullptr;
	auto packet = map_event_queue_.front();
	map_event_queue_.pop();
	return packet;
}

PKT_PICK_ITEM* Room::item_dequeue()
{
	auto item = scenes_[using_scene_]->item_dequeue();
	if (item != nullptr)
	{
		if (item->Item_type == ITEM_TYPE_HEALING)
		{
			item_cooltime_[0] = 0.0f;
			item_spawn_[0] = false;
		}
		else if (item->Item_type == ITEM_TYPE_AMMO1)
		{
			item_cooltime_[1] = 0.0f;
			item_spawn_[1] = false;
		}
		else if (item->Item_type == ITEM_TYPE_AMMO2)
		{
			item_cooltime_[2] = 0.0f;
			item_spawn_[2] = false;
		}
	}
	return item;
}

PKT_PLAYER_LIFE* Room::player_life_dequeue()
{
	return scenes_[using_scene_]->player_life_dequeue();
}

PKT_SCORE* Room::score_dequeue()
{
	return scenes_[using_scene_]->score_dequeue();
}

PKT_PLAYER_DIE* Room::player_die_dequeue()
{
	return scenes_[using_scene_]->player_die_dequeue();
}

void Player::init()
{
	socket_ = -1;
	used_ = false;
	serverid_ = -1;
	robot_ = ROBOT_TYPE_GM;
	team_ = -1;
	load_ = false;
	send_ = false;
	slot_ = -1;
}
