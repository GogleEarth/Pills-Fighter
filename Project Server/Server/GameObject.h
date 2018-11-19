#pragma once

#include "Mesh.h"

////////////////////////////////////////////////////////////////////////////////

class CGameObject
{
public:
	XMFLOAT4X4 m_xmf4x4World;
	OBJECT_TYPE						m_Object_Type;
protected:

	XMFLOAT3 m_xmf3Position;
	XMFLOAT3 m_xmf3Right;
	XMFLOAT3 m_xmf3Up;
	XMFLOAT3 m_xmf3Look;

	XMFLOAT3 m_xmf3Direction;

	float m_fPitch;
	float m_fYaw;
	float m_fRoll;

	// ������ �ϱ� �� ������Ʈ ȸ�� ����
	float m_PPitch;
	float m_PYaw;
	float m_PRoll;

	// �̵� �ӷ�
	float m_MovingSpeed;
	
	UINT							m_nMeshes = 0;
	CMesh							**m_ppMeshes = NULL;
	CCubeMesh						**m_ppCubeMeshes = NULL;


	BoundingOrientedBox				*m_xmOOBB = NULL;

	bool m_Delete = FALSE;

public:
	CGameObject();
	virtual ~CGameObject();

	void SetMesh(CMesh **ppMeshes, CCubeMesh **ppCubeMeshes, UINT nMeshes);
	UINT GetNumMeshes() { return m_nMeshes; }
	
	virtual void Animate(float fTimeElapsed);
	
	//���� ��ü�� ���� ��ȯ ��Ŀ��� ��ġ ���Ϳ� ����(x-��, y-��, z-��) ���͸� ��ȯ�Ѵ�. 
	void SetLook(XMFLOAT3 xmf3Look) { m_xmf3Look = xmf3Look; }
	void SetUp(XMFLOAT3 xmf3Up) { m_xmf3Up = xmf3Up; }
	void SetRight(XMFLOAT3 xmf3Right) { m_xmf3Right = xmf3Right; }
	void SetDirection(XMFLOAT3 xmf3Direction) { m_xmf3Direction = xmf3Direction; }
	void SetWorldTransf(XMFLOAT4X4& xmf4x4World);
	XMFLOAT3 GetPosition() { return m_xmf3Position; }
	XMFLOAT3 GetLook() { return m_xmf3Look; }
	XMFLOAT3 GetUp() { return m_xmf3Up; }
	XMFLOAT3 GetRight() { return m_xmf3Right; }
	XMFLOAT3 GetDirection() { return m_xmf3Direction; }
	float GetMovingSpeed() { return(m_MovingSpeed); }
	XMFLOAT4X4 CGameObject::GetWorldTransf()
	{
		XMFLOAT4X4 xmf4x4World;
		xmf4x4World = XMFLOAT4X4{
		   m_xmf3Right.x,      m_xmf3Right.y,      m_xmf3Right.z, 0.0f,
		   m_xmf3Up.x,         m_xmf3Up.y,         m_xmf3Up.z, 0.0f,
		   m_xmf3Look.x,      m_xmf3Look.y,      m_xmf3Look.z, 0.0f,
		   m_xmf3Position.x,   m_xmf3Position.y,   m_xmf3Position.z, 0.0f,
		};

		return xmf4x4World;
	}

	//���� ��ü�� ��ġ�� �����Ѵ�.
	void SetPosition(float x, float y, float z);
	void SetPosition(XMFLOAT3& xmf3Position);
	void SetMovingSpeed(float MovingSpeed) { m_MovingSpeed = MovingSpeed; }

	void SetPrepareRotate(float Pitch, float Yaw, float Roll) {	m_PPitch = Pitch; m_PYaw = Yaw; m_PRoll = Roll; }

	//���� ��ü�� ���� x-��, y-��, z-�� �������� �̵��Ѵ�.
	void MoveStrafe(float fDistance = 1.0f);
	void MoveUp(float fDistance = 1.0f);
	void MoveForward(float fDistance = 1.0f);
	void Move(XMFLOAT3 xmf3Direction, float fDistance = 1.0f);

	//���� ��ü�� ȸ��(x-��, y-��, z-��)�Ѵ�. 
	void Rotate(float fPitch = 10.0f, float fYaw = 10.0f, float fRoll = 10.0f);
	void OnPrepareRender();

	BoundingOrientedBox GetOOBB(UINT nIndex) { return m_xmOOBB[nIndex]; }
	void DeleteObject() { m_Delete = TRUE; }
	bool IsDelete() { return m_Delete; }

protected:
	int		m_iHitPoint;

public:
	int *GetHitPoint() { return &m_iHitPoint; }
	void SetHitPoint(int iHitPoint) { m_iHitPoint = iHitPoint; }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////

class RandomMoveObject : public CGameObject
{
public:
	RandomMoveObject();
	virtual ~RandomMoveObject();

	void InitRandomRotate();
	virtual void Animate(float ElapsedTime);

private:
	// ������
	float m_Time; // �ð��Ǹ� ���� �ٲٴ� �ð� 
	float m_ElapsedTime; // �ൿ�� �ð�
	float m_Angle; // ȸ���� ����

	// ������
	float m_RotateSpeed; // ȸ�� �ӵ� ( �ʴ� ȸ���� ���� Angle )
	float m_RotateAngle; // ȸ���ؾ��� ����
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

extern void CreateRobotObjectMesh(CMesh**& ppMeshes, CCubeMesh**& ppCubeMeshes, UINT& nMeshes);