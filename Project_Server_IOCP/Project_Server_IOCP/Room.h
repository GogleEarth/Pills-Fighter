#pragma once

#include "Scene.h"
#include "GameObject.h"

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
	char slot_;
	wchar_t name_[MAX_NAME_LENGTH];
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
	inline void set_slot(char slot) { slot_ = slot; }
	inline char get_slot() { return slot_; }
	inline void set_name(wchar_t* name) { lstrcpynW(name_, name, MAX_NAME_LENGTH); }
	inline wchar_t* get_name() { return name_; }
};

class Room
{
	Player players_[8];
	bool slots_[8];
	Scene* scenes_[2];
	int	using_scene_;
	bool in_use_;
	bool is_playing_;
	std::queue<PKT_PLAYER_INFO*> player_info_queue_;
	std::queue<PKT_CREATE_OBJECT*> create_object_queue_;
	std::queue<PKT_MAP_EVENT*> map_event_queue_;
	char blue_score_;
	char red_score_;
	float item_cooltime_[3];
	bool item_spawn_[3];
	bool game_end_;
	wchar_t name_[MAX_ROOM_NAME_LENGTH];
public:
	Room();
	~Room();

	inline bool get_is_use() { return in_use_; }
	inline void set_is_use(bool use) { in_use_ = use; };

	inline void set_map(int map) { using_scene_ = map; }
	inline int get_map() { return using_scene_; }

	inline void set_blue_score(char score) { blue_score_ = score; }
	inline char get_blue_score() { return scenes_[using_scene_]->get_blue_score(); }

	inline void set_red_score(char score) { red_score_ = score; }
	inline char get_red_score() { return scenes_[using_scene_]->get_red_score(); }

	inline Player* get_players() { return players_; }
	inline bool get_playing() { return is_playing_; }

	inline GameObject* get_object(int id) { return scenes_[using_scene_]->get_object(id); }

	void set_player_slot(int id, char slot);
	inline char get_player_slot(int id) { return players_[id].get_slot(); }

	inline char get_player_team(int id) { return players_[id].get_team(); }

	inline void set_game_end(bool end) { game_end_ = end; }
	inline bool get_game_end() { return game_end_; }

	inline void set_name(wchar_t* name) { lstrcpynW(name_, name, MAX_ROOM_NAME_LENGTH); }
	inline wchar_t* get_name() { return name_; }

	int get_empty_slot();
	int get_empty_slot_red_team();
	int get_empty_slot_blue_team();

	XMFLOAT3 get_respawn_point(int id);
	XMFLOAT4X4 get_player_worldmatrix(int id);
	XMFLOAT4X4 make_matrix();

	void set_player_worldmatrix(int id, XMFLOAT4X4 matrix);
	void set_player_is_play(int id, bool play);
	void set_object_id(int id);

	int get_num_player_in_room();
	void init(Repository* repository);
	void init();
	void end_game();
	bool search_client(SOCKET client);
	void disconnect_client(SOCKET client);
	int findindex();
	void add_player(int id, SOCKET socket, char slot, wchar_t* name);
	int add_object(OBJECT_TYPE type, XMFLOAT4X4 matrix, int id = -1);
	void set_player_lobby_info(int id, char selectedrobot, char team, char slot);
	PKT_CREATE_OBJECT* shoot(int id, XMFLOAT4X4 matrix, WEAPON_TYPE weapon);
	PKT_CREATE_EFFECT* shoot(int id, XMFLOAT4X4 matrix, WEAPON_TYPE weapon, float len, int* index);
	void player_load_complete(SOCKET socket);
	void player_send_complete(int id);
	bool all_send_complete();
	bool all_load_complete();
	Player* get_player(int id);
	int find_player_by_socket(SOCKET client);
	void start_game();
	void room_update(float elapsed_time);
	void spawn_healing_item();
	void spawn_ammo_item();
	void change_team(int id, char team);

	void check_collision_obstacles(int object);
	void check_saber_collision_player(int object);
	void check_collision_player(int object);
	void check_collision_player_to_vector(int object, float len, float* dis);

	void player_info_inqueue(char* packet);
	PKT_PLAYER_INFO* player_info_dequeue();
	PKT_CREATE_OBJECT* create_object_dequeue();
	PKT_CREATE_EFFECT* create_effect_dequeue();
	PKT_MAP_EVENT* map_event_dequeue();
	PKT_PICK_ITEM* item_dequeue();
	PKT_PLAYER_LIFE* player_life_dequeue();
	PKT_SCORE* score_dequeue();
	PKT_PLAYER_DIE* player_die_dequeue();
	PKT_KILL_MESSAGE* kill_message_dequeue();
};

