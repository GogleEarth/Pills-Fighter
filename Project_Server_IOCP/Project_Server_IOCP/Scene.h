#pragma once

#include "Repository.h"
#include "Protocol.h"
#include "GameObject.h"

#define EVENT_TIME_GROUND 30.0f
#define EVENT_START_INTERVAL_GROUND 90.0f
#define EVENT_TIME_SPACE 15.0f
#define EVENT_START_INTERVAL_SPACE 90.0f

class Scene
{
protected:
	CModel*	bullet_mesh_;
	CModel*	robot_mesh_;
	CModel* saber_mesh_;
	std::vector<CModel*> models_;

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

public:
	Scene();
	~Scene();

	virtual void BuildObjects(CRepository* pRepository) = 0;
	virtual void AnimateObjects(float fTimeElapsed);
	virtual void SceneEvent(float fTimeElapsed) = 0;

	virtual void start_event();
	virtual void end_event();

	void InsertObjectFromLoadInfFromBin(char *pstrFileName, int nGroup);

	float ReadFloatFromFile(FILE *pInFile) {
		float fValue = 0;
		UINT nReads = (UINT)::fread(&fValue, sizeof(float), 1, pInFile);
		return(fValue);
	}
	BYTE LeftByteFromFile(FILE *pInFile, int byte) {
		UINT nReads = 0;
		char waste[64] = { '\0' };
		nReads = (UINT)::fread(waste, sizeof(char), byte, pInFile);

		return(nReads);
	}
	BYTE ReadPosrotFromFile(FILE *pInFile, char *pstrToken) {
		BYTE nStrLength = 41;
		BYTE m_value = 7;
		UINT nReads = 0;
		nReads = (UINT)::fread(pstrToken, sizeof(char), nStrLength, pInFile);
		nReads = (UINT)::fread(pstrToken, sizeof(char), m_value, pInFile);
		pstrToken[m_value] = '\0';

		return(nReads);
	}

	void init(CRepository* pRepository);
	void init();
	void InsertObject(GameObject* pObject, int nGroup, bool bPrepareRotate, void *pContext);
	int GetIndex();
	int AddObject(OBJECT_TYPE type, int hp, float life_time, float speed, XMFLOAT4X4 matrix, int id = -1);
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


	inline void releaseObject(int index) { Objects_[index].SetUse(false); }
	inline void deleteObject(int index) { Objects_[index].Delete(); }

	inline void set_player_worldmatrix(int id, XMFLOAT4X4 matrix) { Objects_[id].SetWorldTransf(matrix); }
	inline void set_player_is_play(int id, bool play) { Objects_[id].SetPlay(play); }
	inline void set_object_id(int id) { Objects_[id].SetId(id); }
	inline XMFLOAT4X4 get_player_worldmatrix(int id) { return Objects_[id].GetWorldTransf(); }
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

	virtual void BuildObjects(CRepository* pRepository);
	virtual void AnimateObjects(float fTimeElapsed);
	virtual void SceneEvent(float fTimeElapsed);
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

	virtual void BuildObjects(CRepository* pRepository);
	virtual void AnimateObjects(float fTimeElapsed);
	virtual void SceneEvent(float fTimeElapsed);
	virtual void start_event();
	virtual void end_event();
	
	inline void init_meteor_cooltime_duration() { meteor_cooltime_duration_ = 0.0f; }
	inline float get_meteor_cooltime_duration() { return meteor_cooltime_duration_; }
	inline float get_meteor_cooltime() { return meteor_cooltime_; }
};