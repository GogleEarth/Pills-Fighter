#include "stdafx.h"
#include "Weapon.h"
#include "Shader.h"
#include "Player.h"
#include "Scene.h"

extern CFMODSound gFmodSound;

CWeapon::CWeapon() : CGameObject()
{
	m_bRender = false;
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

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

CSword::CSword() : CWeapon()
{

}

CSword::~CSword()
{

}

void CSword::OnPrepareAnimate()
{
	m_pBlade = m_pModel->FindFrame("Bone001");
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

void CGun::OnPrepareAnimate()
{
	m_pMuzzle = m_pModel->FindFrame("Bone001");
}

void CGun::SetShotCoolTime()
{
	SetShootable(false);

	if (m_nShootedCount == ShootNumber())
	{
		ResetShootCount();
		m_bCoolDown = true;
	}
	else
		m_bCoolDown = false;

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

	XMFLOAT3 xmf3MuzzlePos = m_pMuzzle->GetPosition();
	XMFLOAT4X4 xmf4x4World = pPlayer->GetToTarget(xmf3MuzzlePos);

	pBullet->SetRight(XMFLOAT3(xmf4x4World._11, xmf4x4World._12, xmf4x4World._13));
	pBullet->SetUp(XMFLOAT3(xmf4x4World._21, xmf4x4World._22, xmf4x4World._23));
	pBullet->SetLook(XMFLOAT3(xmf4x4World._31, xmf4x4World._32, xmf4x4World._33));
	pBullet->SetPosition(XMFLOAT3(xmf4x4World._41, xmf4x4World._42, xmf4x4World._43));

	m_pBulletShader->InsertObject(m_pd3dDevice, m_pd3dCommandList, pBullet, m_nBulletGroup, true, NULL);

	((CEffectShader*)m_pEffectShader)->AddEffect(TIMED_EFFECT_INDEX_MUZZLE_FIRE, xmf3MuzzlePos, XMFLOAT2(2.5f, 2.5f), 0, rand() % 360, XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));
#else
	pPlayer->SendShootPacket();
#endif

	m_nShootedCount++;
	m_nReloadedAmmo--;

	SetShotCoolTime();
}

void CGun::CheckShootable(float fElapsedTime)
{
	if (m_nReloadedAmmo > 0)
	{
		m_bShootable = true;
	}
}

void CGun::Animate(float fElapsedTime, CCamera *pCamera)
{
	CWeapon::Animate(fElapsedTime, pCamera);

	if (m_fShotCoolTime > 0.0f)
	{
		m_fShotCoolTime -= fElapsedTime;
	}
	else
	{
		CheckShootable(fElapsedTime);
	}
}

void CGun::Reset()
{ 
	m_bCoolDown = true; 
	m_bShootable = true; 
	m_nReloadedAmmo = m_nMaxReloadAmmo;
	ResetShootCount();
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

void CGimGun::Shot()
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

	((CEffectShader*)m_pBulletShader)->AddEffectWithLookV(LASER_EFFECT_INDEX_LASER_BEAM, xmf3MuzzlePos, XMFLOAT2(1.0f, fDistance), xmf3Look, 0, XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));

	float fSize = SPRITE_EFFECT_BEAM_HIT_SIZE;
	((CEffectShader*)m_pEffectShader)->AddEffect(SPRITE_EFFECT_INDEX_BEAM_HIT, xmf3TargetPos, XMFLOAT2(fSize, fSize), EFFECT_ANIMATION_TYPE_ONE, 360, XMFLOAT4(1.0f, 0.6f, 1.0f, 1.0f));

#else
	pPlayer->SendShootPacket();
#endif

	m_nShootedCount++;
	m_nReloadedAmmo--;

	SetShotCoolTime();

#ifndef ON_NETWORKING
	gFmodSound.PlayFMODSound(gFmodSound.m_pSoundBeamRifle);
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

void CGimGun::ResetShootCount()
{
	CGun::ResetShootCount();

	SetBurstCoolTime();
}

void CGimGun::Reset()
{
	CGun::Reset();

	SetBurstCoolTime();
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

void CMachineGun::Shot()
{
	CGun::Shot();

#ifndef ON_NETWORKING
	gFmodSound.PlayFMODSound(gFmodSound.m_pSoundMGShot);
#endif
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

CBeamGun::CBeamGun() : CGun()
{
}

CBeamGun::~CBeamGun()
{
}

void CBeamGun::Initialize()
{
	CGun::Initialize();

	m_nReloadedAmmo = 100;
	SetShootEnergy();
	SetSizeX();	
}

void CBeamGun::SetType()
{
	m_nType |= WEAPON_TYPE_OF_BEAM_GUN;

	CGun::SetType();
}

void CBeamGun::Shot()
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

	((CEffectShader*)m_pBulletShader)->AddEffectWithLookV(LASER_EFFECT_INDEX_LASER_BEAM, xmf3MuzzlePos, XMFLOAT2(m_fSizeX, fDistance), xmf3Look, 0, XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));

	float fSize = SPRITE_EFFECT_BEAM_HIT_SIZE;
	((CSpriteShader*)m_pEffectShader)->AddEffect(SPRITE_EFFECT_INDEX_BEAM_HIT, xmf3TargetPos, XMFLOAT2(fSize, fSize), EFFECT_ANIMATION_TYPE_ONE, 360, XMFLOAT4(1.0f, 0.6f, 1.0f, 1.0f));
#else
	pPlayer->SendShootPacket();
#endif

	m_nShootedCount++;
	m_nReloadedAmmo = m_nReloadedAmmo - m_nShootEnergy < 0 ? 0 : m_nReloadedAmmo - m_nShootEnergy;

	SetShotCoolTime();

#ifndef ON_NETWORKING
	gFmodSound.PlayFMODSound(gFmodSound.m_pSoundBeamRifle);
#endif
}

void CBeamGun::CheckShootable(float fElapsedTime)
{
	if (m_nReloadedAmmo >= m_nShootEnergy)
	{
		m_bShootable = true;
	}
}

void CBeamGun::Animate(float fElapsedTime, CCamera *pCamera)
{
	if (m_nReloadedAmmo < m_nMaxReloadAmmo)
	{
		m_fReloadTime -= fElapsedTime;

		if (m_fReloadTime < 0.0f)
		{
			m_nReloadedAmmo++;
			SetReloadTime();
			((CPlayer*)m_pOwner)->ChangeUIAmmo();
		}
	}

	CGun::Animate(fElapsedTime, pCamera);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

CBeamRifle::CBeamRifle()
{

}

CBeamRifle::~CBeamRifle()
{

}

void CBeamRifle::SetType()
{
	m_nType |= WEAPON_TYPE_OF_BEAM_RIFLE;

	CBeamGun::SetType();
}

void CBeamRifle::SetShotCoolTime()
{
	CGun::SetShotCoolTime();

	m_fShotCoolTime = BR_SHOT_COOLTIME;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

CBeamSniper::CBeamSniper()
{

}

CBeamSniper::~CBeamSniper()
{

}

void CBeamSniper::SetType()
{
	m_nType |= WEAPON_TYPE_OF_BEAM_SNIPER;

	CBeamGun::SetType();
}

void CBeamSniper::SetShotCoolTime()
{
	CGun::SetShotCoolTime();

	m_fShotCoolTime = BS_SHOT_COOLTIME;
}

void CBeamSniper::OnPrepareAnimate()
{
	CBeamGun::OnPrepareAnimate();

	m_pScope = m_pModel->FindFrame("Bone002");
}