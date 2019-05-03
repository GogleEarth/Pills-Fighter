#include "stdafx.h"
#include "Weapon.h"
#include "Shader.h"
#include "Player.h"
#include "Scene.h"

extern CFMODSound gFmodSound;

CWeapon::CWeapon() : CGameObject()
{
}

CWeapon::~CWeapon()
{
}

void CWeapon::Initialize()
{
	SetType();
}

void CWeapon::SetOwnerTransform(XMFLOAT4X4 xmf4x4World)
{
	m_xmf3Right = XMFLOAT3(xmf4x4World._11, xmf4x4World._12, xmf4x4World._13);
	m_xmf3Up = XMFLOAT3(xmf4x4World._21, xmf4x4World._22, xmf4x4World._23);
	m_xmf3Look = XMFLOAT3(xmf4x4World._31, xmf4x4World._32, xmf4x4World._33);
	m_xmf3Position = XMFLOAT3(xmf4x4World._41, xmf4x4World._42, xmf4x4World._43);
}

void CWeapon::Animate(float fTimeElapsed, CCamera *pCamera)
{
	if (m_pParentModel) SetOwnerTransform(m_pParentModel->GetWorldTransf());

	CGameObject::Animate(fTimeElapsed, pCamera);
}

void CWeapon::Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera, bool bSetTexture, int nInstances)
{
	if (m_pParentModel) SetOwnerTransform(m_pParentModel->GetWorldTransf());

	CGameObject::Render(pd3dCommandList, pCamera, nInstances);
}

void CWeapon::RenderWire(ID3D12GraphicsCommandList *pd3dCommandList, CCamera* pCamera, int nInstances)
{
	if (m_pParentModel) SetOwnerTransform(m_pParentModel->GetWorldTransf());

	CGameObject::RenderWire(pd3dCommandList, pCamera, nInstances);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

CGun::CGun() : CWeapon()
{
}

CGun::~CGun()
{
}

void CGun::Initialize()
{
	CWeapon::Initialize();

	SetReloadTime();
		SetMaxReloadAmmo();
}

void CGun::Reload(int& nAmmo)
{
	m_nReloadedAmmo += nAmmo;

	if (m_nReloadedAmmo > m_nMaxReloadAmmo)
	{
		nAmmo = m_nReloadedAmmo - m_nMaxReloadAmmo;
		m_nReloadedAmmo = m_nMaxReloadAmmo;
	}
	else nAmmo = 0;
}

void CGun::Shot(FMOD::Sound *pFmodSound)
{
	CPlayer *pPlayer = (CPlayer*)m_pOwner;

#ifndef ON_NETWORKING
	Bullet *pBullet = NULL;
	pBullet = new Bullet();

	if (!m_pMuzzle) return;

	XMFLOAT4X4 xmf4x4World = pPlayer->GetToTarget(m_pMuzzle->GetPosition());

	pBullet->SetRight(XMFLOAT3(xmf4x4World._11, xmf4x4World._12, xmf4x4World._13));
	pBullet->SetUp(XMFLOAT3(xmf4x4World._21, xmf4x4World._22, xmf4x4World._23));
	pBullet->SetLook(XMFLOAT3(xmf4x4World._31, xmf4x4World._32, xmf4x4World._33));
	pBullet->SetPosition(XMFLOAT3(xmf4x4World._41, xmf4x4World._42, xmf4x4World._43));

	gFmodSound.PlayFMODSound(pFmodSound);
	m_pBulletShader->InsertObject(m_pd3dDevice, m_pd3dCommandList, pBullet, m_nBulletGroup, true, NULL);
#else
	pPlayer->IsShotable(true);
#endif

	SetShotCoolTime();

	m_nReloadedAmmo--;
}

void CGun::Animate(float ElapsedTime, CCamera *pCamera)
{
	if (m_fShotCoolTime > 0.0f)
	{
		m_fShotCoolTime -= ElapsedTime;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

CGimGun::CGimGun() : CGun()
{
}

CGimGun::~CGimGun()
{
}

void CGimGun::Initialize()
{
	CGun::Initialize();

	m_nReloadedAmmo = 30;
}

void CGimGun::SetType()
{
	m_nType |= WEAPON_TYPE_OF_GIM_GUN;

	CGun::SetType();
}

void CGimGun::Attack()
{
	if (m_fBurstCoolTime < 0.0f && m_nReloadedAmmo > 0)
	{
		m_bBurst = true;
		SetBurstCoolTime();
	}
}

void CGimGun::Animate(float ElapsedTime, CCamera *pCamera)
{
	if (m_bBurst)
	{
		if (m_fShotCoolTime <= 0.0f)
		{
			Shot(gFmodSound.m_pSoundGGShot);

			m_nShotCount++;
		}

		if (m_nShotCount >= 3 || m_nReloadedAmmo == 0)
		{
			m_nShotCount = 0;
			m_bBurst = false;
		}
	}
	else
	{
		m_fBurstCoolTime -= ElapsedTime;
	}

	CGun::Animate(ElapsedTime, pCamera);
}

void CGimGun::OnPrepareAnimate()
{
	m_pMuzzle = m_pModel->FindFrame("Bone001");
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

CBazooka::CBazooka() : CGun()
{
}

CBazooka::~CBazooka()
{
}

void CBazooka::Initialize()
{
	CGun::Initialize();

	m_nReloadedAmmo = 5;
}

void CBazooka::SetType()
{
	m_nType |= WEAPON_TYPE_OF_BAZOOKA;

	CGun::SetType();
}

void CBazooka::Attack()
{
	if (m_nReloadedAmmo > 0 && m_fShotCoolTime <= 0.0f)
	{
		Shot(gFmodSound.m_pSoundBZKShot);
		SetShotCoolTime();
	}
}

void CBazooka::OnPrepareAnimate()
{
	m_pMuzzle = m_pModel->FindFrame("Bone001");
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

CMachineGun::CMachineGun() : CGun()
{
}

CMachineGun::~CMachineGun()
{
}

void CMachineGun::Initialize()
{
	CGun::Initialize();

	m_nReloadedAmmo = 30;
}

void CMachineGun::SetType()
{
	m_nType |= WEAPON_TYPE_OF_MACHINEGUN;

	CGun::SetType();
}

void CMachineGun::Attack()
{
	if (m_nReloadedAmmo > 0 && m_fShotCoolTime <= 0.0f)
	{
		Shot(gFmodSound.m_pSoundMGShot);
		SetShotCoolTime();
	}
}

void CMachineGun::OnPrepareAnimate()
{
	m_pMuzzle = m_pModel->FindFrame("Bone001");
}
