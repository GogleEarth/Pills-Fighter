#pragma once

#include "Scene.h"

#define COLONY 0
#define SPACE 1

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
	Scene* scenes_[2];
	int	using_scene_;
	bool in_use_;
	bool is_playing_;
	std::queue<PKT_PLAYER_INFO*> player_info_queue_;
	std::queue<PKT_CREATE_OBJECT*> create_object_queue_;
	std::queue<PKT_UPDATE_OBJECT*> update_object_queue_;
	std::queue<PKT_DELETE_OBJECT*> delete_object_queue_;
	std::queue<PKT_CREATE_EFFECT*> create_effect_queue_;

	char blue_score_;
	char red_score_;
	float item_cooltime_[3];
	bool item_spawn_[3];
public:
	Room();
	~Room();

	inline bool get_is_use() { return in_use_; }
	inline void set_is_use(bool use) { in_use_ = use; };
	inline void set_map(int map) { using_scene_ = map; }
	inline int get_map() { return using_scene_; }
	inline void set_blue_score(char score) { blue_score_ = score; }
	inline char get_blue_score() { return blue_score_; }
	inline void set_red_score(char score) { red_score_ = score; }
	inline char get_red_score() { return red_score_; }
	inline Player* get_players() { return players_; }
	inline bool get_playing() { return is_playing_; }

	XMFLOAT4X4 get_player_worldmatrix(int id);
	void set_player_worldmatrix(int id, XMFLOAT4X4 matrix);
	void set_player_is_play(int id, bool play);
	void set_object_id(int id);

	int get_num_player_in_room();
	void init(CRepository* repository);
	bool search_client(SOCKET client);
	void disconnect_client(SOCKET client);
	int findindex();
	void add_player(int id, SOCKET socket);
	void set_player_lobby_info(int id, char selectedrobot, char team);
	void shoot(int id, XMFLOAT4X4 matrix, WEAPON_TYPE weapon);
	void player_load_complete(int id);
	void player_send_complete(int id);
	bool all_send_complete();
	bool all_load_complete();
	Player* get_player(int id);
	int find_player_by_socket(SOCKET client);
	int get_players_in_room();
	void start_game();
	void room_update(float elapsed_time);
	void spawn_healing_item();
	void spawn_ammo_item();

	void player_info_inqueue(char* packet);
	PKT_PLAYER_INFO* player_info_dequeue();
	PKT_CREATE_OBJECT* create_object_dequeue();
	PKT_UPDATE_OBJECT* update_object_dequeue();
	PKT_DELETE_OBJECT* delete_object_dequeue();
	PKT_CREATE_EFFECT* create_effect_dequeue();
};

