#pragma once

#include "GameObject.h"
#include "Camera.h"

#define DIR_FORWARD				0x01
#define DIR_BACKWARD			0x02
#define DIR_LEFT				0x04
#define DIR_RIGHT				0x08
#define DIR_UP					0x10
#define DIR_DOWN				0x20

#define SHOT_COOLTIME			0.05f

class CShader;
class CUserInterface;
class CObjectsShader;
class CRepository;
class CScene;

class CPlayer : public CGameObject
{
protected:
	LPVOID m_pPlayerUpdatedContext = NULL;
	LPVOID m_pCameraUpdatedContext = NULL;

	CCamera *m_pCamera = NULL;

public:
	CPlayer(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature, CRepository *pRepository, void *pContext = NULL);
	virtual ~CPlayer();

	CCamera *GetCamera() { return(m_pCamera); }
	void SetCamera(CCamera *pCamera) { m_pCamera = pCamera; }

	void Move(ULONG nDirection, float fDistance);
	void Move(const XMFLOAT3& xmf3Shift);
	virtual void Rotate(float x, float y, float z);
	
	//�÷��̾��� ��ġ�� ȸ�� ������ ��� �ð��� ���� �����ϴ� �Լ��̴�. 
	void Update(float fTimeElapsed);

	//�÷��̾��� ��ġ�� �ٲ� ������ ȣ��Ǵ� �Լ��� �� �Լ����� ����ϴ� ������ �����ϴ� �Լ��̴�.
	virtual void OnPlayerUpdateCallback(float fTimeElapsed);
	void SetPlayerUpdatedContext(LPVOID pContext) { m_pPlayerUpdatedContext = pContext; }

	//ī�޶��� ��ġ�� �ٲ� ������ ȣ��Ǵ� �Լ��� �� �Լ����� ����ϴ� ������ �����ϴ� �Լ��̴�. 
	virtual void OnCameraUpdateCallback(float fTimeElapsed);
	void SetCameraUpdatedContext(LPVOID pContext) { m_pCameraUpdatedContext = pContext; }

	virtual void CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void ReleaseShaderVariables();

	virtual void ReleaseUploadBuffers();

	virtual CCamera *SetCamera(float fTimeElapsed);

	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera);

private:
	CObjectsShader *m_pBulletShader = NULL;
	bool m_Shotable = TRUE;
	float m_ShotTime;
	BOOL m_bIsShooting = FALSE;

public:
	BOOL IsShoothin(void) { return m_bIsShooting; }
	void SetBullet(CShader *Bullet) { m_pBulletShader = (CObjectsShader*)Bullet; }
	void Shot(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);

	void CheckElapsedTime(float ElapsedTime); // �ð��� ������ ���� ���Ǿ��� ������ üũ�ϴ� �Լ�

protected:
	CShader				*m_pUserInterface = NULL;

protected:
	int					m_nBoosterGauge = 100;
	int					m_nState = 0x00;
	float				m_fBoosteringTime = 0.0f;
	float				m_fBoosterGaugeChargeTime = 0.0f;

	float				m_fOnGroundTime = 0.0f;
	float				m_fGravAcc;
	float				m_fMass = 10.0f;

	float				m_fVelocityY = 0.0f;
	float				m_fAccelerationY = 0.0f;
	float				m_fBoosterPower = 0.0f;
	float				m_fGravity = 0.0f;

public:
	int GetBoosterGauge() { return m_nBoosterGauge; }
	void SetGravity(float fGravity) { m_fGravAcc = fGravity; }

	void SetBoosterPower(float fPower) { m_fBoosterPower = fPower; }
	float GetVelocity() { return m_fVelocityY; }
	void SetVelocity(float fVelocity) { m_fVelocityY = fVelocity; }

public:
	void ProcessBooster(float fElapsedTime);
	void ProcessOnGround(float fTimeElapsed);
	void ProcessHitPoint();
	void ProcessGravity(float fTimeElapsed);

protected:
	CScene			*m_pScene = NULL;

public:
	void SetScene(CScene *pCScene) { m_pScene = pCScene; }
	void SetOnGround();

public:
	XMFLOAT4X4 GetToTarget();

	virtual void ProcessMoveToCollision(BoundingBox *pxmAABB, BoundingBox *pxmObjAABB);
};