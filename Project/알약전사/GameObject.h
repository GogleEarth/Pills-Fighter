#pragma once

#include "Mesh.h"
#include "Camera.h"

class CShader;

class CGameObject
{
private:
	int m_nReferences = 0;

protected:
	//���� ��ȯ ���
	XMFLOAT4X4 m_xmf4x4World;

	//��ġ ����, x-��(Right), y-��(Up), z-��(Look) �����̴�. 
	XMFLOAT3 m_xmf3Position;
	XMFLOAT3 m_xmf3Right;
	XMFLOAT3 m_xmf3Up;
	XMFLOAT3 m_xmf3Look;

	//���� x-��(Right), y-��(Up), z-��(Look)���� �󸶸�ŭ ȸ���ߴ°��� ��Ÿ����. 
	float m_fPitch;
	float m_fYaw;
	float m_fRoll;

	// ������ �ϱ� �� ������Ʈ ȸ�� ����
	float m_PPitch;
	float m_PYaw;
	float m_PRoll;

	// �̵� �ӷ�
	float m_MovingSpeed;
	
	CMesh *m_pMesh = NULL;
	CShader *m_pShader = NULL;

	BoundingOrientedBox	m_xmOOBB;
public:
	CGameObject();
	virtual ~CGameObject();
	void SetGameObjectInf();

	void AddRef() { m_nReferences++; }
	void Release() { if (--m_nReferences <= 0) delete this; }

	virtual void ReleaseUploadBuffers();
	virtual void SetMesh(CMesh *pMesh);
	virtual void SetShader(CShader *pShader);
	virtual void Animate(float fTimeElapsed);
	virtual void OnPrepareRender();
	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera* pCamera);

	//��� ���۸� �����Ѵ�. 
	virtual void CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);

	//��� ������ ������ �����Ѵ�. 
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void ReleaseShaderVariables();

	//���� ��ü�� ���� ��ȯ ��Ŀ��� ��ġ ���Ϳ� ����(x-��, y-��, z-��) ���͸� ��ȯ�Ѵ�. 
	void SetLook(XMFLOAT3 xmf3Look) { m_xmf3Look = xmf3Look; }
	void SetUp(XMFLOAT3 xmf3Up) { m_xmf3Up = xmf3Up; }
	void SetRight(XMFLOAT3 xmf3Right) { m_xmf3Right = xmf3Right; }
	XMFLOAT3 GetPosition() { return m_xmf3Position; }
	XMFLOAT3 GetLook() { return m_xmf3Look; }
	XMFLOAT3 GetUp() { return m_xmf3Up; }
	XMFLOAT3 GetRight() { return m_xmf3Right; }
	float GetMovingSpeed() { return(m_MovingSpeed); }

	//���� ��ü�� ��ġ�� �����Ѵ�.
	void SetPosition(float x, float y, float z);
	void SetPosition(XMFLOAT3& xmf3Position);
	void SetMovingSpeed(float MovingSpeed) { m_MovingSpeed = MovingSpeed; }

	void SetPrepareRotate(float Pitch, float Yaw, float Roll) {	m_PPitch = Pitch; m_PYaw = Yaw; m_PRoll = Roll; }

	//���� ��ü�� ���� x-��, y-��, z-�� �������� �̵��Ѵ�.
	void MoveStrafe(float fDistance = 1.0f);
	void MoveUp(float fDistance = 1.0f);
	void MoveForward(float fDistance = 1.0f);

	//���� ��ü�� ȸ��(x-��, y-��, z-��)�Ѵ�. 
	void Rotate(float fPitch = 10.0f, float fYaw = 10.0f, float fRoll = 10.0f);
	
	BoundingOrientedBox GetOOBB() { return m_xmOOBB; }
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