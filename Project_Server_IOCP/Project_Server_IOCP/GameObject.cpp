#include "pch.h"
#include "GameObject.h"


GameObject::GameObject()
{
	world_matrix_ = Matrix4x4::Identity();
	right_ = XMFLOAT3(1.0f, 0.0f, 0.0f);
	up_ = XMFLOAT3(0.0f, 1.0f, 0.0f);
	look_ = XMFLOAT3(0.0f, 0.0f, 1.0f);
	position_ = XMFLOAT3(0.0f, 0.0f, 0.0f);

	pitch_ = 0.0f;
	roll_ = 0.0f;
	yaw_ = 0.0f;

	set_prepare_rotate(0.0f, 0.0f, 0.0f);

	elapsed_time_ = 0.0f;
	respawn_time_ = 5.0f;
	is_die_ = false;
	is_player_ = false;
}


GameObject::~GameObject()
{
}

void GameObject::set_model(Model * model)
{
	if (model_) model_->release();

	if (model)
	{
		model_ = model;
		model_->add_ref();

		aabbs_.clear();

		num_meshes_ = model->get_num_meshes();
		for (int i = 0; i < num_meshes_; i++) aabbs_.emplace_back(BoundingBox());
	}
}

void GameObject::set_mesh(Mesh * mesh, Cube_mesh * cube_mesh)
{
	if (!model_)
	{
		Model *pModel = new Model();
		pModel->set_mesh(mesh, cube_mesh, false);

		set_model(pModel);
	}
	else
		model_->set_mesh(mesh, cube_mesh, false);
}

void GameObject::animate(float time_elapsed, char map)
{
	if (object_type_ == OBJECT_TYPE_MACHINE_BULLET
		|| object_type_ == OBJECT_TYPE_BZK_BULLET
		|| object_type_ == OBJECT_TYPE_BEAM_BULLET)
	{
		if (position_.y <= 0.0f && map == 0)
		{
			object_delete();
		}
		else if (elapsed_time_ >= duration_time_)
		{
			object_delete();
		}
		else
		{
			move_forward(moving_speed_ * time_elapsed);
			elapsed_time_ += time_elapsed;
		}
	}
	else if (object_type_ == OBJECT_TYPE_METEOR)
	{
		if (elapsed_time_ >= duration_time_)
		{
			object_delete();
		}
		else
		{
			move_forward(moving_speed_ * time_elapsed);
			elapsed_time_ += time_elapsed;
		}
	}
	else if (object_type_ == OBJECT_TYPE_SABER)
	{
		object_delete();
	}

	if (is_player_)
	{
		if (is_die_)
		{
			elapsed_time_ += time_elapsed;
			if (elapsed_time_ >= respawn_time_)
			{
				is_die_ = false;
				elapsed_time_ = 0.0f;
			}
		}
	}

	if (model_)
	{
		on_prepare();
		update_world_transform();
		int i = 0;
		model_->update_collision_box(aabbs_, &i);
	}
}

void GameObject::on_prepare()
{
	world_matrix_._11 = right_.x;
	world_matrix_._12 = right_.y;
	world_matrix_._13 = right_.z;
	world_matrix_._21 = up_.x;
	world_matrix_._22 = up_.y;
	world_matrix_._23 = up_.z;
	world_matrix_._31 = look_.x;
	world_matrix_._32 = look_.y;
	world_matrix_._33 = look_.z;
	world_matrix_._41 = position_.x;
	world_matrix_._42 = position_.y;
	world_matrix_._43 = position_.z;

	XMMATRIX rotate = XMMatrixRotationRollPitchYaw(XMConvertToRadians(prepare_pitch_), XMConvertToRadians(prepate_yaw_), XMConvertToRadians(prepare_roll));
	world_matrix_ = Matrix4x4::Multiply(rotate, world_matrix_);
}

