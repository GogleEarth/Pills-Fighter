#pragma once

#include"GameObject.h"

class CObjectsShader;

class CWeapon : public CGameObject
{
public:
	CWeapon();
	virtual ~CWeapon();

	virtual void Initialize();

protected:
	CGameObject	*m_pOwner = NULL;
	CModel		*m_pParentModel = NULL;

	int			m_nType = 0;

	ID3D12Device *m_pd3dDevice = NULL;
	ID3D12GraphicsCommandList *m_pd3dCommandList = NULL;

public:
	void SetForCreate(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList) { m_pd3dDevice = pd3dDevice; m_pd3dCommandList = pd3dCommandList; };

	void SetOwner(CGameObject *pOwner) { m_pOwner = pOwner; }
	void SetParentModel(CModel *pModel) { m_pParentModel = pModel; }
	int GetType() { return m_nType; }

	virtual void SetType() {};
	virtual void SetType(int nType) { m_nType |= nType; };

	virtual void Animate(float fTimeElapsed, CCamera *pCamera = NULL);
	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera = NULL);
	virtual void RenderWire(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera = NULL);
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CGun : public CWeapon
{
public:
	CGun();
	virtual ~CGun();

	virtual void Initialize();


	CObjectsShader *m_pBulletShader = NULL;

	float m_fShotCoolTime = 0.0f;

	int m_nReloadedAmmo = 0;
	int m_nMaxReloadAmmo = 0;
	float m_fReloadTime = 0.0f;

public:
	float GetReloadTime() { return m_fReloadTime; }
	int GetMaxReloadAmmo() { return m_nMaxReloadAmmo; }
	int GetReloadedAmmo() { return m_nReloadedAmmo; }
	void SetBullet(CShader *Bullet) { m_pBulletShader = (CObjectsShader*)Bullet; }

	virtual void Reload(int& nAmmo);

public:
	virtual void Shot() {};

	virtual void SetReloadTime() {}
	virtual void SetShotCoolTime() {}
	virtual void SetBurstCoolTime() {}
	virtual void SetMaxReloadAmmo() {}
	virtual void SetType() { m_nType |= WEAPON_TYPE_OF_GUN; }

public:
	virtual void Animate(float ElapsedTime, CCamera *pCamera = NULL);
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

#define SHOT_COOLTIME	0.05f
#define BURST_COOLTIME	0.5f
#define RELOAD_TIME	3.0f

class CGimGun : public CGun
{
public:
	CGimGun();
	virtual ~CGimGun();

protected:
	float	m_fBurstCoolTime = 0.0f;
	bool	m_bBurst = false;
	int		m_nShotCount = 0;

public:
	virtual void Shot();

	virtual void SetReloadTime() { m_fReloadTime = RELOAD_TIME; }
	virtual void SetShotCoolTime() { m_fShotCoolTime = SHOT_COOLTIME; }
	virtual void SetBurstCoolTime() { m_fBurstCoolTime = BURST_COOLTIME; }
	virtual void SetMaxReloadAmmo() { m_nMaxReloadAmmo = 30; }
	virtual void SetType();

	virtual void Animate(float ElapsedTime, CCamera *pCamera = NULL);
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

class CBazooka : public CGimGun // 나중에 Gun으로 바꿔야함
{
public:
	CBazooka();
	virtual ~CBazooka();

	virtual void SetType();
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

class CMachineGun : public CGimGun // 나중에 Gun으로 바꿔야함
{
public:
	CMachineGun();
	virtual ~CMachineGun();

	virtual void SetType();
};