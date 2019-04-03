#include "stdafx.h"
#include "Weapon.h"
#include "Shader.h"
#include "Player.h"
#include "Scene.h"

extern CSound gSound;

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

void CWeapon::Animate(float fTimeElapsed, CCamera *pCamera)
{
	if (m_pModel)
	{
		if (m_pParentModel) m_xmf4x4World = m_pParentModel->GetWorldTransf();

		UpdateWorldTransform();

		int i = 0;
		m_pModel->UpdateCollisionBox(m_vxmAABB, &i);
	}
}

void CWeapon::Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera* pCamera)
{
	if (m_pModel)
	{
		if (m_pParentModel) m_xmf4x4World = m_pParentModel->GetWorldTransf();

		UpdateWorldTransform();

		int i = 0;
		m_pModel->Render(pd3dCommandList, pCamera, m_vd3dcbGameObject, m_vcbMappedGameObject, &i);
	}
}

void CWeapon::RenderWire(ID3D12GraphicsCommandList *pd3dCommandList, CCamera* pCamera)
{
	if (m_pModel)
	{
		if (m_pParentModel) m_xmf4x4World = m_pParentModel->GetWorldTransf();

		UpdateWorldTransform();

		int i = 0;
		m_pModel->RenderWire(pd3dCommandList, pCamera, m_vd3dcbGameObject, m_vcbMappedGameObject, &i);
	}
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

void CGun::Shot()
{
	CPlayer *pPlayer = (CPlayer*)m_pOwner;

#ifndef ON_NETWORKING
	Bullet *pBullet = NULL;
	pBullet = new Bullet();

	XMFLOAT4X4 xmf4x4World = pPlayer->GetToTarget();

	pBullet->SetRight(XMFLOAT3(xmf4x4World._11, xmf4x4World._12, xmf4x4World._13));
	pBullet->SetUp(XMFLOAT3(xmf4x4World._21, xmf4x4World._22, xmf4x4World._23));
	pBullet->SetLook(XMFLOAT3(xmf4x4World._31, xmf4x4World._32, xmf4x4World._33));
	pBullet->SetPosition(XMFLOAT3(xmf4x4World._41, xmf4x4World._42, xmf4x4World._43));

	m_pBulletShader->InsertObject(m_pd3dDevice, m_pd3dCommandList, pBullet);
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
			Shot();

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

void CGimGun::Shot()
{
	CGun::Shot();

	CRobotObjectSound *pSound = ((CPlayer*)m_pOwner)->m_pSound;
	if (pSound) pSound->PlayFMODSound(pSound->m_pSoundGGShot);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

CBazooka::CBazooka() : CGun()
{
}

CBazooka::~CBazooka()
{
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
		Shot();
		SetShotCoolTime();
	}
}

void CBazooka::Shot()
{
	CGun::Shot();

	CRobotObjectSound *pSound = ((CPlayer*)m_pOwner)->m_pSound;
	if (pSound) pSound->PlayFMODSound(pSound->m_pSoundBZKShot);
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

CMachineGun::CMachineGun() : CGun()
{
}

CMachineGun::~CMachineGun()
{
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
		Shot();
		SetShotCoolTime();
	}
}

void CMachineGun::Shot()
{
	CGun::Shot();

	CRobotObjectSound *pSound = ((CPlayer*)m_pOwner)->m_pSound;
	if(pSound) pSound->PlayFMODSound(pSound->m_pSoundMGShot);
}