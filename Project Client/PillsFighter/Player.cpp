#include "stdafx.h"
#include "Player.h"
#include "Shader.h"
#include "Repository.h"
#include "Scene.h"
#include "Animation.h"
#include "Weapon.h"

#define CAMERA_POSITION XMFLOAT3(0.0f, 30.0f, -35.0f)

CPlayer::CPlayer(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature, CRepository *pRepository, void *pContext) : CRobotObject()
{
	m_pCamera = SetCamera(0.0f);

	m_nHitPoint = m_nMaxHitPoint = 100;
	SetPosition(XMFLOAT3(0.0f, 0.0f, -50.0f));

	AddPrepareRotate(0.0f, 180.0f, 0.0f);

	//CModel *pModel = pRepository->GetModel(pd3dDevice, pd3dCommandList, "./Resource/GM/GM.bin", true);
	CModel *pModel = pRepository->GetModel(pd3dDevice, pd3dCommandList, "./Resource/GM/GM.bin", true);
	//CModel *pModel = pRepository->GetModel(pd3dDevice, pd3dCommandList, "./Resource/Player/Angrybot.bin", true);
	m_pAnimationController = new CAnimationController(1, pModel->GetAnimationSet());
	m_pAnimationController->SetTrackAnimation(0, 0);

	SetModel(pModel);
	
	CreateShaderVariables(pd3dDevice, pd3dCommandList);

	CUserInterface *pUserInterface = new CUserInterface();
	pUserInterface->CreateShader(pd3dDevice, pd3dGraphicsRootSignature);
	pUserInterface->Initialize(pd3dDevice, pd3dCommandList, NULL);
	pUserInterface->SetPlayer(this);

	m_pUserInterface = pUserInterface;

	CHeightMapTerrain *pTerrain = (CHeightMapTerrain *)pContext;

	SetPlayerUpdatedContext(pTerrain);
	SetCameraUpdatedContext(pTerrain);
}

CPlayer::~CPlayer()
{
	ReleaseShaderVariables();

	if (m_pCamera) delete m_pCamera;
	if (m_pUserInterface) delete m_pUserInterface;

}

void CPlayer::CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList)
{
	if (m_pCamera) m_pCamera->CreateShaderVariables(pd3dDevice, pd3dCommandList);

	CGameObject::CreateShaderVariables(pd3dDevice, pd3dCommandList);
}

void CPlayer::ReleaseShaderVariables()
{
	if (m_pCamera) m_pCamera->ReleaseShaderVariables();
	if (m_pUserInterface) m_pUserInterface->ReleaseShaderVariables();
}

void CPlayer::ReleaseUploadBuffers()
{
	if (m_pUserInterface) m_pUserInterface->ReleaseUploadBuffers();

	CGameObject::ReleaseUploadBuffers();
}

CCamera *CPlayer::SetCamera(float fTimeElapsed)
{
	m_pCamera = new CCamera();

	m_xmf3Right = m_pCamera->GetRightVector();
	m_xmf3Up = m_pCamera->GetUpVector();
	m_xmf3Look = m_pCamera->GetLookVector();

	m_pCamera->SetPlayer(this);

	m_pCamera->SetTimeLag(0.0f);
	m_pCamera->SetOffset(CAMERA_POSITION);
	m_pCamera->GenerateProjectionMatrix(1.01f, 5000.0f, ASPECT_RATIO, 60.0f);
	m_pCamera->SetViewport(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 0.0f, 1.0f);
	m_pCamera->SetScissorRect(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT);

	Update(fTimeElapsed);

	return(m_pCamera);
}

void CPlayer::Move(ULONG dwDirection, float fDistance)
{
	if (dwDirection)
	{

		XMFLOAT3 xmf3Shift = XMFLOAT3(0, 0, 0);

		if (dwDirection & DIR_FORWARD)
		{
			ChangeAnimation(ANIMATION_STATE_WALK_FORWARD);
			xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Look, fDistance);
		}
		if (dwDirection & DIR_BACKWARD)
		{
			ChangeAnimation(ANIMATION_STATE_WALK_BACKWARD);
			xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Look, -fDistance);
		}
		if (dwDirection & DIR_RIGHT)
		{
			ChangeAnimation(ANIMATION_STATE_WALK_RIGHT);
			xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Right, fDistance);
		}
		if (dwDirection & DIR_LEFT)
		{
			ChangeAnimation(ANIMATION_STATE_WALK_LEFT);
			xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Right, -fDistance);
		}

		if (dwDirection & DIR_UP) ActivationBooster();

		if (dwDirection & DIR_DOWN);

		Move(xmf3Shift);
	}		
}

