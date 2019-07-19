#pragma once

#include "Protocol.h"
#include "Model.h"
#include "Mesh.h"

class GameObject
{
protected:
	XMFLOAT4X4 m_xmf4x4World;

	XMFLOAT3 m_xmf3Position;
	XMFLOAT3 m_xmf3PrevPosition;
	XMFLOAT3 m_xmf3Right;
	XMFLOAT3 m_xmf3Up;
	XMFLOAT3 m_xmf3Look;

	OBJECT_TYPE	Object_Type_;

	int id_;
	int index_;

	float m_fPitch;
	float m_fYaw;
	float m_fRoll;

	// ������ �ϱ� �� ������Ʈ ȸ�� ����
	float m_fPreparePitch;
	float m_fPrepareYaw;
	float m_fPrepareRoll;

	float moving_speed_;

	bool in_used_ = false;
	bool play_ = false;

	CModel *model_ = NULL;
	std::vector<BoundingBox> aabb_;

	bool delete_ = false;
	int	num_meshes_ = 0;

	int	hp_;
	int	max_hp_;

	float MovingSpeed_;
	float DurationTime_;
	float ElapsedTime_;

public:
	GameObject();
	~GameObject();

	void SetModel(CModel *pModel);
	void SetMesh(CMesh *pMesh, CCubeMesh *pCubeMesh);

	void Animate(float fTimeElapsed);
	void OnPrepareRender();

	bool CollisionCheck(GameObject* object);

	//���� ��ü�� ���� ��ȯ ��Ŀ��� ��ġ ���Ϳ� ����(x-��, y-��, z-��) ���͸� ��ȯ�Ѵ�. 
	void SetLook(XMFLOAT3 xmf3Look) { m_xmf3Look = xmf3Look; }
	void SetUp(XMFLOAT3 xmf3Up) { m_xmf3Up = xmf3Up; }
	void SetRight(XMFLOAT3 xmf3Right) { m_xmf3Right = xmf3Right; }
	void SetWorldTransf(XMFLOAT4X4& xmf4x4World);
	XMFLOAT3 GetPosition() { return m_xmf3Position; }
	XMFLOAT3 GetLook() { return m_xmf3Look; }
	XMFLOAT3 GetUp() { return m_xmf3Up; }
	XMFLOAT3 GetRight() { return m_xmf3Right; }
	XMFLOAT4X4 GetWorldTransf();

	//���� ��ü�� ��ġ�� �����Ѵ�.
	void SetPosition(float x, float y, float z);
	void SetPosition(XMFLOAT3& xmf3Position);

	void SetPrepareRotate(float Pitch, float Yaw, float Roll) { m_fPreparePitch = Pitch; m_fPrepareYaw = Yaw; m_fPrepareRoll = Roll; }
	void AddPrepareRotate(float Pitch, float Yaw, float Roll) { m_fPreparePitch += Pitch; m_fPrepareYaw += Yaw; m_fPrepareRoll += Roll; }

	//���� ��ü�� ���� x-��, y-��, z-�� �������� �̵��Ѵ�.
	void MoveStrafe(float fDistance = 1.0f);
	void MoveUp(float fDistance = 1.0f);
	void MoveForward(float fDistance = 1.0f);
	void Move(XMFLOAT3 xmf3Direction, float fDistance = 1.0f);

	//���� ��ü�� ȸ��(x-��, y-��, z-��)�Ѵ�. 
	void Rotate(float fPitch = 10.0f, float fYaw = 10.0f, float fRoll = 10.0f);

	void UpdateWorldTransform();

	inline std::vector<BoundingBox>& GetAABB() { return aabb_; }
	inline int GetNumAABB() { return num_meshes_; }
	inline void Delete() { delete_ = true; }
	inline bool IsDelete() { return delete_; }
	inline void CallBackPosition() { m_xmf3Position = m_xmf3PrevPosition; }

	inline int GetHitPoint() { return hp_; }
	inline int GetMaxHitPoint() { return max_hp_; }
	inline void SetMaxHitPoint(int nMaxHitPoint) { max_hp_ = nMaxHitPoint; }
	inline void SetHitPoint(int nHitPoint) { hp_ = nHitPoint; }

	inline void SetId(int id) { id_ = id; }
	inline int GetId() { return id_; }
	inline void SetIndex(int index) { index_ = index; }
	inline int GetIndex() { return index_; }
	void SetUse(bool use);
	inline bool GetUse() { return in_used_; }
	inline void SetPlay(bool play) { play_ = play; }
	inline bool GetPlay() { return play_; }
	inline void SetObjectType(OBJECT_TYPE type) { Object_Type_ = type; }
	inline OBJECT_TYPE GetObjectType() { return Object_Type_; }

	inline void set_speed(float speed) { MovingSpeed_ = speed; }
	inline float get_speed() { return MovingSpeed_; }
	inline void set_life(float life) { DurationTime_ = life; }
};

////////////////////////////////////////////////////////////////////////////////////////////////////
