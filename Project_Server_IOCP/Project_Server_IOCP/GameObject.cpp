#include "pch.h"
#include "GameObject.h"


GameObject::GameObject()
{
	m_xmf4x4World = Matrix4x4::Identity();
	m_xmf3Right = XMFLOAT3(1.0f, 0.0f, 0.0f);
	m_xmf3Up = XMFLOAT3(0.0f, 1.0f, 0.0f);
	m_xmf3Look = XMFLOAT3(0.0f, 0.0f, 1.0f);
	m_xmf3Position = XMFLOAT3(0.0f, 0.0f, 0.0f);

	m_fPitch = 0.0f;
	m_fRoll = 0.0f;
	m_fYaw = 0.0f;

	SetPrepareRotate(0.0f, 0.0f, 0.0f);

	ElapsedTime_ = 0.0f;
	respawn_time_ = 5.0f;
	is_die_ = false;
	is_player_ = false;
}


GameObject::~GameObject()
{
}

void GameObject::SetModel(CModel * pModel)
{
	if (model_) model_->Release();

	if (pModel)
	{
		model_ = pModel;
		model_->AddRef();

		aabb_.clear();

		num_meshes_ = pModel->GetMeshes();
		for (int i = 0; i < num_meshes_; i++) aabb_.emplace_back(BoundingBox());
	}
}

void GameObject::SetMesh(CMesh * pMesh, CCubeMesh * pCubeMesh)
{
	if (!model_)
	{
		CModel *pModel = new CModel();
		pModel->SetMesh(pMesh, pCubeMesh, false);

		SetModel(pModel);
	}
	else
		model_->SetMesh(pMesh, pCubeMesh, false);
}

void GameObject::Animate(float fTimeElapsed, char map)
{
	if (Object_Type_ == OBJECT_TYPE_MACHINE_BULLET
		|| Object_Type_ == OBJECT_TYPE_BZK_BULLET
		|| Object_Type_ == OBJECT_TYPE_BEAM_BULLET)
	{
		if (m_xmf3Position.y <= 0.0f && map == 0)
		{
			Delete();
		}
		else if (ElapsedTime_ >= DurationTime_)
		{
			Delete();
		}
		else
		{
			MoveForward(MovingSpeed_ * fTimeElapsed);
			ElapsedTime_ += fTimeElapsed;
		}
	}
	else if (Object_Type_ == OBJECT_TYPE_METEOR)
	{
		if (ElapsedTime_ >= DurationTime_)
		{
			Delete();
		}
		else
		{
			MoveForward(MovingSpeed_ * fTimeElapsed);
			ElapsedTime_ += fTimeElapsed;
		}
	}
	else if (Object_Type_ == OBJECT_TYPE_SABER)
	{
		Delete();
	}

	if (is_player_)
	{
		if (is_die_)
		{
			ElapsedTime_ += fTimeElapsed;
			//std::cout << 5.0f - ElapsedTime_ << "초 후 부활\n";
			if (ElapsedTime_ >= respawn_time_)
			{
				is_die_ = false;
				ElapsedTime_ = 0.0f;
			}
		}
	}

	if (model_)
	{
		OnPrepareRender();
		UpdateWorldTransform();
		int i = 0;
		model_->UpdateCollisionBox(aabb_, &i);
	}
}

void GameObject::OnPrepareRender()
{
	m_xmf4x4World._11 = m_xmf3Right.x;
	m_xmf4x4World._12 = m_xmf3Right.y;
	m_xmf4x4World._13 = m_xmf3Right.z;
	m_xmf4x4World._21 = m_xmf3Up.x;
	m_xmf4x4World._22 = m_xmf3Up.y;
	m_xmf4x4World._23 = m_xmf3Up.z;
	m_xmf4x4World._31 = m_xmf3Look.x;
	m_xmf4x4World._32 = m_xmf3Look.y;
	m_xmf4x4World._33 = m_xmf3Look.z;
	m_xmf4x4World._41 = m_xmf3Position.x;
	m_xmf4x4World._42 = m_xmf3Position.y;
	m_xmf4x4World._43 = m_xmf3Position.z;

	XMMATRIX mtxRotate = XMMatrixRotationRollPitchYaw(XMConvertToRadians(m_fPreparePitch), XMConvertToRadians(m_fPrepareYaw), XMConvertToRadians(m_fPrepareRoll));
	m_xmf4x4World = Matrix4x4::Multiply(mtxRotate, m_xmf4x4World);
}

bool GameObject::CollisionCheck(GameObject* object)
{
	auto AABB = (object->GetAABB())[0];

	if (!object->IsDelete())
	{
		if (aabb_[0].Intersects(AABB))
			return true;
	}

	return false;
}

