#pragma once

#include "GameObject.h"
#include "Camera.h"

#define DIR_FORWARD				0x01
#define DIR_BACKWARD			0x02
#define DIR_LEFT				0x04
#define DIR_RIGHT				0x08
#define DIR_UP					0x10
#define DIR_DOWN				0x20

class CShader;
class CUserInterface;
class CRepository;
class CScene;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

class CPlayer : public CRobotObject
{
protected:
	LPVOID m_pPlayerUpdatedContext = NULL;
	LPVOID m_pCameraUpdatedContext = NULL;

	CCamera *m_pCamera = NULL;

public:
	CPlayer(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature, CRepository *pRepository, void *pContext, int nRobotType);
	virtual ~CPlayer();

	CCamera *GetCamera() { return(m_pCamera); }
	void SetCamera(CCamera *pCamera) { m_pCamera = pCamera; }

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

protected:
	int					m_nBoosterGauge = 100;
	float				m_fElapsedBGConsumeTime = 0.0f;
	float				m_fElapsedBGChargeTime = 0.0f;
	float				m_fKeepBoosteringTime = 0.0f;
	float				m_fMaxSpeed = 5.0f;
	float				m_fBoosterBasicForce = 3.0f;
	bool				m_bChargeBG = false;

	float				m_fGravAcc;
	float				m_fMass = 15.0f;

	float				m_fVelocityY = 0.0f;
	float				m_fAccelerationY = 0.0f;
	float				m_fGravity = 0.0f;

public:
	void ActivationBooster();
	void DeactivationBooster() {m_nState &= ~OBJECT_STATE_BOOSTERING;};
	void SetElapsedBGConsumeTime() { m_fElapsedBGConsumeTime = 1.0f; }
	void SetElapsedBGChargeTime() { m_fElapsedBGChargeTime = 1.0f; }

	int GetBoosterGauge() { return m_nBoosterGauge; }
	void SetGravity(float fGravity) { m_fGravAcc = fGravity; }

	float GetVelocity() { return m_fVelocityY; }
	void SetVelocity(float fVelocity) { m_fVelocityY = fVelocity; }

public:
	void ProcessBoosterGauge(float fElapsedTime);
	void ProcessHitPoint();
	void ProcessGravity(float fTimeElapsed);

protected:
	CScene			*m_pScene = NULL;

public:
	void SetScene(CScene *pCScene) { m_pScene = pCScene; }

public:
	XMFLOAT4X4 GetToTarget(XMFLOAT3 xmf3Position);
	virtual void ProcessMoveToCollision(BoundingBox *pxmAABB, BoundingBox *pxmObjAABB);

protected:
	int		m_nGimGunAmmo = 0;
	int		m_nBazookaAmmo = 0;
	int		m_nMachineGunAmmo = 0;

	bool	m_bReloading = false;
	float	m_fReloadTime = 0.0f;

public:
	void PickUpAmmo(int nType, int nAmmo);

	void ProcessTime(CWeapon *pWeapon, float fTimeElapsed);
	void Attack(CWeapon *pWeapon);
	void PrepareAttack(CWeapon *pWeapon);
	void Reload(CWeapon *pWeapon);

	WEAPON_TYPE GetWeaponType();

	virtual void ChangeWeapon(int nIndex);
	void AddWeapon(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, CModel *pWeaponModel, int nType, CShader *pBulletShader, CShader *pEffectShader, int nGroup);

protected:
	BOOL		m_bWeaponChanged = FALSE;

public:
	BOOL GetWeaponChanged() { return m_bWeaponChanged; }
	void SetWeaponChanged(BOOL bWeaponChanged) { m_bWeaponChanged = bWeaponChanged; }

	void ProcessAnimation();

	void DeactiveMoving() { m_nState &= ~OBJECT_STATE_MOVING; }

	// For Animation	
protected:
	// Call Back Data
	bool m_bShootStartEndPoint = false;
	bool m_bShootReturnEndPoint = false;
	bool m_bShootOnceEndPoint = false;
	bool m_bSwordingEndPoint = false;
	bool m_bJumpEndPoint = false;

	// Others
	bool m_bChangeableOnceAni = false;

	int m_nAnimationList[3] = { ANIMATION_STATE_BEAM_SABER_1_ONE , ANIMATION_STATE_BEAM_SABER_2_ONE, ANIMATION_STATE_BEAM_SABER_3_ONE };
	int m_nSaberAnimationIndex = 0;

	// For Attack
protected:
	bool m_bShootable = false;
	bool m_LButtonDown = false;
	bool m_bShootBullet = false;
	float m_fMouseUpTime = 0.0f;

public:
	bool IsShootable() { return m_bShootable; }
	void SetShootable(bool bShot) { m_bShootable = bShot; }
	void LButtonUp() { m_LButtonDown = false; m_fMouseUpTime = 0.0f;}
	bool GetShootBullet() { return m_bShootBullet; }
	void SetShootBullet(bool b) { m_bShootBullet = b; }

	void ProcessMouseUpTime(float fElapsedTime);
};