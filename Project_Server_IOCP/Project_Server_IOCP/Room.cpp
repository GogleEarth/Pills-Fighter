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

XMFLOAT4X4 Room::get_player_worldmatrix(int id)
{
	return 	scenes_[using_scene_]->get_player_worldmatrix(id);
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

void Room::init(CRepository* repository)
{
	in_use_ = false;
	is_playing_ = false;
	using_scene_ = 3;
	blue_score_ = 0;
	red_score_ = 0;

	GroundScene* groundscene = new GroundScene();
	groundscene->init(repository);
	groundscene->BuildObjects(repository);
	SpaceScene* spacescene = new SpaceScene();
	spacescene->init(repository);
	spacescene->BuildObjects(repository);

	scenes_[0] = groundscene;
	scenes_[1] = spacescene;

	for (int i = 0; i < 8; ++i)
		players_[i].init();
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

void Room::add_player(int id, SOCKET socket)
{
	int num = findindex();
	players_[num].set_use(true);
	players_[num].set_serverid(id);
	players_[num].set_socket(socket);
	players_[num].set_robot(ROBOT_TYPE_GM);
}

void Room::set_player_lobby_info(int id, char selectedrobot, char team)
{
	players_[id].set_robot(selectedrobot);
	players_[id].set_team(team);
}

void Room::shoot(int id, XMFLOAT4X4 matrix, WEAPON_TYPE weapon)
{
	Bullet* object = new Bullet();
	object->SetWorldTransf(matrix);
	OBJECT_TYPE type;
	if (weapon == WEAPON_TYPE_MACHINE_GUN)
	{
		object->SetHitPoint(3);
		object->set_life(1.5f);
		object->set_speed(500.0f);
		type = OBJECT_TYPE_MACHINE_BULLET;
	}
	else if (weapon == WEAPON_TYPE_BAZOOKA)
	{
		object->SetHitPoint(15);
		object->set_life(2.5f);
		object->set_speed(400.0f);
		type = OBJECT_TYPE_BZK_BULLET;
	}
	else if (weapon == WEAPON_TYPE_BEAM_RIFLE)
	{
		object->SetHitPoint(5);
		object->set_life(0.75f);
		object->set_speed(600.0f);
		type = OBJECT_TYPE_BEAM_BULLET;
	}

	PKT_CREATE_OBJECT* pkt_co = new PKT_CREATE_OBJECT();
	pkt_co->PktId = PKT_ID_CREATE_OBJECT;
	pkt_co->PktSize = sizeof(PKT_CREATE_OBJECT);
	pkt_co->Robot_Type = ROBOT_TYPE_GM;
	pkt_co->WorldMatrix = matrix;
	pkt_co->Object_Type = type;
	object->SetObjectType(type);
	pkt_co->Object_Index = scenes_[using_scene_]->AddObject(object);

	create_object_queue_.push(pkt_co);
}

void Room::player_load_complete(int id)
{
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
		if (players_[i].get_socket() == client) ret = i;

	return ret;
}

int Room::get_players_in_room()
{
	int ret = 0;
	for (int i = 0; i < MAX_CLIENT; ++i)
	{
		if (!players_[i].get_use()) continue;
		ret += 1;
	}
	return ret;
}

void Room::start_game()
{
	is_playing_ = true;
	blue_score_ = MAX_SCORE;
	red_score_ = MAX_SCORE;
	using_scene_ -= 2;
}

void Room::room_update(float elapsed_time)
{
	scenes_[using_scene_]->AnimateObjects(elapsed_time);
	for (int i = MAX_CLIENT; i < MAX_NUM_OBJECT; ++i)
	{
		GameObject* object = scenes_[using_scene_]->get_object(i);
		if (object != NULL)
		{
			PKT_UPDATE_OBJECT* updateobj = new PKT_UPDATE_OBJECT();
			updateobj->PktId = (char)PKT_ID_UPDATE_OBJECT;
			updateobj->PktSize = (char)sizeof(PKT_UPDATE_OBJECT);
			updateobj->Object_Index = i;
			updateobj->Object_Position = object->GetPosition();
			update_object_queue_.push(updateobj);
			if (object->IsDelete())
			{
				PKT_DELETE_OBJECT* pkt_d = new PKT_DELETE_OBJECT();
				pkt_d->PktId = (char)PKT_ID_DELETE_OBJECT;
				pkt_d->PktSize = (char)sizeof(PKT_DELETE_OBJECT);
				pkt_d->Object_Index = i;
				delete_object_queue_.push(pkt_d);

				OBJECT_TYPE type = object->GetObjectType();
				if (type == OBJECT_TYPE_MACHINE_BULLET
					|| type == OBJECT_TYPE_BZK_BULLET
					|| type == OBJECT_TYPE_BEAM_BULLET)
				{
					PKT_CREATE_EFFECT* pktCE = new PKT_CREATE_EFFECT();
					pktCE->PktId = PKT_ID_CREATE_EFFECT;
					pktCE->PktSize = (char)sizeof(PKT_CREATE_EFFECT);
					if (type == OBJECT_TYPE_BZK_BULLET)
						pktCE->efType = EFFECT_TYPE_EXPLOSION;
					else
						pktCE->efType = EFFECT_TYPE_HIT;
					pktCE->EftAnitType = EFFECT_ANIMATION_TYPE_ONE;
					pktCE->xmf3Position = object->GetPosition();
					create_effect_queue_.push(pktCE);
				}
			}
		}
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

PKT_UPDATE_OBJECT* Room::update_object_dequeue()
{
	if (update_object_queue_.empty()) return nullptr;
	auto packet = update_object_queue_.front();
	update_object_queue_.pop();
	return packet;
}

PKT_DELETE_OBJECT* Room::delete_object_dequeue()
{
	if (delete_object_queue_.empty()) return nullptr;
	auto packet = delete_object_queue_.front();
	delete_object_queue_.pop();
	return packet;
}

PKT_CREATE_EFFECT* Room::create_effect_dequeue()
{
	if (create_effect_queue_.empty()) return nullptr;
	auto packet = create_effect_queue_.front();
	create_effect_queue_.pop();
	return packet;
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
}