void CPlayer::Move(const XMFLOAT3& xmf3Shift)
{
	m_xmf3Position = Vector3::Add(m_xmf3Position, xmf3Shift);

	m_pCamera->Move(xmf3Shift);
}

void CPlayer::Update(float fTimeElapsed)
{
	ProcessGravity(fTimeElapsed);
	ProcessBoosterGauge(fTimeElapsed);
	ProcessHitPoint();
	ProcessTime(m_pRHWeapon, fTimeElapsed);

	if (m_pPlayerUpdatedContext) OnPlayerUpdateCallback(fTimeElapsed);
	m_pCamera->Update(fTimeElapsed);
	if (m_pCameraUpdatedContext) OnCameraUpdateCallback(fTimeElapsed);
	XMFLOAT3 xmf3LookAt = Vector3::Add(m_xmf3Position, XMFLOAT3(0.0f, 20.0f, 0.0f));
	m_pCamera->SetLookAt(xmf3LookAt);

	CRobotObject::Animate(fTimeElapsed);
}

void CPlayer::Rotate(float x, float y, float z)
{
	if (!IsZero(y))
	{
		m_fYaw += y;
		if (m_fYaw > 360.0f) m_fYaw -= 360.0f;
		if (m_fYaw < 0.0f) m_fYaw += 360.0f;

		XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3Up), XMConvertToRadians(y));
		m_xmf3Look = Vector3::TransformNormal(m_xmf3Look, xmmtxRotate);
		m_xmf3Right = Vector3::TransformNormal(m_xmf3Right, xmmtxRotate);
	}

	m_xmf3Look = Vector3::Normalize(m_xmf3Look);
	m_xmf3Right = Vector3::CrossProduct(m_xmf3Up, m_xmf3Look, true);
	m_xmf3Up = Vector3::CrossProduct(m_xmf3Look, m_xmf3Right, true);

	m_pCamera->Rotate(x, y, 0.0f);
}

void CPlayer::Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera)
{
	if (m_pUserInterface) m_pUserInterface->Render(pd3dCommandList, pCamera);

	CRobotObject::Render(pd3dCommandList, pCamera);
}

XMFLOAT4X4 CPlayer::GetToTarget()
{
	float fDistance = m_pScene->GetToTargetDistance();

	XMFLOAT3 xmf3CameraPos = m_pCamera->GetPosition();
	XMFLOAT3 xmf3CameraLook = m_pCamera->GetLookVector();
	XMFLOAT3 xmf3DestPos = XMFLOAT3(xmf3CameraPos.x + xmf3CameraLook.x * fDistance,
		xmf3CameraPos.y + xmf3CameraLook.y * fDistance,
		xmf3CameraPos.z + xmf3CameraLook.z * fDistance);

	XMFLOAT3 xmf3Position = Vector3::Add(GetPosition(), XMFLOAT3(0.0f, 5.0f, 0.0f));
	XMFLOAT3 xmf3Right = m_pCamera->GetRightVector();
	XMFLOAT3 xmf3Look = Vector3::Normalize(Vector3::Subtract(xmf3DestPos, xmf3Position));
	XMFLOAT3 xmf3Up = Vector3::CrossProduct(xmf3Right, xmf3Look, true);
	xmf3Right = Vector3::CrossProduct(xmf3Up, xmf3Look, true);

	return XMFLOAT4X4(xmf3Right.x, xmf3Right.y, xmf3Right.z, 0.0f,
		xmf3Up.x, xmf3Up.y, xmf3Up.z, 0.0f,
		xmf3Look.x, xmf3Look.y, xmf3Look.z, 0.0f,
		xmf3Position.x, xmf3Position.y, xmf3Position.z, 1.0f);
}

void CPlayer::OnPlayerUpdateCallback(float fTimeElapsed)
{
	//if(m_pScene) m_pScene->CheckCollisionPlayer();

	CHeightMapTerrain *pTerrain = (CHeightMapTerrain *)m_pPlayerUpdatedContext;
	XMFLOAT3 xmf3Scale = pTerrain->GetScale();
	XMFLOAT3 xmf3PlayerPosition = GetPosition();
	int z = (int)(xmf3PlayerPosition.z / xmf3Scale.z);
	bool bReverseQuad = ((z % 2) != 0);
	float fHeight = pTerrain->GetHeight(xmf3PlayerPosition.x, xmf3PlayerPosition.z, bReverseQuad);
	if (xmf3PlayerPosition.y < fHeight)
	{
		float fPlayerVelocity = GetVelocity();
		fPlayerVelocity = 0.0f;
		SetVelocity(fPlayerVelocity);
		xmf3PlayerPosition.y = fHeight;
		SetPosition(xmf3PlayerPosition);
	}
}

