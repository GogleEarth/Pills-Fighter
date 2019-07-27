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

void CWeapon::Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera, bool bSetTexture, bool bSetShader, int nInstances)
{
	if (m_pParentModel) SetOwnerTransform(m_pParentModel->GetWorldTransf());

	CGameObject::Render(pd3dCommandList, pCamera, bSetTexture, bSetShader, nInstances);
}

void CWeapon::RenderToShadow(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera, bool bSetTexture, bool bSetShader, int nInstances)
{
	if (m_pParentModel) SetOwnerTransform(m_pParentModel->GetWorldTransf());

	CGameObject::Render(pd3dCommandList, pCamera, bSetTexture, bSetShader, nInstances);
}

void CWeapon::RenderWire(ID3D12GraphicsCommandList *pd3dCommandList, CCamera* pCamera, int nInstances)
{
	if (m_pParentModel) SetOwnerTransform(m_pParentModel->GetWorldTransf());

	CGameObject::RenderWire(pd3dCommandList, pCamera, nInstances);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

CSword::CSword() : CWeapon()
{

}

CSword::~CSword()
{

}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

CSaber::CSaber() : CSword()
{

}

CSaber::~CSaber()
{

}

void CSaber::SetType()
{
	CSword::SetType();  

	m_nType |= WEAPON_TYPE_OF_SABER;
};


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

CTomahawk::CTomahawk() : CSword()
{

}

CTomahawk::~CTomahawk()
{

}

void CTomahawk::SetType()
{
	CSword::SetType();

	m_nType |= WEAPON_TYPE_OF_TOMAHAWK;
};

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

void CGun::SetShotCoolTime()
{
	SetShootable(false);
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

	if (!m_pMuzzle) return;

	XMFLOAT4X4 xmf4x4World = pPlayer->GetToTarget(m_pMuzzle->GetPosition());

	pBullet->SetRight(XMFLOAT3(xmf4x4World._11, xmf4x4World._12, xmf4x4World._13));
	pBullet->SetUp(XMFLOAT3(xmf4x4World._21, xmf4x4World._22, xmf4x4World._23));
	pBullet->SetLook(XMFLOAT3(xmf4x4World._31, xmf4x4World._32, xmf4x4World._33));
	pBullet->SetPosition(XMFLOAT3(xmf4x4World._41, xmf4x4World._42, xmf4x4World._43));

	m_pBulletShader->InsertObject(m_pd3dDevice, m_pd3dCommandList, pBullet, m_nBulletGroup, true, NULL);

	m_pEffectShader->AddEffect(TIMED_EFFECT_INDEX_MUZZLE_FIRE, XMFLOAT3(xmf4x4World._41, xmf4x4World._42, xmf4x4World._43), XMFLOAT2(2.5f, 2.5f), 0, TIMED_EFFECT_INDEX_MUZZLE_FIRE_TEXTURES);
#else
	pPlayer->Shoot();
#endif

	SetShotCoolTime();
	
	m_nShootCount++;
	m_nReloadedAmmo--;
}

void CGun::CheckShootable(float fElapsedTime)
{
	if (m_nReloadedAmmo > 0)
	{
		m_bShootable = true;
	}
}

void CGun::SetShootCount()
{
	m_nShootCount = 0;
}

void CGun::Animate(float fElapsedTime, CCamera *pCamera)
{
	if (m_fShotCoolTime > 0.0f)
	{
		m_fShotCoolTime -= fElapsedTime;
	}
	else
	{
		CheckShootable(fElapsedTime);
	}

	if (ShootedCount() == ShootNumber())
	{
		SetShootCount();
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
	m_nType |= WEAPON_TYPE_OF_GM_GUN;

	CGun::SetType();
}

void CGimGun::SetShotCoolTime()
{
	CGun::SetShotCoolTime();

	m_fShotCoolTime = GG_SHOT_COOLTIME;
}

void CGimGun::OnPrepareAnimate()
{
	m_pMuzzle = m_pModel->FindFrame("Bone001");
}

void CGimGun::Shot()
{
	CGun::Shot();

#ifndef ON_NETWORKING
	gFmodSound.PlayFMODSound(gFmodSound.m_pSoundGGShot);
#endif
}

void CGimGun::CheckShootable(float fElapsedTime)
{
	if (m_nReloadedAmmo > 0)
	{
		if (m_fBurstCoolTime > 0.0f)
			m_fBurstCoolTime -= fElapsedTime;
		else
			m_bShootable = true;
	}
}

void CGimGun::SetShootCount()
{
	SetBurstCoolTime();
	m_nShootCount = 0;
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

void CBazooka::SetShotCoolTime()
{
	CGun::SetShotCoolTime();

	m_fShotCoolTime = BZK_SHOT_COOLTIME;
}

void CBazooka::OnPrepareAnimate()
{
	m_pMuzzle = m_pModel->FindFrame("Bone001");
}

void CBazooka::Shot()
{
	CGun::Shot();

#ifndef ON_NETWORKING
	gFmodSound.PlayFMODSound(gFmodSound.m_pSoundBZKShot);
#endif
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

void CMachineGun::SetShotCoolTime()
{
	CGun::SetShotCoolTime();

	m_fShotCoolTime = MG_SHOT_COOLTIME;
}

void CMachineGun::OnPrepareAnimate()
{
	m_pMuzzle = m_pModel->FindFrame("Bone001");
}

void CMachineGun::Shot()
{
	CGun::Shot();

#ifndef ON_NETWORKING
	gFmodSound.PlayFMODSound(gFmodSound.m_pSoundMGShot);
#endif
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

CBeamRifle::CBeamRifle() : CGun()
{
}

CBeamRifle::~CBeamRifle()
{
}

void CBeamRifle::Initialize()
{
	CGun::Initialize();

	m_nReloadedAmmo = 100;
}

void CBeamRifle::SetType()
{
	m_nType |= WEAPON_TYPE_OF_BEAM_RIFLE;

	CGun::SetType();
}

void CBeamRifle::SetShotCoolTime()
{
	CGun::SetShotCoolTime();

	m_fShotCoolTime = BR_SHOT_COOLTIME;
}

void CBeamRifle::OnPrepareAnimate()
{
	m_pMuzzle = m_pModel->FindFrame("Bone001");
}

void CBeamRifle::Shot()
{
	CPlayer *pPlayer = (CPlayer*)m_pOwner;

#ifndef ON_NETWORKING
	Bullet *pBullet = NULL;
	pBullet = new Bullet();

	if (!m_pMuzzle) return;

	XMFLOAT3 xmf3MuzzlePos = m_pMuzzle->GetPosition();

	XMFLOAT3 xmf3TargetPos = pPlayer->GetToTargetPosition(xmf3MuzzlePos);

	XMFLOAT3 xmf3Distance = Vector3::Subtract(xmf3TargetPos, xmf3MuzzlePos);
	XMFLOAT3 xmf3Look = Vector3::Normalize(xmf3Distance);
	float fDistance = Vector3::Length(xmf3Distance);

	m_pEffectShader->AddEffectWithLookV(LASER_EFFECT_INDEX_LASER_BEAM, xmf3MuzzlePos, XMFLOAT2(3.0f, fDistance), xmf3Look, 0, LASER_EFFECT_INDEX_LASER_BEAM_TEXTURE_COUNT);

#else
	pPlayer->Shoot();
#endif

	SetShotCoolTime();
	pPlayer->SetReloadTime(m_fReloadTime);

	m_nShootCount++;
	m_nReloadedAmmo = m_nReloadedAmmo - 10 < 0 ? 0 : m_nReloadedAmmo - 10;

#ifndef ON_NETWORKING
	gFmodSound.PlayFMODSound(gFmodSound.m_pSoundBeamRifle);
#endif
}

void CBeamRifle::CheckShootable(float fElapsedTime)
{
	if (m_nReloadedAmmo >= 10)
	{
		m_bShootable = true;
	}
}

void CBeamRifle::Charge()
{ 
	if (m_nReloadedAmmo < m_nMaxReloadAmmo)
	{
		m_nReloadedAmmo++;
		SetReloadTime();
	}
}