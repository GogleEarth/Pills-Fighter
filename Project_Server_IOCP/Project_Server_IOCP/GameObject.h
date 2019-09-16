#pragma once

#include "Protocol.h"
#include "Model.h"
#include "Mesh.h"

class GameObject
{
protected:
	XMFLOAT4X4 world_matrix_;

	XMFLOAT3 position_;
	XMFLOAT3 prev_position_;
	XMFLOAT3 right_;
	XMFLOAT3 up_;
	XMFLOAT3 look_;

	OBJECT_TYPE	object_type_;

	int id_;
	int index_;

	float pitch_;
	float yaw_;
	float roll_;

	float prepare_pitch_;
	float prepate_yaw_;
	float prepare_roll;

	bool in_used_ = false;
	bool play_ = false;

	Model *model_ = NULL;
	std::vector<BoundingBox> aabbs_;

	bool delete_ = false;
	int	num_meshes_ = 0;

	int	hp_;
	int	max_hp_;

	float moving_speed_;
	float duration_time_;
	float elapsed_time_;

	char team_;
	int owner_id_;
	
	bool is_player_;
	bool is_die_;
	float respawn_time_;
public:
	GameObject();
	~GameObject();

	void set_model(Model *model);
	void set_mesh(Mesh *mesh, Cube_mesh *cube_mesh);

	void animate(float time_elapsed, char map);
	void on_prepare();

	bool collsion_check(GameObject* object);

	//게임 객체의 월드 변환 행렬에서 위치 벡터와 방향(x-축, y-축, z-축) 벡터를 반환한다. 
	inline void set_look(XMFLOAT3 look) { look_ = look; }
	inline void set_up(XMFLOAT3 up) { up_ = up; }
	inline void set_right(XMFLOAT3 right) { right_ = right; }

	void set_world_matrix(XMFLOAT4X4& world);
	XMFLOAT4X4 get_world_matrix();

	inline XMFLOAT3 get_position() { return position_; }
	inline XMFLOAT3 get_prev_position() { return prev_position_; }
	inline XMFLOAT3 get_look() { return look_; }
	inline XMFLOAT3 get_up() { return up_; }
	inline XMFLOAT3 get_right() { return right_; }

	void set_position(float x, float y, float z);
	void set_position(XMFLOAT3& position);
	void set_prev_position(XMFLOAT3& position);

	inline void set_prepare_rotate(float pitch, float yaw, float roll) { prepare_pitch_ = pitch; prepate_yaw_ = yaw; prepare_roll = roll; }
	inline void add_prepare_rotate(float pitch, float yaw, float roll) { prepare_pitch_ += pitch; prepate_yaw_ += yaw; prepare_roll += roll; }

	void move_strafe(float distance);
	void move_up(float distance);
	void move_forward(float distance);
	void move(XMFLOAT3 direction, float distance);

	void rotate(float pitch, float yaw, float roll);

	void update_world_transform();

	inline std::vector<BoundingBox>& get_aabbs() { return aabbs_; }
	inline int get_num_aabbs() { return aabbs_.size(); }

	inline void object_delete() { delete_ = true; }
	inline bool is_delete() { return delete_; }
	inline void callback_position() { position_ = prev_position_; }

	inline void set_max_hp(int max_hp) { max_hp_ = max_hp; }
	inline int get_max_hp() { return max_hp_; }

	void set_hp(int hp);
	inline int get_hp() { return hp_; }

	inline void set_id(int id) { id_ = id; }
	inline int get_id() { return id_; }

	inline void set_index(int index) { index_ = index; }
	inline int get_index() { return index_; }

	void set_use(bool use);
	inline bool get_use() { return in_used_; }

	inline void set_play(bool play) { play_ = play; }
	inline bool get_play() { return play_; }

	inline void set_object_type(OBJECT_TYPE type) { object_type_ = type; }
	inline OBJECT_TYPE get_object_type() { return object_type_; }

	inline void set_speed(float speed) { moving_speed_ = speed; }
	inline float get_speed() { return moving_speed_; }

	inline void set_life(float life) { duration_time_ = life; }

	inline void set_team(char team) { team_ = team; }
	inline char get_team() { return team_; }

	inline void set_owner_id(int id) { owner_id_ = id; }
	inline int get_owner_id() { return owner_id_; }

	inline void set_is_player(bool is_player) { is_player_ = is_player; }
	inline bool get_is_player() { return is_player_; }

	inline void set_is_die(bool is_die) { is_die_ = is_die; }
	inline bool get_is_die() { return is_die_; }

	inline void set_elapsed_time_to_zero() { elapsed_time_ = 0.0f; }

};