void CPlayer::OnCameraUpdateCallback(float fTimeElapsed)
{
	//m_pScene->CheckCollisionCamera();

	CHeightMapTerrain *pTerrain = (CHeightMapTerrain *)m_pCameraUpdatedContext;
	XMFLOAT3 xmf3Scale = pTerrain->GetScale();
	XMFLOAT3 xmf3CameraPosition = m_pCamera->GetPosition();
	int z = (int)(xmf3CameraPosition.z / xmf3Scale.z);
	bool bReverseQuad = ((z % 2) != 0);
	float fHeight = pTerrain->GetHeight(xmf3CameraPosition.x, xmf3CameraPosition.z, bReverseQuad)+5.0f;
	if (xmf3CameraPosition.y <= fHeight)
	{
		xmf3CameraPosition.y = fHeight;
		m_pCamera->SetPosition(xmf3CameraPosition);
	}
}

void CPlayer::ProcessHitPoint()
{
	if (m_nHitPoint < 0)
	{
		m_nHitPoint = 100;
		SetPosition(serverPosition);
	}
}

void CPlayer::ActivationBooster()
{
	if (!(m_nState & OBJECT_STATE_BOOSTERING))
	{
		m_nState |= OBJECT_STATE_BOOSTERING;
		m_bChargeBG = false;
		SetElapsedBGConsumeTime();
	}
}

void CPlayer::DeactivationBooster()
{
	m_nState &= ~OBJECT_STATE_BOOSTERING;
}

void CPlayer::ProcessBoosterGauge(float fTimeElapsed)
{
	if (m_nState & OBJECT_STATE_BOOSTERING)
	{
		if (m_fElapsedBGConsumeTime <= 0.0f)
		{
			SetElapsedBGConsumeTime();
			m_nBoosterGauge = ((m_nBoosterGauge - 5) <= 0) ? 0 : m_nBoosterGauge - 5;
		}
		else m_fElapsedBGConsumeTime -= fTimeElapsed;

		if (m_nBoosterGauge == 0)
		{
			DeactivationBooster();
			m_nBoosterGauge = 0;
		}
		else
			m_fKeepBoosteringTime += fTimeElapsed;
	}
	else
	{
		if (!m_bChargeBG)
		{
			if (m_fKeepBoosteringTime <= 0.0f)
			{
				m_bChargeBG = true;
				SetElapsedBGChargeTime();
				m_fKeepBoosteringTime = 0.0f;
			}
			else
			{
				m_bChargeBG = false;
				m_fKeepBoosteringTime -= fTimeElapsed;
			}
		}

		if (m_nBoosterGauge < 100)
		{
			if (m_bChargeBG)
			{
				if (m_fElapsedBGChargeTime <= 0.0f)
				{
					SetElapsedBGChargeTime();
					m_nBoosterGauge = ((m_nBoosterGauge + 10) >= 100) ? 100 : m_nBoosterGauge + 10;
				}
				else m_fElapsedBGChargeTime -= fTimeElapsed;
			}
		}
	}
}

void CPlayer::ProcessGravity(float fTimeElapsed)
{
	m_fGravity = m_fGravAcc * m_fMass * fTimeElapsed;
	m_fAccelerationY = -m_fGravity;

	if (m_nState & OBJECT_STATE_BOOSTERING)
	{
		float fBoosterPower = m_fGravity * ( (m_fKeepBoosteringTime > m_fMaxBoosterPower) ? m_fMaxBoosterPower : m_fKeepBoosteringTime);
		m_fAccelerationY += fBoosterPower;
	}

	m_fVelocityY += (m_fAccelerationY * fTimeElapsed);
	Move(XMFLOAT3(0.0f, m_fVelocityY, 0.0f));
}

void CPlayer::ProcessMoveToCollision(BoundingBox *pxmAABB, BoundingBox *pxmObjAABB)
{
	XMFLOAT3 xmf3ObjMax = Vector3::Add(pxmObjAABB->Center, pxmObjAABB->Extents);
	XMFLOAT3 xmf3Min = Vector3::Subtract(pxmAABB->Center, pxmAABB->Extents);

	if (xmf3Min.y == xmf3ObjMax.y)
	{
		std::cout << "Set" << std::endl;
	}
}

void CPlayer::Attack(CWeapon *pWeapon)
{
	if (pWeapon)
	{
		int nType = pWeapon->GetType();

		if (nType & WEAPON_TYPE_OF_GUN)
		{
			CGun *pGun = (CGun*)pWeapon;

			if (!m_bReloading) pGun->Shot();
		}
	}
}

