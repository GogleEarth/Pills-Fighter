#pragma once

#include "Repository.h"
#include "Protocol.h"
#include "GameObject.h"

#define EVENT_TIME_GROUND 10.0f
#define EVENT_START_INTERVAL_GROUND 180.0f
#define EVENT_TIME_SPACE 30.0f
#define EVENT_START_INTERVAL_SPACE 300.0f

class Scene
{
protected:
	CModel*	bullet_mesh_;
	CModel*	robot_mesh_;
	std::vector<CModel*> models_;
	GameObject Objects_[MAX_NUM_OBJECT];
	std::vector<GameObject*> Obstacles_;
	GameObject BeamsaberCollisionmesh_[24];
	float elapsed_game_time_;
	float event_time_;
	bool is_being_event_;
	bool alert_;
	float gravity_;
public:
	Scene();
	~Scene();

	virtual void BuildObjects(CRepository* pRepository) = 0;
	virtual void AnimateObjects(float fTimeElapsed);
	virtual void SceneEvent(float fTimeElapsed) = 0;

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
	int AddObject(OBJECT_TYPE type, int hp, float life_time, float speed, XMFLOAT4X4 matrix);

	bool check_collision_obstacles(int object);
	bool check_collision_player(int object);

	virtual void start_event();
	virtual void end_event();

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
public:
	SpaceScene() {}
	~SpaceScene() {}

	virtual void BuildObjects(CRepository* pRepository);
	virtual void AnimateObjects(float fTimeElapsed);
	virtual void SceneEvent(float fTimeElapsed) {}

};