bool GameObject::collsion_check(GameObject* object)
{
	auto aabb = (object->get_aabbs())[0];

	if (!object->is_delete())
	{
		if (aabbs_[0].Intersects(aabb))
			return true;
	}

	return false;
}

void GameObject::set_world_matrix(XMFLOAT4X4& world)
{
	right_ = XMFLOAT3(world._11, world._12, world._13);
	up_ = XMFLOAT3(world._21, world._22, world._23);
	look_ = XMFLOAT3(world._31, world._32, world._33);
	position_ = XMFLOAT3(world._41, world._42, world._43);
}

XMFLOAT4X4 GameObject::get_world_matrix()
{
	XMFLOAT4X4 world;
	world = XMFLOAT4X4{
		right_.x,		right_.y,		right_.z, 0.0f,
		up_.x,			up_.y,			up_.z, 0.0f,
		look_.x,		look_.y,		look_.z, 0.0f,
		position_.x,	position_.y,	position_.z, 0.0f,
	};

	return world;
}

void GameObject::set_position(float x, float y, float z)
{
	prev_position_.x = position_.x;
	prev_position_.y = position_.y;
	prev_position_.z = position_.z;

	position_.x = x;
	position_.y = y;
	position_.z = z;
}

void GameObject::set_position(XMFLOAT3 & position)
{
	set_position(position.x, position.y, position.z);
}

void GameObject::set_prev_position(XMFLOAT3 & position)
{
	prev_position_.x = position.x;
	prev_position_.y = position.y;
	prev_position_.z = position.z;
}

void GameObject::move_strafe(float distance)
{
	XMFLOAT3 position = get_position();
	XMFLOAT3 right = get_right();
	position = Vector3::Add(position, right, distance);
	set_position(position);
}

void GameObject::move_up(float distance)
{
	XMFLOAT3 position = get_position();
	XMFLOAT3 up = get_up();
	position = Vector3::Add(position, up, distance);
	set_position(position);
}

void GameObject::move_forward(float distance)
{
	XMFLOAT3 position = get_position();
	XMFLOAT3 look = get_look();
	position = Vector3::Add(position, look, distance);
	set_position(position);
}

void GameObject::move(XMFLOAT3 direction, float distance)
{
	XMFLOAT3 position = get_position();
	position = Vector3::Add(position, direction, distance);
	set_position(position);
}

void GameObject::rotate(float pitch, float yaw, float roll)
{
	if (pitch != 0.0f)
	{
		XMMATRIX rotate = XMMatrixRotationAxis(XMLoadFloat3(&right_),
			XMConvertToRadians(pitch));
		look_ = Vector3::TransformNormal(look_, rotate);
		up_ = Vector3::TransformNormal(up_, rotate);
	}
	if (yaw != 0.0f)
	{
		XMMATRIX rotate = XMMatrixRotationAxis(XMLoadFloat3(&up_),
			XMConvertToRadians(yaw));
		look_ = Vector3::TransformNormal(look_, rotate);
		right_ = Vector3::TransformNormal(right_, rotate);
	}
	if (roll != 0.0f)
	{
		XMMATRIX rotate = XMMatrixRotationAxis(XMLoadFloat3(&look_),
			XMConvertToRadians(roll));
		right_ = Vector3::TransformNormal(right_, rotate);
		up_ = Vector3::TransformNormal(up_, rotate);
	}

	look_ = Vector3::Normalize(look_);
	right_ = Vector3::CrossProduct(up_, look_, true);
	up_ = Vector3::CrossProduct(look_, right_, true);
}

void GameObject::update_world_transform()
{
	if (model_) model_->update_world_transform(&world_matrix_);
}

void GameObject::set_hp(int hp)
{
	if (hp >= max_hp_)
		hp_ = max_hp_;
	else
		hp_ = hp;
}

void GameObject::set_use(bool use)
{
	 in_used_ = use; 
	 if (use)
	 {
		 delete_ = false;
		 elapsed_time_ = 0.0f;
	 }
}
