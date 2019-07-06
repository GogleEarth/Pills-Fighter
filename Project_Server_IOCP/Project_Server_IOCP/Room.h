#pragma once

#include "Scene.h"

class Player{};

class Room
{
	Player players_[8];
	Scene scenes_[2];
	int	using_scene_;
	bool in_use_;
	std::queue<PKT_PLAYER_INFO*> player_info_queue;
public:
	Room();
	~Room();

	inline bool get_is_use() { return in_use_; }
	inline void set_is_use(bool use) { in_use_ = use; };

	void init(CRepository* repository);
	bool search_client(SOCKET client);
	void disconnect_client(SOCKET client);
	int findindex();
	void add_player(int id);
	void set_player_lobby_info(int id, char selectedrobot, char team);
	void shoot(int id);
	void player_load_complete(int id);
	void player_send_complete(int id);
	bool all_send_complete();
	bool all_load_complete();
	Player* get_player(int id);
	int find_player_by_socket(SOCKET client);

	void player_info_inqueue(char* packet);
};

