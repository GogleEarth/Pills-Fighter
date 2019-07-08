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
}

void Room::set_player_lobby_info(int id, char selectedrobot, char team)
{
	players_[id].set_robot(selectedrobot);
	players_[id].set_team(team);
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

void Room::player_info_inqueue(char* packet)
{
	player_info_queue.push(reinterpret_cast<PKT_PLAYER_INFO*>(packet));
}

void Player::init()
{
	socket_ = -1;
	used_ = false;
	serverid_ = -1;
	robot_ = -1;
	team_ = -1;
	load_ = false;
	send_ = false;
}
