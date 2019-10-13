#pragma once

#include "Repository.h"
#include "Protocol.h"
#include "GameObject.h"

#define EVENT_TIME_GROUND 30.0f
#define EVENT_START_INTERVAL_GROUND 90.0f
#define EVENT_TIME_SPACE 15.0f
#define EVENT_START_INTERVAL_SPACE 90.0f

struct KILL_MESSAGE 
{
	int kill_id;
	int die_id;
};

class Scene
{
protected:
	Model*	bullet_mesh_;
	Model*	robot_mesh_;
	Model* saber_mesh_;
	std::vector<Model*> models_;

	std::mutex obj_lock;
	GameObject Objects_[MAX_NUM_OBJECT];

	std::vector<GameObject*> Obstacles_;
	float elapsed_game_time_;
	float event_time_;
	bool is_being_event_;
	bool alert_;
	float gravity_;

	int red_score_;
	int blue_score_;

	std::mutex item_lock;
	std::queue<PKT_PICK_ITEM*> item_queue_;
	std::mutex life_lock;
	std::queue<PKT_PLAYER_LIFE*> player_life_queue_;
	std::mutex score_lock;
	std::queue<PKT_SCORE*> score_queue_;
	std::mutex effect_lock_;
	std::queue<PKT_CREATE_EFFECT*> create_effect_queue_;
	std::mutex die_lock_;
	std::queue<PKT_PLAYER_DIE*> player_die_queue_;
	std::mutex kill_lock_;
	std::queue<KILL_MESSAGE*> kill_queue_;

public:
	Scene();
	virtual ~Scene();

	virtual void build_objects(Repository* repository) = 0;
	virtual void animate_objects(float time_elapsed);
	virtual void scene_event(float time_elapsed) = 0;

	virtual void start_event();
	virtual void end_event();

	void inert_objects_from_file(char *file_name, int group);

	float read_float_from_file(FILE *file) {
		float value = 0;
		UINT reads = (UINT)::fread(&value, sizeof(float), 1, file);
		return(value);
	}
	BYTE left_byte_from_file(FILE *file, int byte) {
		UINT reads = 0;
		char waste[64] = { '\0' };
		reads = (UINT)::fread(waste, sizeof(char), byte, file);

		return(reads);
	}
	BYTE read_posrot_from_file(FILE *file, char *token) {
		BYTE str_len = 41;
		BYTE value = 7;
		UINT reads = 0;
		reads = (UINT)::fread(token, sizeof(char), str_len, file);
		reads = (UINT)::fread(token, sizeof(char), value, file);
		token[value] = '\0';

		return(reads);
	}

	void init(Repository* repository);
	void init();
	void insert_object(GameObject* object, int group, bool prepare_rotate, void *context);
	int get_index();
	int add_object(OBJECT_TYPE type, int hp, float life_time, float speed, XMFLOAT4X4 matrix, int id = -1);
	void set_player_team(int id, char team);

	bool check_collision_obstacles(int object);
	bool check_saber_collision_player(int object);
	bool check_collision_player(int object);
	bool check_collision_player_to_vector(int object, float len, float* dis);

	PKT_PICK_ITEM* item_dequeue();
	PKT_PLAYER_LIFE* player_life_dequeue();
	PKT_SCORE* score_dequeue();
	PKT_CREATE_EFFECT* create_effect_dequeue();
	PKT_PLAYER_DIE* player_die_dequeue();
	KILL_MESSAGE* kill_dequeue();

	inline void release_object(int index) { Objects_[index].set_use(false); }
	inline void delete_object(int index) { Objects_[index].object_delete(); }

	inline void set_player_worldmatrix(int id, XMFLOAT4X4 matrix) { Objects_[id].set_world_matrix(matrix); }
	inline void set_player_is_play(int id, bool play) { Objects_[id].set_play(play); }
	inline void set_object_id(int id) { Objects_[id].set_id(id); }
	inline XMFLOAT4X4 get_player_worldmatrix(int id) { return Objects_[id].get_world_matrix(); }
	inline GameObject* get_object(int id) { return &Objects_[id]; }
	inline float get_elapsed_game_time() { return elapsed_game_time_; }
	inline float get_event_time() { return event_time_; }
	inline bool get_is_being_event() { return is_being_event_; }
	inline void set_alert(bool alert) { alert_ = alert; }
	inline bool get_alert() { return alert_; }
	inline float get_gravity() { return gravity_; }
	inline void set_red_score(int score) { red_score_ = score; }
	inline void set_blue_score(int score) { blue_score_ = score; }
	inline int get_red_score() { return red_score_; }
	inline int get_blue_score() { return blue_score_; }

};

///////////////////////////////////////////////////////////////////////////////////////////////////

class GroundScene : public Scene
{
public:
	GroundScene() {}
	~GroundScene() {}

	virtual void build_objects(Repository* repository);
	virtual void animate_objects(float time_elapsed);
	virtual void scene_event(float time_elapsed);
	virtual void start_event();
	virtual void end_event();
};

//////////////////////////////////////////////////////////////////////////////////////////////////

class SpaceScene : public Scene
{
	float meteor_cooltime_;
	float meteor_cooltime_duration_;
public:
	SpaceScene() {}
	~SpaceScene() {}

	virtual void build_objects(Repository* repository);
	virtual void animate_objects(float time_elapsed);
	virtual void scene_event(float time_elapsed);
	virtual void start_event();
	virtual void end_event();
	
	inline void init_meteor_cooltime_duration() { meteor_cooltime_duration_ = 0.0f; }
	inline float get_meteor_cooltime_duration() { return meteor_cooltime_duration_; }
	inline float get_meteor_cooltime() { return meteor_cooltime_; }
};