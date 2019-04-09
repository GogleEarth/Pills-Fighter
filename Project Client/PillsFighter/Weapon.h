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

class CGun : public CWeapon
{
public:
	CGun();
	virtual ~CGun();

	virtual void Initialize();


	CObjectsShader *m_pBulletShader = NULL;
	int	m_nBulletGroup;

	float m_fShotCoolTime = 0.0f;

	int m_nReloadedAmmo = 0;
	int m_nMaxReloadAmmo = 0;
	float m_fReloadTime = 0.0f;

public:
	float GetReloadTime() { return m_fReloadTime; }
	int GetMaxReloadAmmo() { return m_nMaxReloadAmmo; }
	int GetReloadedAmmo() { return m_nReloadedAmmo; }
	void SetBullet(CShader *Bullet, int nGroup) { m_pBulletShader = (CObjectsShader*)Bullet; m_nBulletGroup = nGroup; }

	virtual void Reload(int& nAmmo);

public:
	virtual void Attack() {};
	virtual void Shot(FMOD::Sound *pFmodSound);

	virtual void SetReloadTime() {}
	virtual void SetShotCoolTime() {}
	virtual void SetBurstCoolTime() {}
	virtual void SetMaxReloadAmmo() {}
	virtual void SetType() { m_nType |= WEAPON_TYPE_OF_GUN; }

	virtual void Animate(float ElapsedTime, CCamera *pCamera = NULL);
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
	int		m_nShotCount = 0;

public:
	virtual void Attack();

	virtual void SetReloadTime() { m_fReloadTime = GG_RELOAD_TIME; }
	virtual void SetShotCoolTime() { m_fShotCoolTime = GG_SHOT_COOLTIME; }
	virtual void SetBurstCoolTime() { m_fBurstCoolTime = GG_BURST_COOLTIME; }
	virtual void SetMaxReloadAmmo() { m_nMaxReloadAmmo = 30; }
	virtual void SetType();

	virtual void Animate(float ElapsedTime, CCamera *pCamera = NULL);
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

	virtual void Attack();

	virtual void SetReloadTime() { m_fReloadTime = BZK_RELOAD_TIME; }
	virtual void SetShotCoolTime() { m_fShotCoolTime = BZK_SHOT_COOLTIME; }
	virtual void SetMaxReloadAmmo() { m_nMaxReloadAmmo = 5; }

	virtual void SetType();
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

	virtual void Attack();

	virtual void SetReloadTime() { m_fReloadTime = MG_RELOAD_TIME; }
	virtual void SetShotCoolTime() { m_fShotCoolTime = MG_SHOT_COOLTIME; }
	virtual void SetMaxReloadAmmo() { m_nMaxReloadAmmo = 30; }

	virtual void SetType();
};