#pragma once

#include "Scene.h"

class Player
{
	SOCKET socket_;
	bool used_;
	int serverid_;
	char robot_;
	char team_;
	bool load_;
	bool send_;

public:
	Player() {}
	~Player() {}
	void init();
	inline void set_socket(SOCKET s) { socket_ = s; }
	inline SOCKET get_socket() { return socket_; }
	inline void set_use(bool use) { used_ = use; }
	inline bool get_use() { return used_; }
	inline void set_serverid(int id) { serverid_ = id; }
	inline int get_serverid() { return serverid_; }
	inline void set_robot(char robot) { robot_ = robot; }
	inline char get_robot() { return robot_; }
	inline void set_load(bool load) { load_ = load; }
	inline bool get_load() { return load_; }
	inline void set_send(bool send) { send_ = send; }
	inline bool get_send() { return send_; }
	inline void set_team(char team) { team_ = team; }
	inline char get_team() { return team_; }
};

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

