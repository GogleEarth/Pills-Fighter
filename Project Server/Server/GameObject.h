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

	// 렌더링 하기 전 오브젝트 회전 설정
	float m_PPitch;
	float m_PYaw;
	float m_PRoll;

	// 이동 속력
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
	
	//게임 객체의 월드 변환 행렬에서 위치 벡터와 방향(x-축, y-축, z-축) 벡터를 반환한다. 
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

	//게임 객체의 위치를 설정한다.
	void SetPosition(float x, float y, float z);
	void SetPosition(XMFLOAT3& xmf3Position);
	void SetMovingSpeed(float MovingSpeed) { m_MovingSpeed = MovingSpeed; }

	void SetPrepareRotate(float Pitch, float Yaw, float Roll) {	m_PPitch = Pitch; m_PYaw = Yaw; m_PRoll = Roll; }

	//게임 객체를 로컬 x-축, y-축, z-축 방향으로 이동한다.
	void MoveStrafe(float fDistance = 1.0f);
	void MoveUp(float fDistance = 1.0f);
	void MoveForward(float fDistance = 1.0f);
	void Move(XMFLOAT3 xmf3Direction, float fDistance = 1.0f);

	//게임 객체를 회전(x-축, y-축, z-축)한다. 
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
	// 고정값
	float m_Time; // 시간되면 방향 바꾸는 시간 
	float m_ElapsedTime; // 행동한 시간
	float m_Angle; // 회전한 각도

	// 랜덤값
	float m_RotateSpeed; // 회전 속도 ( 초당 회전할 각도 Angle )
	float m_RotateAngle; // 회전해야할 각도
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
	float m_DurationTime; // 발사 후 생존?시간
	float m_ElapsedTime; // 행동한 시간
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern void CreateRobotObjectMesh(CMesh**& ppMeshes, CCubeMesh**& ppCubeMeshes, UINT& nMeshes);