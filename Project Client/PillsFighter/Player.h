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
	void SetShader(CShader* pShader) { m_pShader = pShader; }

	void Move(ULONG nDirection, float fDistance);
	void Move(const XMFLOAT3& xmf3Shift);
	virtual void Rotate(float x, float y, float z);
	
	//플레이어의 위치와 회전 정보를 경과 시간에 따라 갱신하는 함수이다. 
	void Update(float fTimeElapsed);

	//플레이어의 위치가 바뀔 때마다 호출되는 함수와 그 함수에서 사용하는 정보를 설정하는 함수이다.
	virtual void OnPlayerUpdateCallback(float fTimeElapsed);
	void SetPlayerUpdatedContext(LPVOID pContext) { m_pPlayerUpdatedContext = pContext; }

	//카메라의 위치가 바뀔 때마다 호출되는 함수와 그 함수에서 사용하는 정보를 설정하는 함수이다. 
	virtual void OnCameraUpdateCallback(float fTimeElapsed);
	void SetCameraUpdatedContext(LPVOID pContext) { m_pCameraUpdatedContext = pContext; }

	virtual void CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void ReleaseShaderVariables();

	virtual void ReleaseUploadBuffers();

	virtual CCamera *SetCamera(float fTimeElapsed);

	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera);
	virtual void RenderWire(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera);

private:
	CObjectsShader *m_pBulletShader = NULL;
	bool m_Shotable = TRUE;
	float m_ShotTime;
	BOOL m_bIsShooting = FALSE;

public:
	BOOL IsShoothin(void) { return m_bIsShooting; }
	void SetBullet(CShader *Bullet) { m_pBulletShader = (CObjectsShader*)Bullet; }
	void Shot(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);

	void CheckElapsedTime(float ElapsedTime); // 시간이 지남에 따라 사용되야할 변수를 체크하는 함수

protected:
	CShader				*m_pUserInterface = NULL;

protected:
	int					m_nBoosterGauge = 100;
	int					m_nState = 0x00;
	float				m_fBoosteringTime = 0.0f;
	float				m_fBoosterGaugeChargeTime = 0.0f;

	float				m_fOnGroundTime = 0.0f;

public:
	int GetBoosterGauge() { return m_nBoosterGauge; }

public:
	void ProcessBooster(float fElapsedTime);
	void ProcessOnGround(float fTimeElapsed);
	void ProcessHitPoint();
};