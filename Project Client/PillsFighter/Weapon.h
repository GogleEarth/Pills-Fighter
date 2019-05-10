#pragma once

#include"GameObject.h"

class CObjectsShader;
class CEffectShader;

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
	void SetOwnerTransform(XMFLOAT4X4 xmf4x4World);

	void SetOwner(CGameObject *pOwner) { m_pOwner = pOwner; }
	void SetParentModel(CModel *pModel) { m_pParentModel = pModel; }
	int GetType() { return m_nType; }

	virtual void SetType() {};
	virtual void SetType(int nType) { m_nType |= nType; };

	virtual void Animate(float fTimeElapsed, CCamera *pCamera = NULL);
	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera, bool bSetTexture = true, int nInstances = 1);
	virtual void RenderWire(ID3D12GraphicsCommandList *pd3dCommandList, CCamera* pCamera, int nInstances = 1);
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CSaber : public CWeapon
{
public:
	CSaber();
	virtual ~CSaber();

	virtual void SetType() { m_nType |= WEAPON_TYPE_OF_SABER; };
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CGun : public CWeapon
{
public:
	CGun();
	virtual ~CGun();

	virtual void Initialize();

	CModel *m_pMuzzle = NULL;
	CObjectsShader *m_pBulletShader = NULL;
	CEffectShader *m_pEffectShader = NULL;
	int	m_nBulletGroup;

	float	m_fShotCoolTime = 0.0f;

	int		m_nReloadedAmmo = 0;
	int		m_nMaxReloadAmmo = 0;
	float	m_fReloadTime = 0.0f;
	int		m_nShootCount = 0;

public:
	float GetReloadTime() { return m_fReloadTime; }
	int GetMaxReloadAmmo() { return m_nMaxReloadAmmo; }
	int GetReloadedAmmo() { return m_nReloadedAmmo; }
	void SetBullet(CShader *pBullet, CShader *pEffect, int nGroup) { m_pBulletShader = (CObjectsShader*)pBullet; m_pEffectShader = (CEffectShader*)pEffect;  m_nBulletGroup = nGroup; }

	virtual void Reload(int& nAmmo);

public:
	virtual void Attack() {};
	virtual void Shot();

	virtual void SetReloadTime() {}
	virtual void SetShotCoolTime();
	virtual void SetBurstCoolTime() {}
	virtual void SetMaxReloadAmmo() {}
	virtual void SetType() { m_nType |= WEAPON_TYPE_OF_GUN; }

	virtual void Animate(float fElapsedTime, CCamera *pCamera = NULL);
	XMFLOAT3 GetMuzzlePos() { return m_pMuzzle->GetPosition();   }

protected:
	bool m_bShootable = true;

public:
	bool IsShootable() { return m_bShootable; }
	void SetShootable(bool b) { m_bShootable = b; }

	virtual int ShootNumber() { return 0; };
	virtual int ShootedCount() { return m_nShootCount; };

	virtual void CheckShootable(float fElapsedTime);
	virtual void SetShootCount();
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

#define GG_SHOT_COOLTIME	0.05f
#define GG_BURST_COOLTIME	0.5f
#define GG_RELOAD_TIME		2.0f

class CGimGun : public CGun
{
public:
	CGimGun();
	virtual ~CGimGun();

protected:
	float	m_fBurstCoolTime = 0.0f;
	bool	m_bBurst = false;

public:
	virtual void Initialize();

	virtual void Attack();

	virtual void Shot();
	virtual void SetReloadTime() { m_fReloadTime = GG_RELOAD_TIME; }
	virtual void SetShotCoolTime();
	virtual void SetBurstCoolTime() { m_fBurstCoolTime = GG_BURST_COOLTIME; }
	virtual void SetMaxReloadAmmo() { m_nMaxReloadAmmo = 30; }
	virtual void SetType();

	virtual void OnPrepareAnimate();

	virtual int ShootNumber() { return 3; };

	virtual void CheckShootable(float fElapsedTime);
	virtual void SetShootCount();
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

#define BZK_SHOT_COOLTIME	1.0f
#define BZK_RELOAD_TIME		3.0f

class CBazooka : public CGun
{
public:
	CBazooka();
	virtual ~CBazooka();

	virtual void Initialize();

	virtual void Shot();

	virtual void SetReloadTime() { m_fReloadTime = BZK_RELOAD_TIME; }
	virtual void SetShotCoolTime();
	virtual void SetMaxReloadAmmo() { m_nMaxReloadAmmo = 5; }

	virtual void SetType();
	virtual void OnPrepareAnimate();

	virtual int ShootNumber() { return 1; };
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

#define MG_SHOT_COOLTIME	0.08f
#define MG_RELOAD_TIME		3.0f

class CMachineGun : public CGun
{
public:
	CMachineGun();
	virtual ~CMachineGun();

	virtual void Initialize();

	virtual void Shot();

	virtual void SetReloadTime() { m_fReloadTime = MG_RELOAD_TIME; }
	virtual void SetShotCoolTime();
	virtual void SetMaxReloadAmmo() { m_nMaxReloadAmmo = 30; }

	virtual void SetType();
	virtual void OnPrepareAnimate();

	virtual int ShootNumber() { return 1; };
};