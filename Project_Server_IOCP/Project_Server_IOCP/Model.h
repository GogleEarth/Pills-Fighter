#pragma once

#include"Mesh.h"

////////////////////////////////////////////////////////////////////////////////

class Model
{
protected:
	int references_ = 0;

	Mesh* meshes_ = NULL;
	Cube_mesh* cube_meshes_ = NULL;

	int	materials_;

	XMFLOAT4X4 to_parent_matrix_;
	XMFLOAT4X4 world_matrix_;

	Model* parent_ = NULL;
	Model* sibling_ = NULL;

	char model_name_[64] = { 0 };

	int	num_meshes_ = 0;

	const char *file_name_ = NULL;

public:
	Model *child_ = NULL;

	Model();
	~Model();

	void add_ref();
	void release();

	void set_child(Model *child, bool add_reference = false);
	void set_to_parent_matrix(XMFLOAT4X4 to_parent_matrix) { to_parent_matrix_ = to_parent_matrix; }
	XMFLOAT4X4 get_world_matrix() { return world_matrix_; }
	void set_mesh(Mesh *mesh, Cube_mesh *cube_mesh, bool Is_skinned);
	Mesh* get_meshes() { return meshes_; }

	void update_collision_box(std::vector<BoundingBox>& aabbs, int *index);
	void Model::update_world_transform(XMFLOAT4X4 *parent);
	void get_num_meshes(int *standard_meshes, int *skinned_meshes);
	int get_num_meshes() { return num_meshes_; }

	const char* get_file_name() { return file_name_; };
	void set_model_mesh_count(int meshes, int skinned_meshes) { num_meshes_ = meshes; }
	void set_file_name(const char *file_name) { file_name_ = file_name; }

	XMFLOAT3 get_look() { return XMFLOAT3(world_matrix_._31, world_matrix_._32, world_matrix_._33); }
	XMFLOAT3 get_up() { return XMFLOAT3(world_matrix_._21, world_matrix_._22, world_matrix_._23); }
	XMFLOAT3 get_right() { return XMFLOAT3(world_matrix_._11, world_matrix_._12, world_matrix_._13); }
	XMFLOAT3 get_position() { return XMFLOAT3(world_matrix_._41, world_matrix_._42, world_matrix_._43); }

	static Model* load_geometry_and_animation_from_file(char *file_name);
	static Model* load_model_from_file(FILE *file, const char *file_name, const char *file_path);
};