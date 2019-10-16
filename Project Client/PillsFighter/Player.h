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
class CBattleScene;

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
	void DashMove(ULONG dwDirection, float fDistance);
	virtual void Rotate(float x, float y, float z);
	virtual void CameraReset() { m_pCamera->CameraReset(); }
	virtual void SetHitPoint(int nHitPoint);

	void Update(float fTimeElapsed);

	virtual void OnPlayerUpdateCallback(float fTimeElapsed);
	void SetPlayerUpdatedContext(LPVOID pContext) { m_pPlayerUpdatedContext = pContext; }
	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera, bool bSetTexture = true, bool bSetShader = true, int nInstances = 1);
	virtual void RenderToShadow(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera, bool bSetTexture = true, bool bSetShader = true, int nInstances = 1);
	virtual void RenderWire(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera, int nInstances = 1);

	virtual void OnCameraUpdateCallback(float fTimeElapsed);
	void SetCameraUpdatedContext(LPVOID pContext) { m_pCameraUpdatedContext = pContext; }

	virtual void CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void ReleaseShaderVariables();

	virtual void ReleaseUploadBuffers();

	virtual CCamera *SetCamera(float fTimeElapsed);

protected:
#define BOOSTER_POWER 0.1f
#define MAX_UP_POWER 8.0f
#define MAX_DOWN_POWER -8.0f
#define INTERVAL_BOOSTER_GAUGE_CHARGE 0.1f
#define BOOSTER_GAUGE_CHARGE_VALUE 1
#define INTERVAL_BOOSTER_GAUGE_CONSUME 0.15f
#define BOOSTER_GAUGE_CONSUME_VALUE 1

	int					m_nBoosterGauge = 100;
	float				m_fTimeForConsumeBoosterGauge = 0.0f;
	float				m_fTimeForChargeBoosterGauge = 0.0f;
	float				m_fTimeForBoostUp = 0.0f;
	float				m_fTimeForBoostDown = 0.0f;

	float				m_fMass = 2.0f;

public:
#define BOOSTER_TYPE_UP 1
#define BOOSTER_TYPE_DOWN 2
	void ActivationBooster(UINT nType);
	void ActivationDash();
	void DeactivationBooster() { if(!IsDash()) m_nState &= ~OBJECT_STATE_BOOSTER; m_bChangedSpaceStart = false; }
	void DeactivationDash() { m_nState &= ~OBJECT_STATE_BOOSTER; m_nState &= ~OBJECT_STATE_DASH;  m_nDashDirection = 0; m_bChangedDashStart = false; }
	void ApplyGravity(float fGravity, float fTimeElapsed);

	int GetBoosterGauge() { return m_nBoosterGauge; }

	float GetVelocity() { return m_fVelocityY; }
	void SetVelocity(float fVelocity) { m_fVelocityY = fVelocity; }

public:
	void ProcessBooster(float fElapsedTime);
	void ProcessBoosterCharge(float fElapsedTime);
	void ProcessBoosterConsume(float fElapsedTime);
	void ProcessHitPoint();

protected:
	CBattleScene			*m_pScene = NULL;

public:
	void SetScene(CBattleScene *pCScene) { m_pScene = pCScene; }

public:
	XMFLOAT4X4 GetToTarget(XMFLOAT3 xmf3Position);
	XMFLOAT3 GetToTargetPosition(XMFLOAT3 xmf3Position);
	virtual void ProcessMoveToCollision(BoundingBox *pxmAABB, BoundingBox *pxmObjAABB);

protected:
	int		m_nGmGunAmmo = 100;
	int		m_nBazookaAmmo = 20;
	int		m_nMachineGunAmmo = 100;
	int		m_nSMGAmmo = 100;

	bool	m_bReloading = false;
	float	m_fReloadTime = 0.0f;

public:
	void PickUpAmmo(int nType, int nAmmo);

	void ProcessTime(CWeapon *pWeapon, float fTimeElapsed);
	void Attack(CWeapon *pWeapon);
	void PrepareAttack(CWeapon *pWeapon);
	void Reload(CWeapon *pWeapon);
	bool IsReload() { return m_bReloading; }
	float GetReloadElapsedTime() { return m_fReloadTime; }
	void SetReloadTime(float fTime) { m_fReloadTime = fTime; }

	int GetGMGunAmmo() { return m_nGmGunAmmo; };
	int GetBazookaAmmo() { return m_nBazookaAmmo; };
	int GetMachineGunAmmo() { return m_nMachineGunAmmo; };
	int GetSMGAmmo() { return m_nSMGAmmo; };

	WEAPON_TYPE GetWeaponType();

	virtual void ChangeWeapon(int nIndex);
	virtual void AddWeapon(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, CWeapon *pWeapon);
	void SetWeaponShader(CShader *pShader) { m_pWeaponShader = pShader; }