void GameObject::SetWorldTransf(XMFLOAT4X4 & xmf4x4World)
{
	m_xmf3Right = XMFLOAT3(xmf4x4World._11, xmf4x4World._12, xmf4x4World._13);
	m_xmf3Up = XMFLOAT3(xmf4x4World._21, xmf4x4World._22, xmf4x4World._23);
	m_xmf3Look = XMFLOAT3(xmf4x4World._31, xmf4x4World._32, xmf4x4World._33);
	m_xmf3Position = XMFLOAT3(xmf4x4World._41, xmf4x4World._42, xmf4x4World._43);
}

XMFLOAT4X4 GameObject::GetWorldTransf()
{
	XMFLOAT4X4 xmf4x4World;
	xmf4x4World = XMFLOAT4X4{
		m_xmf3Right.x,		m_xmf3Right.y,		m_xmf3Right.z, 0.0f,
		m_xmf3Up.x,			m_xmf3Up.y,			m_xmf3Up.z, 0.0f,
		m_xmf3Look.x,		m_xmf3Look.y,		m_xmf3Look.z, 0.0f,
		m_xmf3Position.x,	m_xmf3Position.y,	m_xmf3Position.z, 0.0f,
	};

	return xmf4x4World;
}

void GameObject::SetPosition(float x, float y, float z)
{
	m_xmf3PrevPosition.x = m_xmf3Position.x;
	m_xmf3PrevPosition.y = m_xmf3Position.y;
	m_xmf3PrevPosition.z = m_xmf3Position.z;

	m_xmf3Position.x = x;
	m_xmf3Position.y = y;
	m_xmf3Position.z = z;
}

void GameObject::SetPosition(XMFLOAT3 & xmf3Position)
{
	SetPosition(xmf3Position.x, xmf3Position.y, xmf3Position.z);
}

void GameObject::SetPrevPosition(XMFLOAT3 & xmf3Position)
{
	m_xmf3PrevPosition.x = xmf3Position.x;
	m_xmf3PrevPosition.y = xmf3Position.y;
	m_xmf3PrevPosition.z = xmf3Position.z;
}

void GameObject::MoveStrafe(float fDistance)
{
	XMFLOAT3 xmf3Position = GetPosition();
	XMFLOAT3 xmf3Right = GetRight();
	xmf3Position = Vector3::Add(xmf3Position, xmf3Right, fDistance);
	SetPosition(xmf3Position);
}

void GameObject::MoveUp(float fDistance)
{
	XMFLOAT3 xmf3Position = GetPosition();
	XMFLOAT3 xmf3Up = GetUp();
	xmf3Position = Vector3::Add(xmf3Position, xmf3Up, fDistance);
	SetPosition(xmf3Position);
}

void GameObject::MoveForward(float fDistance)
{
	XMFLOAT3 xmf3Position = GetPosition();
	XMFLOAT3 xmf3Look = GetLook();
	xmf3Position = Vector3::Add(xmf3Position, xmf3Look, fDistance);
	SetPosition(xmf3Position);
}

void GameObject::Move(XMFLOAT3 xmf3Direction, float fDistance)
{
	XMFLOAT3 xmf3Position = GetPosition();
	xmf3Position = Vector3::Add(xmf3Position, xmf3Direction, fDistance);
	SetPosition(xmf3Position);
}

void GameObject::Rotate(float fPitch, float fYaw, float fRoll)
{
	if (fPitch != 0.0f)
	{
		XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3Right),
			XMConvertToRadians(fPitch));
		m_xmf3Look = Vector3::TransformNormal(m_xmf3Look, xmmtxRotate);
		m_xmf3Up = Vector3::TransformNormal(m_xmf3Up, xmmtxRotate);
	}
	if (fYaw != 0.0f)
	{
		XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3Up),
			XMConvertToRadians(fYaw));
		m_xmf3Look = Vector3::TransformNormal(m_xmf3Look, xmmtxRotate);
		m_xmf3Right = Vector3::TransformNormal(m_xmf3Right, xmmtxRotate);
	}
	if (fRoll != 0.0f)
	{
		XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3Look),
			XMConvertToRadians(fRoll));
		m_xmf3Right = Vector3::TransformNormal(m_xmf3Right, xmmtxRotate);
		m_xmf3Up = Vector3::TransformNormal(m_xmf3Up, xmmtxRotate);
	}

	m_xmf3Look = Vector3::Normalize(m_xmf3Look);
	m_xmf3Right = Vector3::CrossProduct(m_xmf3Up, m_xmf3Look, true);
	m_xmf3Up = Vector3::CrossProduct(m_xmf3Look, m_xmf3Right, true);
}

void GameObject::UpdateWorldTransform()
{
	if (model_) model_->UpdateWorldTransform(&m_xmf4x4World);
}

void GameObject::SetHitPoint(int nHitPoint)
{
	if (nHitPoint >= max_hp_)
		hp_ = max_hp_;
	else
		hp_ = nHitPoint;
}

void GameObject::SetUse(bool use)
{
	 in_used_ = use; 
	 if (use)
	 {
		 delete_ = false;
		 ElapsedTime_ = 0.0f;
	 }
}
