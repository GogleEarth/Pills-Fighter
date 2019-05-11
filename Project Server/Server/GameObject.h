#pragma once

#include "Mesh.h"

////////////////////////////////////////////////////////////////////////////////
class CModel;

class CGameObject
{
public:
	XMFLOAT4X4 m_xmf4x4World;
	bool m_bPlay = false;
protected:

	XMFLOAT3 m_xmf3Position;
	XMFLOAT3 m_xmf3PrevPosition;
	XMFLOAT3 m_xmf3Right;
	XMFLOAT3 m_xmf3Up;
	XMFLOAT3 m_xmf3Look;


public:
	float m_RotationSpeed;
	float m_DurationTime; // �߻� �� ����?�ð�
	float m_ElapsedTime; // �ൿ�� �ð�


	OBJECT_TYPE	m_Object_Type;
	BULLET_TYPE m_Bullet_Type;
	int m_iId;
	int index;
	int hp;

	float m_fPitch;
	float m_fYaw;
	float m_fRoll;

	// ������ �ϱ� �� ������Ʈ ȸ�� ����
	float m_fPreparePitch;
	float m_fPrepareYaw;
	float m_fPrepareRoll;

	// �̵� �ӷ�
	float m_MovingSpeed;
	bool in_used = false;

protected:
	CModel							*m_pModel = NULL;

	std::vector<BoundingBox>			m_vxmAABB;


	bool							 m_Delete = FALSE;
	int									m_nMeshes = 0;
public:
	CGameObject();
	CGameObject(BULLET_TYPE Bullet_Type);
	virtual ~CGameObject();

	void SetModel(CModel *pModel);
	void SetMesh(CMesh *pMesh, CCubeMesh *pCubeMesh);

	virtual void Animate(float fTimeElapsed);
	virtual void OnPrepareRender();

	//���� ��ü�� ���� ��ȯ ��Ŀ��� ��ġ ���Ϳ� ����(x-��, y-��, z-��) ���͸� ��ȯ�Ѵ�. 
	void SetLook(XMFLOAT3 xmf3Look) { m_xmf3Look = xmf3Look; }
	void SetUp(XMFLOAT3 xmf3Up) { m_xmf3Up = xmf3Up; }
	void SetRight(XMFLOAT3 xmf3Right) { m_xmf3Right = xmf3Right; }
	void SetWorldTransf(XMFLOAT4X4& xmf4x4World);
	XMFLOAT3 GetPosition() { return m_xmf3Position; }
	XMFLOAT3 GetLook() { return m_xmf3Look; }
	XMFLOAT3 GetUp() { return m_xmf3Up; }
	XMFLOAT3 GetRight() { return m_xmf3Right; }
	float GetMovingSpeed() { return(m_MovingSpeed); }
	XMFLOAT4X4 GetWorldTransf();

	//���� ��ü�� ��ġ�� �����Ѵ�.
	void SetPosition(float x, float y, float z);
	void SetPosition(XMFLOAT3& xmf3Position);
	void SetMovingSpeed(float MovingSpeed) { m_MovingSpeed = MovingSpeed; }

	void SetPrepareRotate(float Pitch, float Yaw, float Roll) { m_fPreparePitch = Pitch; m_fPrepareYaw = Yaw; m_fPrepareRoll = Roll; }
	void AddPrepareRotate(float Pitch, float Yaw, float Roll) { m_fPreparePitch += Pitch; m_fPrepareYaw += Yaw; m_fPrepareRoll += Roll; }

	//���� ��ü�� ���� x-��, y-��, z-�� �������� �̵��Ѵ�.
	void MoveStrafe(float fDistance = 1.0f);
	void MoveUp(float fDistance = 1.0f);
	void MoveForward(float fDistance = 1.0f);
	void Move(XMFLOAT3 xmf3Direction, float fDistance = 1.0f);

	//���� ��ü�� ȸ��(x-��, y-��, z-��)�Ѵ�. 
	void Rotate(float fPitch = 10.0f, float fYaw = 10.0f, float fRoll = 10.0f);

	std::vector<BoundingBox>& GetAABB() { return m_vxmAABB; }
	int GetNumAABB() { return m_nMeshes; }
	void Delete() { m_Delete = TRUE; }
	bool IsDelete() { return m_Delete; }
	void CallBackPosition() { m_xmf3Position = m_xmf3PrevPosition; }

	void UpdateWorldTransform();

protected:
	int			m_nHitPoint;
	int			m_nMaxHitPoint;
	XMFLOAT3	serverPosition;

public:
	int GetHitPoint() { return m_nHitPoint; }
	int GetMaxHitPoint() { return m_nMaxHitPoint; }
	void SetMaxHitPoint(int nMaxHitPoint) { m_nMaxHitPoint = nMaxHitPoint; }
	void SetHitPoint(int nHitPoint) { m_nHitPoint = nHitPoint; }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Bullet : public CGameObject
{
public:
	Bullet();
	virtual ~Bullet();

	virtual void Animate(float ElapsedTime);

private:
	float m_RotationSpeed;
	float m_DurationTime; // �߻� �� ����?�ð�
	float m_ElapsedTime; // �ൿ�� �ð�
};
////////////////////////////////////////////////////////////////////////////////////////////////////////////