protected:
	CShader		*m_pWeaponShader = NULL;
	BOOL		m_bWeaponChanged = FALSE;
	ULONG		m_nDashDirection = 0;
	bool		m_bShiftDown = false;
	bool		m_bChangedDashStart = false;

	bool		m_bSpaceDown = false;
	bool		m_bChangedSpaceStart = false;

	bool		m_bVDown = false;
public:
	void ShiftUp() { m_bShiftDown = false; }
	void SpaceUp() { m_bSpaceDown = false; }
	void VUp() { m_bVDown = false; }

	BOOL GetWeaponChanged() { return m_bWeaponChanged; }
	void SetWeaponChanged(BOOL bWeaponChanged) { m_bWeaponChanged = bWeaponChanged; }

	void ProcessAnimation();
	void ProcessDashAnimation();
	void ProcessJumpAnimation();
	void ProcessShootAnimation();

	void DeactiveMoving() { m_nState &= ~OBJECT_STATE_WALK; }

	// For Animation	
protected:
	// Others
	bool m_bShootable = false;

	int m_nAnimationList[3] = { ANIMATION_STATE_BEAM_SABER_1_ONCE , ANIMATION_STATE_BEAM_SABER_2_ONCE, ANIMATION_STATE_BEAM_SABER_3_ONCE };
	int m_nSaberAnimationIndex = 0;

public:
	bool IsSameUpDownAnimation() { return m_ppAnimationControllers[ANIMATION_UP] && m_ppAnimationControllers[ANIMATION_DOWN]; }
	bool IsOnGround() { return m_nState & OBJECT_STATE_ONGROUND; }
	bool IsJumping() { return m_nState & OBJECT_STATE_JUMP; }
	bool IsWalking() { return m_nState & OBJECT_STATE_WALK; }
	bool IsBoostering() { return m_nState & OBJECT_STATE_BOOSTER; }
	bool IsDash() { return m_nState & OBJECT_STATE_DASH; }
	bool IsDoingAttack() { return (m_nState & OBJECT_STATE_SHOOTING) || (m_nState & OBJECT_STATE_SWORDING); }
	bool IsShooting() { return m_nState & OBJECT_STATE_SHOOTING; }
	bool IsSwording() { return m_nState & OBJECT_STATE_SWORDING; }
	bool IsMoving() { return IsJumping() || IsWalking() || IsBoostering() || IsDoingAttack(); }
	bool IsIdle() { return !IsMoving(); }
	bool IsUnderBodyChangeable() { return !IsJumping() && !IsWalking() && !IsDash() && IsOnGround(); }
	bool AnimationIsShootStart() { return AnimationIs(ANIMATION_UP, ANIMATION_STATE_GM_GUN_SHOOT_START) || AnimationIs(ANIMATION_UP, ANIMATION_STATE_DASH_SHOOT_START_ONCE); }
	bool AnimationIsShootOnce() { return AnimationIs(ANIMATION_UP, ANIMATION_STATE_SHOOT_ONCE) || AnimationIs(ANIMATION_UP, ANIMATION_STATE_SHOOT_DASH_ONCE); }
	bool AnimationIsShootReturn() { return AnimationIs(ANIMATION_UP, ANIMATION_STATE_GM_GUN_SHOOT_RETURN) || AnimationIs(ANIMATION_UP, ANIMATION_STATE_DASH_SHOOT_RETURN_ONCE); }
	bool IsPrepareDashAnimation();

	// For Attack
protected:
	bool m_LButtonDown = false;
	bool m_RButtonDown = false;
	float m_fMouseUpTime = 0.0f;

	bool m_bZoomIn = false;
	bool m_bAim = false;

public:
	void LButtonUp() { m_LButtonDown = false; m_fMouseUpTime = 0.0f;}
	void RButtonUp() { m_RButtonDown = false; }
	void ProcessMouseUpTime(float fElapsedTime);

	void ZoomIn();
	void ZoomOut();
	void TakeAim();
	bool IsZoomIn() { return m_bZoomIn; }

public:
	void GenerateViewMatrix();
	XMFLOAT4X4 GetViewMatrix() { return(m_xmf4x4View); }
	void SetUserInterface(CUserInterface *pUI) { m_pUI = pUI; }
	void ChangeUIAmmo();

protected:
	XMFLOAT4X4 	m_xmf4x4View;

protected:
	CUserInterface *m_pUI = NULL;
	int m_nEquipWeaponIndex = 0;

public:
	void SendShootPacket();
	virtual void ProcessDie(float fRespawnTime);
	virtual void ProcessRespawn(int nHP, XMFLOAT3 xmf3Position);
};