void CPlayer::PickUpAmmo(int nType, int nAmmo)
{
	if (nType & WEAPON_TYPE_OF_GIM_GUN) m_nGimGunAmmo += nAmmo;
	else if (nType & WEAPON_TYPE_OF_BAZOOKA) m_nBazookaAmmo += nAmmo;
	else if (nType & WEAPON_TYPE_OF_MACHINEGUN) m_nMachineGunAmmo += nAmmo;
}

void CPlayer::PrepareAttack(CWeapon *pWeapon)
{
	if (pWeapon)
	{
		int nType = pWeapon->GetType();

		if (nType & WEAPON_TYPE_OF_GUN)
		{
			CGun *pGun = (CGun*)pWeapon;

			if (pGun->GetReloadedAmmo() == 0)
			{
				if (nType & WEAPON_TYPE_OF_GIM_GUN)
				{
					if (m_nGimGunAmmo > 0) Reload(pGun);
				}
				else if (nType & WEAPON_TYPE_OF_BAZOOKA)
				{
					if (m_nBazookaAmmo > 0) Reload(pGun);
				}
				else if (nType & WEAPON_TYPE_OF_MACHINEGUN)
				{
					if (m_nMachineGunAmmo > 0) Reload(pGun);
				}
				
			}
		}
	}
}

void CPlayer::Reload(CWeapon *pWeapon)
{
	if (pWeapon)
	{
		CGun *pGun = (CGun*)pWeapon;

		if (!m_bReloading)
		{
			m_bReloading = true;
			m_fReloadTime = pGun->GetReloadTime();
		}
	}
}

void CPlayer::ProcessTime(CWeapon *pWeapon, float fTimeElapsed)
{
	if (pWeapon)
	{
		int nType = pWeapon->GetType();

		if (nType & WEAPON_TYPE_OF_GUN)
		{
			CGun *pGun = (CGun*)pWeapon;

			if (m_bReloading)
			{
				m_fReloadTime -= fTimeElapsed;

				if (m_fReloadTime < 0.0f)
				{
					if (nType & WEAPON_TYPE_OF_GIM_GUN)
					{
						if (m_nGimGunAmmo > 0) pGun->Reload(m_nGimGunAmmo);
					}
					else if (nType & WEAPON_TYPE_OF_BAZOOKA)
					{
						if (m_nBazookaAmmo > 0) pGun->Reload(m_nBazookaAmmo);
					}
					else if (nType & WEAPON_TYPE_OF_MACHINEGUN)
					{
						if (m_nMachineGunAmmo > 0) pGun->Reload(m_nMachineGunAmmo);
					}

					m_bReloading = false;
				}
			}
		}
	}
}

void CPlayer::ChangeWeapon(int nSlotIndex)
{
	CRobotObject::ChangeWeapon(nSlotIndex);

	m_bReloading = false;
	m_bWeaponChanged = TRUE;
}

WEAPON_TYPE CPlayer::GetWeaponType()
{
	if (m_pRHWeapon)
	{
		int nType = m_pRHWeapon->GetType();

		if (nType & WEAPON_TYPE_OF_GIM_GUN)
			return WEAPON_TYPE::WEAPON_TYPE_BEAM_RIFLE;
		else if (nType & WEAPON_TYPE_OF_MACHINEGUN)
			return WEAPON_TYPE::WEAPON_TYPE_MACHINE_GUN;
		else if (nType & WEAPON_TYPE_OF_BAZOOKA)
			return WEAPON_TYPE::WEAPON_TYPE_BAZOOKA;
	}

	return WEAPON_TYPE::WEAPON_TYPE_BEAM_RIFLE;
}

void CPlayer::AddWeapon(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, CModel *pWeaponModel, int nType, void *pContext)
{
	CWeapon *pWeapon = NULL;

	switch (nType)
	{
	case WEAPON_TYPE_OF_GIM_GUN:
		pWeapon = new CGimGun();
		if (pContext) ((CGimGun*)pWeapon)->SetBullet((CShader*)pContext);
		break;
	case WEAPON_TYPE_OF_BAZOOKA:
		pWeapon = new CBazooka();
		if (pContext) ((CBazooka*)pWeapon)->SetBullet((CShader*)pContext);
		break;
	case WEAPON_TYPE_OF_MACHINEGUN:
		pWeapon = new CMachineGun();
		if (pContext) ((CMachineGun*)pWeapon)->SetBullet((CShader*)pContext);
		break;
	default:
		exit(1);
		break;
	}

	pWeapon->SetModel(pWeaponModel);
	pWeapon->CreateShaderVariables(pd3dDevice, pd3dCommandList);
	pWeapon->Initialize();
	pWeapon->SetForCreate(pd3dDevice, pd3dCommandList);

	if (!m_pRHWeapon) EquipOnRightHand(pWeapon);

	m_vpWeapon.emplace_back(pWeapon);
}
