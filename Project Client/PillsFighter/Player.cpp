#include "stdafx.h"
#include "Player.h"
#include "Shader.h"
#include "Repository.h"
#include "Scene.h"
#include "Animation.h"
#include "Weapon.h"

extern CFMODSound gFmodSound;

#define CAMERA_POSITION XMFLOAT3(0.0f, 30.0f, -35.0f)

CPlayer::CPlayer(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature, CRepository *pRepository, void *pContext, int nRobotType) : CRobotObject()
{
	m_pCamera = SetCamera(0.0f);

	m_nHitPoint = m_nMaxHitPoint = 100;
	SetPosition(XMFLOAT3(0.0f, 0.0f, 0.0f));

	AddPrepareRotate(0.0f, 180.0f, 0.0f);

	CModel *pModel;

	switch (nRobotType)
	{
	case SELECT_CHARACTER_GM:
		pModel = pRepository->GetModel(pd3dDevice, pd3dCommandList, "./Resource/Robot/GM_Sniper.bin", "./Resource/Animation/gundam_UpperBody.bin", "./Resource/Animation/gundam_LowerBody.bin");
		break;
	case SELECT_CHARACTER_GUNDAM:
		pModel = pRepository->GetModel(pd3dDevice, pd3dCommandList, "./Resource/Robot/Gundam.bin", "./Resource/Animation/gundam_UpperBody.bin", "./Resource/Animation/gundam_LowerBody.bin");
		break;
	case SELECT_CHARACTER_ZAKU:
		pModel = pRepository->GetModel(pd3dDevice, pd3dCommandList, "./Resource/Robot/Zaku.bin", "./Resource/Animation/Zaku_UpperBody.bin", "./Resource/Animation/UnderBody.bin");
		break;
	default:
		pModel = pRepository->GetModel(pd3dDevice, pd3dCommandList, "./Resource/Robot/GM.bin", "./Resource/Animation/UpperBody.bin", "./Resource/Animation/UnderBody.bin");
		break;
	}

	SetModel(pModel);
	
	CreateShaderVariables(pd3dDevice, pd3dCommandList);

	CHeightMapTerrain *pTerrain = (CHeightMapTerrain *)pContext;

	SetPlayerUpdatedContext(pTerrain);
	SetCameraUpdatedContext(pTerrain);

	CPlayerShader *pPlayerShader = new CPlayerShader();
	pPlayerShader->CreateShader(pd3dDevice, pd3dGraphicsRootSignature);
	SetShader(pPlayerShader);

	CPlayerWeaponShader *pShader = new CPlayerWeaponShader();
	pShader->CreateShader(pd3dDevice, pd3dGraphicsRootSignature);
	SetWeaponShader(pShader);

	SetCallBackKeys(m_pModel);
}

CPlayer::~CPlayer()
{
	ReleaseShaderVariables();

	if (m_pCamera)
	{
		m_pCamera->ReleaseShaderVariables();
		delete m_pCamera;
		m_pCamera = NULL;
	}

	for (CWeapon *pWeapon : m_vpWeapon)
	{
		pWeapon->ReleaseShaderVariables();
		delete pWeapon;
		pWeapon = NULL;
	}
	m_vpWeapon.clear();

	if (m_pWeaponShader)
	{
		m_pWeaponShader->ReleaseObjects();
		m_pWeaponShader->ReleaseShaderVariables();
		delete m_pWeaponShader;
		m_pWeaponShader = NULL;
	}
}

bool CPlayer::IsPrepareDashAnimation()
{ 
	return AnimationIs(ANIMATION_DOWN, ANIMATION_STATE_DASH_BACKWARD) || AnimationIs(ANIMATION_DOWN, ANIMATION_STATE_DASH_FORWARD) ||
		AnimationIs(ANIMATION_DOWN, ANIMATION_STATE_DASH_LEFT) || AnimationIs(ANIMATION_DOWN, ANIMATION_STATE_DASH_RIGHT);
}

void CPlayer::CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList)
{
	if (m_pCamera) m_pCamera->CreateShaderVariables(pd3dDevice, pd3dCommandList);

	CGameObject::CreateShaderVariables(pd3dDevice, pd3dCommandList);
}

void CPlayer::ReleaseShaderVariables()
{
	if (m_pCamera) m_pCamera->ReleaseShaderVariables();

	CRobotObject::ReleaseShaderVariables();
}

void CPlayer::ReleaseUploadBuffers()
{

	CGameObject::ReleaseUploadBuffers();
}

CCamera *CPlayer::SetCamera(float fTimeElapsed)
{
	m_pCamera = new CCamera();

	m_xmf3Right = m_pCamera->GetRightVector();
	m_xmf3Up = m_pCamera->GetUpVector();
	m_xmf3Look = m_pCamera->GetLookVector();

	m_pCamera->SetPlayer(this);

	m_pCamera->SetOffset(CAMERA_POSITION);
	m_pCamera->GenerateProjectionMatrix(1.01f, 5000.0f, ASPECT_RATIO, 60.0f);
	m_pCamera->SetViewPort(0.0f, 0.0f, float(FRAME_BUFFER_WIDTH), float(FRAME_BUFFER_HEIGHT));
	m_pCamera->SetScissorRect(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT);

	return(m_pCamera);
}

void CPlayer::SetHitPoint(int nHitPoint)
{
	int before = m_nHitPoint;
	CGameObject::SetHitPoint(nHitPoint);
	int after = m_nHitPoint;

	if (before > after) m_pCamera->SetShake();
}

void CPlayer::Move(ULONG dwDirection, float fDistance)
{
	if (m_bDie) return;

	XMFLOAT3 xmf3Shift = XMFLOAT3(0, 0, 0);
	
	if (IsDash())
	{
		float fDashSpeed = fDistance;

		if (IsPrepareDashAnimation()) fDashSpeed *= 1.5f;
		else fDashSpeed *= 2.5f;

		DashMove(dwDirection, fDashSpeed);
	}
	else
	{
		if (dwDirection == 0) return;

		if (IsDoingAttack() && IsOnGround()) fDistance *= 0.5f;

		float fSpeed = fDistance;

		float fKeyDownCount = 0.0f;

		if (dwDirection & DIR_FORWARD) fKeyDownCount +=1.0f;
		if (dwDirection & DIR_BACKWARD) fKeyDownCount +=2.0f;
		if (dwDirection & DIR_RIGHT) fKeyDownCount +=1.5f;
		if (dwDirection & DIR_LEFT) fKeyDownCount +=1.5f;

		fSpeed /= fKeyDownCount;

		if (dwDirection & DIR_FORWARD)
		{
			if (IsOnGround())
			{
				if (!IsBoostering() && !IsDash())
				{
					ChangeAnimation(ANIMATION_DOWN, 0, ANIMATION_STATE_WALK_FORWARD);

					if (!IsDoingAttack())
						ChangeAnimation(ANIMATION_UP, 0, ANIMATION_STATE_WALK_FORWARD);
				}
			}

			xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Look, fSpeed);
		}
		if (dwDirection & DIR_BACKWARD)
		{
			if (IsOnGround())
			{
				if (!IsBoostering() && !IsDash())
				{
					ChangeAnimation(ANIMATION_DOWN, 0, ANIMATION_STATE_WALK_BACKWARD);

					if (!IsDoingAttack())
						ChangeAnimation(ANIMATION_UP, 0, ANIMATION_STATE_WALK_BACKWARD);
				}
			}

			xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Look, -fSpeed);
		}
		if (dwDirection & DIR_RIGHT)
		{
			if (IsOnGround())
			{
				if (!IsBoostering() && !IsDash())
				{
					ChangeAnimation(ANIMATION_DOWN, 0, ANIMATION_STATE_WALK_RIGHT);

					if (!IsDoingAttack())
						ChangeAnimation(ANIMATION_UP, 0, ANIMATION_STATE_WALK_RIGHT);
				}
			}

			xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Right, fSpeed);
		}
		if (dwDirection & DIR_LEFT)
		{
			if (IsOnGround())
			{
				if (!IsBoostering() && !IsDash())
				{
					ChangeAnimation(ANIMATION_DOWN, 0, ANIMATION_STATE_WALK_LEFT);

					if (!IsDoingAttack())
						ChangeAnimation(ANIMATION_UP, 0, ANIMATION_STATE_WALK_LEFT);
				}
			}

			xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Right, -fSpeed);
		}


		m_nState |= OBJECT_STATE_WALK;

		Move(xmf3Shift);
	}
}

void CPlayer::DashMove(ULONG dwDirection, float fDistance)
{
	XMFLOAT3 xmf3Shift = XMFLOAT3(0, 0, 0);
	float fSpeed = fDistance;

	if (m_nDashDirection == DIR_FORWARD)
	{
		xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Look, fSpeed);

		if (dwDirection & DIR_BACKWARD)
		{
			fSpeed = fDistance * 0.5f;

			xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Look, -fSpeed);
		}
		if (dwDirection & DIR_RIGHT)
		{
			fSpeed = fDistance * 0.25f;

			xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Right, fSpeed);
		}
		if (dwDirection & DIR_LEFT)
		{
			fSpeed = fDistance * 0.25f;

			xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Right, -fSpeed);
		}
	}
	else if (m_nDashDirection == DIR_BACKWARD)
	{
		xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Look, -fSpeed);

		if (dwDirection & DIR_FORWARD)
		{
			fSpeed = fDistance * 0.5f;
			xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Look, fSpeed);
		}
		if (dwDirection & DIR_RIGHT)
		{
			fSpeed = fDistance * 0.25f;
			xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Right, fSpeed);
		}
		if (dwDirection & DIR_LEFT)
		{
			fSpeed = fDistance * 0.25f;
			xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Right, -fSpeed);
		}
	}
	else if (m_nDashDirection == DIR_RIGHT)
	{
		xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Right, fSpeed);

		if (dwDirection & DIR_FORWARD)
		{
			fSpeed = fDistance * 0.25f;
			xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Look, fSpeed);
		}
		if (dwDirection & DIR_BACKWARD)
		{
			fSpeed = fDistance * 0.25f;

			xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Look, -fSpeed);
		}
		if (dwDirection & DIR_LEFT)
		{
			fSpeed = fDistance * 0.5f;
			xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Right, -fSpeed);
		}
	}
	else if (m_nDashDirection == DIR_LEFT)
	{
		xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Right, -fSpeed);

		if (dwDirection & DIR_FORWARD)
		{
			fSpeed = fDistance * 0.25f;
			xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Look, fSpeed);
		}
		if (dwDirection & DIR_BACKWARD)
		{
			fSpeed = fDistance * 0.25f;

			xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Look, -fSpeed);
		}
		if (dwDirection & DIR_RIGHT)
		{
			fSpeed = fDistance * 0.5f;
			xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Right, fSpeed);
		}
	}

	Move(xmf3Shift);
}

void CPlayer::Move(const XMFLOAT3& xmf3Shift)
{
	m_xmf3Position = Vector3::Add(m_xmf3Position, xmf3Shift);
}

void CPlayer::Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera, bool bSetTexture, bool bSetShader, int nInstances)
{
	if (m_bDie) return;
	if (m_bZoomIn) return;

	CRobotObject::Render(pd3dCommandList, pCamera, bSetTexture, bSetShader, nInstances);

	if (m_pWeaponShader) m_pWeaponShader->Render(pd3dCommandList, pCamera);

	if (m_pRHWeapon)
	{
		m_pRHWeapon->SetOwnerTransform(m_pRightHand->GetWorldTransf());
		m_pRHWeapon->Render(pd3dCommandList, pCamera, true, false, 1);
	}

	if (m_pLHWeapon)
	{
		m_pLHWeapon->SetOwnerTransform(m_pLHWeapon->GetWorldTransf());
		m_pLHWeapon->Render(pd3dCommandList, pCamera, true, false, 1);
	}
}

void CPlayer::RenderToShadow(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera, bool bSetTexture, bool bSetShader, int nInstances)
{
	if (m_bDie) return;

	CRobotObject::RenderToShadow(pd3dCommandList, pCamera, bSetTexture, bSetShader, nInstances);

	if (m_pWeaponShader) m_pWeaponShader->RenderToShadow(pd3dCommandList, pCamera);

	if (m_pRHWeapon) m_pRHWeapon->RenderToShadow(pd3dCommandList, pCamera, true, false, 1);
	if (m_pLHWeapon) m_pLHWeapon->RenderToShadow(pd3dCommandList, pCamera, true, false, 1);
}

void CPlayer::RenderWire(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera, int nInstances)
{
	if (m_bDie) return;

	CRobotObject::RenderWire(pd3dCommandList, pCamera, nInstances);

	if (m_pRHWeapon) m_pRHWeapon->RenderWire(pd3dCommandList, pCamera);
	if (m_pLHWeapon) m_pLHWeapon->RenderWire(pd3dCommandList, pCamera);
}

void CPlayer::Update(float fTimeElapsed)
{	
	if (!m_bDie)
	{
		for (const auto& Weapon : m_vpWeapon) Weapon->Animate(fTimeElapsed, NULL);

		ProcessBooster(fTimeElapsed);
		ProcessHitPoint();
		ProcessTime(m_pRHWeapon, fTimeElapsed);
		ProcessAnimation();

		if (!IsZero(m_fVelocityY))
		{
			if (m_xmf3Position.y + m_fVelocityY < 1700.0f)
				Move(XMFLOAT3(0.0f, m_fVelocityY, 0.0f));
		}
	}

	CRobotObject::Animate(fTimeElapsed);

	if (m_pPlayerUpdatedContext) OnPlayerUpdateCallback(fTimeElapsed);
	m_pCamera->Update(fTimeElapsed);
	if (m_pCameraUpdatedContext) OnCameraUpdateCallback(fTimeElapsed);
	XMFLOAT3 xmf3LookAt = Vector3::Add(m_xmf3Position, XMFLOAT3(0.0f, 20.0f, 0.0f));
	m_pCamera->SetLookAt(xmf3LookAt);
	m_pCamera->GenerateViewMatrix();

	m_xmf3PrevPosition = m_xmf3Position;
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

XMFLOAT4X4 CPlayer::GetToTarget(XMFLOAT3 xmf3Position)
{
	float fDistance = m_pScene->GetToTargetDistance();

	XMFLOAT3 xmf3CameraPos = m_pCamera->GetPosition();
	XMFLOAT3 xmf3CameraLook = m_pCamera->GetLookVector();
	XMFLOAT3 xmf3DestPos = XMFLOAT3(xmf3CameraPos.x + xmf3CameraLook.x * fDistance,
		xmf3CameraPos.y + xmf3CameraLook.y * fDistance,
		xmf3CameraPos.z + xmf3CameraLook.z * fDistance);

	XMFLOAT3 xmf3Right = m_pCamera->GetRightVector();
	XMFLOAT3 xmf3Look = Vector3::Normalize(Vector3::Subtract(xmf3DestPos, xmf3Position));
	XMFLOAT3 xmf3Up = Vector3::CrossProduct(xmf3Right, xmf3Look, true);
	xmf3Right = Vector3::CrossProduct(xmf3Up, xmf3Look, true);

	return XMFLOAT4X4(xmf3Right.x, xmf3Right.y, xmf3Right.z, 0.0f,
		xmf3Up.x, xmf3Up.y, xmf3Up.z, 0.0f,
		xmf3Look.x, xmf3Look.y, xmf3Look.z, 0.0f,
		xmf3Position.x, xmf3Position.y, xmf3Position.z, 1.0f);
}

XMFLOAT3 CPlayer::GetToTargetPosition(XMFLOAT3 xmf3Position)
{
	float fDistance = m_pScene->GetToTargetDistance();

	XMFLOAT3 xmf3CameraPos = m_pCamera->GetPosition();
	XMFLOAT3 xmf3CameraLook = m_pCamera->GetLookVector();
	XMFLOAT3 xmf3DestPos = XMFLOAT3(xmf3CameraPos.x + xmf3CameraLook.x * fDistance,
		xmf3CameraPos.y + xmf3CameraLook.y * fDistance,
		xmf3CameraPos.z + xmf3CameraLook.z * fDistance);

	return xmf3DestPos;
}

void CPlayer::OnPlayerUpdateCallback(float fTimeElapsed)
{
	//if(m_pScene) m_pScene->CheckCollisionPlayer();

	CHeightMapTerrain *pTerrain = (CHeightMapTerrain *)m_pPlayerUpdatedContext;
	if (pTerrain == NULL)
	{
		m_nState &= ~OBJECT_STATE_ONGROUND;
		return;
	}

	XMFLOAT3 xmf3Scale = pTerrain->GetScale();
	XMFLOAT3 xmf3PlayerPosition = GetPosition();
	int z = (int)(xmf3PlayerPosition.z / xmf3Scale.z);
	bool bReverseQuad = ((z % 2) != 0);
	float fHeight = pTerrain->GetHeight(xmf3PlayerPosition.x, xmf3PlayerPosition.z, bReverseQuad);

	if (xmf3PlayerPosition.y <= fHeight)
	{
		float fPlayerVelocity = GetVelocity();
		fPlayerVelocity = 0.0f;
		SetVelocity(fPlayerVelocity);
		xmf3PlayerPosition.y = fHeight;

		SetPosition(xmf3PlayerPosition);

		m_nState |= OBJECT_STATE_ONGROUND;
	}
	else
	{
		m_nState &= ~OBJECT_STATE_ONGROUND;
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

void CPlayer::ActivationBooster(UINT nType)
{
	if (IsBoostering()) return;
	if (m_bDie) return;

	// Active by Space Bar
	if (m_nBoosterGauge > 0)
	{
		if (IsOnGround() && !IsJumping())
		{
			ChangeAnimation(ANIMATION_DOWN, 0, ANIMATION_STATE_JUMP, true);

			if (!IsDoingAttack())
				ChangeAnimation(ANIMATION_UP, 0, ANIMATION_STATE_JUMP, true);

			m_nState |= OBJECT_STATE_JUMP;

			m_fVelocityY = 0.0f;
		}

		m_nState |= OBJECT_STATE_BOOSTER;

		m_fTimeForChargeBoosterGauge = 0.0f;

		if(nType == BOOSTER_TYPE_UP)
			m_bSpaceDown = true;
		else if(nType == BOOSTER_TYPE_DOWN)
			m_bVDown = true;
	}
}

void CPlayer::ActivationDash()
{
	if (IsBoostering()) return;
	if (m_bDie) return;

	static UCHAR pKeyBuffer[256];

	if (::GetKeyboardState(pKeyBuffer))
	{
		if (pKeyBuffer['W'] & 0xF0) 
			m_nDashDirection = DIR_FORWARD;
		else if (pKeyBuffer['S'] & 0xF0) 
			m_nDashDirection = DIR_BACKWARD;
		else if (pKeyBuffer['A'] & 0xF0) 
			m_nDashDirection = DIR_LEFT;
		else if (pKeyBuffer['D'] & 0xF0) 
			m_nDashDirection = DIR_RIGHT;
	}

	if (m_nDashDirection == 0) return;

	if (m_nBoosterGauge > 0)
	{
		if (m_nDashDirection & DIR_FORWARD)
		{
			m_nDashDirection = DIR_FORWARD;

			ChangeAnimation(ANIMATION_DOWN, 0, ANIMATION_STATE_DASH_FORWARD, true);

			if (!IsDoingAttack())
				ChangeAnimation(ANIMATION_UP, 0, ANIMATION_STATE_DASH_FORWARD, true);
		}
		else if (m_nDashDirection & DIR_BACKWARD)
		{
			m_nDashDirection = DIR_BACKWARD;

			ChangeAnimation(ANIMATION_DOWN, 0, ANIMATION_STATE_DASH_BACKWARD, true);

			if (!IsDoingAttack())
				ChangeAnimation(ANIMATION_UP, 0, ANIMATION_STATE_DASH_BACKWARD, true);
		}
		else if (m_nDashDirection & DIR_LEFT)
		{
			m_nDashDirection = DIR_LEFT;

			ChangeAnimation(ANIMATION_DOWN, 0, ANIMATION_STATE_DASH_LEFT, true);

			if (!IsDoingAttack())
				ChangeAnimation(ANIMATION_UP, 0, ANIMATION_STATE_DASH_LEFT, true);
		}
		else if (m_nDashDirection & DIR_RIGHT)
		{
			m_nDashDirection = DIR_RIGHT;

			ChangeAnimation(ANIMATION_DOWN, 0, ANIMATION_STATE_DASH_RIGHT, true);

			if (!IsDoingAttack())
				ChangeAnimation(ANIMATION_UP, 0, ANIMATION_STATE_DASH_RIGHT, true);
		}

		m_nState |= OBJECT_STATE_BOOSTER;
		m_nState |= OBJECT_STATE_DASH;
		m_bShiftDown = true;

		m_fTimeForChargeBoosterGauge = 0.0f;
	}
}

void CPlayer::ProcessBooster(float fTimeElapsed)
{
	if (m_fTimeForBoostUp > 0.0f)
	{
		float fUpPower = 0.0f;

		fUpPower = m_fTimeForBoostUp * BOOSTER_POWER;

		m_fVelocityY += fUpPower;

		if (m_fVelocityY > MAX_UP_POWER) m_fVelocityY = MAX_UP_POWER;
	}
	if (m_fTimeForBoostDown > 0.0f)
	{
		float fDownPower = 0.0f;

		fDownPower = m_fTimeForBoostDown * BOOSTER_POWER;

		m_fVelocityY -= fDownPower;

		if (m_fVelocityY < MAX_DOWN_POWER) m_fVelocityY = MAX_DOWN_POWER;
	}

	if (m_bSpaceDown && IsBoostering())
		m_fTimeForBoostUp += fTimeElapsed;
	else
	{
		if (m_fTimeForBoostUp > 0.0f)
		{
			m_fTimeForBoostUp *= 0.5f;
			m_fTimeForBoostUp = m_fTimeForBoostUp - fTimeElapsed < 0.0f ? 0.0f : m_fTimeForBoostUp - fTimeElapsed;
		}
	}

	if (m_bVDown && IsBoostering())
		m_fTimeForBoostDown += fTimeElapsed;
	else
	{
		if (m_fTimeForBoostDown > 0.0f)
		{
			m_fTimeForBoostDown *= 0.5f;
			m_fTimeForBoostDown = m_fTimeForBoostDown - fTimeElapsed < 0.0f ? 0.0f : m_fTimeForBoostDown - fTimeElapsed;
		}
	}

	if(IsBoostering())
		ProcessBoosterConsume(fTimeElapsed);
	else
		ProcessBoosterCharge(fTimeElapsed);
}

void CPlayer::ProcessBoosterCharge(float fElapsedTime)
{
	if ((m_fTimeForBoostUp <= 0.0f) && (m_fTimeForBoostDown <= 0.0f))
	{
		if (m_nBoosterGauge < 100)
		{
			if (m_fTimeForChargeBoosterGauge >= INTERVAL_BOOSTER_GAUGE_CHARGE)
			{
				m_fTimeForChargeBoosterGauge = fmod(m_fTimeForChargeBoosterGauge, INTERVAL_BOOSTER_GAUGE_CHARGE);

				m_nBoosterGauge = m_nBoosterGauge + BOOSTER_GAUGE_CHARGE_VALUE > 100 ? 100 : m_nBoosterGauge + BOOSTER_GAUGE_CHARGE_VALUE;
			}
			else
			{
				m_fTimeForChargeBoosterGauge += fElapsedTime;
			}
		}
	}
}

void CPlayer::ProcessBoosterConsume(float fElapsedTime)
{
	if (m_nBoosterGauge > 0)
	{
		if (m_fTimeForConsumeBoosterGauge >= INTERVAL_BOOSTER_GAUGE_CONSUME)
		{
			int nCousumCount = static_cast<int>(m_fTimeForConsumeBoosterGauge / INTERVAL_BOOSTER_GAUGE_CONSUME);
			int nConsumeVal = nCousumCount * BOOSTER_GAUGE_CONSUME_VALUE;

			m_fTimeForConsumeBoosterGauge = fmod(m_fTimeForConsumeBoosterGauge, INTERVAL_BOOSTER_GAUGE_CONSUME);

			m_nBoosterGauge = m_nBoosterGauge - nConsumeVal < 0 ? 0 : m_nBoosterGauge - nConsumeVal;
		}
		else
		{
			m_fTimeForConsumeBoosterGauge += fElapsedTime;
		}
	}
	else
	{
		if (IsDash())
			DeactivationDash();
		else
			DeactivationBooster();
	}
}

void CPlayer::ApplyGravity(float fGravity, float fTimeElapsed)
{
	float fGravAcc = fGravity * m_fMass * fTimeElapsed;

	if (!IsBoostering() || IsDash())
	{
		if (IsZero(fGravity))
			m_fVelocityY *= 0.5f;
		else
			m_fVelocityY += fGravAcc;
	}
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

void CPlayer::ProcessMouseUpTime(float fTimeElapsed)
{
	if (!m_LButtonDown)
	{
		m_fMouseUpTime += fTimeElapsed;
		
		if (m_fMouseUpTime > 1.0f)
		{
			m_nSaberAnimationIndex = 0;
		}
	}
}

void CPlayer::ProcessAnimation()
{
	if (IsShooting())
		ProcessShootAnimation();

	if (IsSwording())
	{
		if (IsAnimationEnd(ANIMATION_UP, 0))
		{
			if (m_LButtonDown)
			{
				ChangeAnimation(ANIMATION_UP, 0, m_nAnimationList[m_nSaberAnimationIndex], true);

				m_nSaberAnimationIndex = (m_nSaberAnimationIndex + 1) % 3;
			}
			else
			{
				m_nState &= ~OBJECT_STATE_SWORDING;
			}
		}
	}

	if (IsDash()) 
		ProcessDashAnimation();

	if (IsBoostering() && !IsDash())
		ProcessJumpAnimation();

	if (!IsOnGround() && !IsDash())
	{
		if (!IsJumping())
		{
			ChangeAnimation(ANIMATION_DOWN, 0, ANIMATION_STATE_JUMP_LOOP);

			if (!IsDoingAttack())
				ChangeAnimation(ANIMATION_UP, 0, ANIMATION_STATE_JUMP_LOOP);
		}

		if (AnimationIs(ANIMATION_DOWN, ANIMATION_STATE_JUMP) && IsAnimationEnd(ANIMATION_DOWN, 0))
		{
			ChangeAnimation(ANIMATION_DOWN, 0, ANIMATION_STATE_JUMP_LOOP);

			if (!IsDoingAttack())
				ChangeAnimation(ANIMATION_UP, 0, ANIMATION_STATE_JUMP_LOOP);

			m_nState &= ~OBJECT_STATE_JUMP;
		}
	}

	// 땅 위에서 가만히 있을 때 Idle
	if (IsOnGround() && !IsWalking() && !IsJumping() && !IsBoostering() && !IsDash())
	{
		ChangeAnimation(ANIMATION_DOWN, 0, ANIMATION_STATE_IDLE);

		if (!IsDoingAttack())
		{
			ChangeAnimation(ANIMATION_UP, 0, ANIMATION_STATE_IDLE);
		}
	}

	if (IsSameUpDownAnimation())
	{
		// 하반신이 우선인 상태
		if (IsWalking() || IsDash())
		{
			if (!IsDoingAttack())
			{
				m_ppAnimationControllers[ANIMATION_UP]->SetTrackPosition(0, m_ppAnimationControllers[ANIMATION_DOWN]->GetTrackPosition(0));
			}
		}
	}
}

void CPlayer::ProcessShootAnimation()
{
	CGun *pGun = (CGun*)m_pRHWeapon;
	
	if (m_bShootable)
	{
		if (pGun->IsShootable())
		{
			if (IsDash()) ChangeAnimation(ANIMATION_UP, 0, ANIMATION_STATE_SHOOT_DASH_ONCE, true);
			else ChangeAnimation(ANIMATION_UP, 0, ANIMATION_STATE_SHOOT_ONCE, true);

			pGun->Shot();
			ChangeUIAmmo();

			m_bShootable = false;
		}
	}

	// Start 자세
	if (AnimationIsShootStart())
	{
		if (IsUnderBodyChangeable()) ChangeAnimation(ANIMATION_DOWN, 0, ANIMATION_STATE_GM_GUN_SHOOT_START);

		if (IsAnimationEnd(ANIMATION_UP, 0))
		{
			if (pGun->IsCoolDown() || !pGun->IsShootable())
			{
				if (m_LButtonDown)
				{
					m_bShootable = true;
				}
				else
				{
					if (m_bAim)
					{
						if (!m_bZoomIn && !m_RButtonDown)
						{
							ZoomIn();
						}
					}

					if (!m_RButtonDown && !m_bZoomIn)
					{
						if (IsDash()) ChangeAnimation(ANIMATION_UP, 0, ANIMATION_STATE_DASH_SHOOT_RETURN_ONCE, true);
						else ChangeAnimation(ANIMATION_UP, 0, ANIMATION_STATE_GM_GUN_SHOOT_RETURN, true);

						if (IsUnderBodyChangeable())
						{
							ChangeAnimation(ANIMATION_DOWN, 0, ANIMATION_STATE_GM_GUN_SHOOT_RETURN, true);
						}
					}

					m_bAim = false;
					m_bShootable = false;
				}
			}
			else
			{
				m_bShootable = true;
			}
		}
	}

	// Return 자세
	if (AnimationIsShootReturn())
	{
		if (IsUnderBodyChangeable()) ChangeAnimation(ANIMATION_DOWN, 0, ANIMATION_STATE_GM_GUN_SHOOT_RETURN);

		if (IsAnimationEnd(ANIMATION_UP, 0))
		{
			m_nState &= ~OBJECT_STATE_SHOOTING;

			m_bShootable = false;
		}
	}

	if (AnimationIsShootOnce())
	{
		if (IsAnimationEnd(ANIMATION_UP, 0))
		{
			if (IsDash()) ChangeAnimation(ANIMATION_UP, 0, ANIMATION_STATE_DASH_SHOOT_START_ONCE, true);
			else ChangeAnimation(ANIMATION_UP, 0, ANIMATION_STATE_GM_GUN_SHOOT_START, true);

			m_ppAnimationControllers[ANIMATION_UP]->SetTrackPosition(0, m_ppAnimationControllers[ANIMATION_UP]->GetTrackLength(0));
		}
	}
}

void CPlayer::ProcessDashAnimation()
{
	if (IsAnimationEnd(ANIMATION_DOWN, 0))
	{
		m_bChangedDashStart = true;

		if (AnimationIs(ANIMATION_DOWN, ANIMATION_STATE_DASH_FORWARD))
		{
			ChangeAnimation(ANIMATION_DOWN, 0, ANIMATION_STATE_DASH_FORWARD_LOOP);

			if (!IsDoingAttack())
				ChangeAnimation(ANIMATION_UP, 0, ANIMATION_STATE_DASH_FORWARD_LOOP);
		}
		else if (AnimationIs(ANIMATION_DOWN, ANIMATION_STATE_DASH_BACKWARD))
		{
			ChangeAnimation(ANIMATION_DOWN, 0, ANIMATION_STATE_DASH_BACKWARD_LOOP);

			if (!IsDoingAttack())
				ChangeAnimation(ANIMATION_UP, 0, ANIMATION_STATE_DASH_BACKWARD_LOOP);
		}
		else if (AnimationIs(ANIMATION_DOWN, ANIMATION_STATE_DASH_LEFT))
		{
			ChangeAnimation(ANIMATION_DOWN, 0, ANIMATION_STATE_DASH_LEFT_LOOP);

			if (!IsDoingAttack())
				ChangeAnimation(ANIMATION_UP, 0, ANIMATION_STATE_DASH_LEFT_LOOP);
		}
		else if (AnimationIs(ANIMATION_DOWN, ANIMATION_STATE_DASH_RIGHT))
		{
			ChangeAnimation(ANIMATION_DOWN, 0, ANIMATION_STATE_DASH_RIGHT_LOOP);

			if (!IsDoingAttack())
				ChangeAnimation(ANIMATION_UP, 0, ANIMATION_STATE_DASH_RIGHT_LOOP);
		}
	}

	if (m_bChangedDashStart)
	{
		if (m_bShiftDown)
		{
			if (AnimationIs(ANIMATION_DOWN, ANIMATION_STATE_DASH_FORWARD_LOOP))
			{
				if (!IsDoingAttack())
					ChangeAnimation(ANIMATION_UP, 0, ANIMATION_STATE_DASH_FORWARD_LOOP);
			}
			else if (AnimationIs(ANIMATION_DOWN, ANIMATION_STATE_DASH_BACKWARD_LOOP))
			{
				if (!IsDoingAttack())
					ChangeAnimation(ANIMATION_UP, 0, ANIMATION_STATE_DASH_BACKWARD_LOOP);
			}
			else if (AnimationIs(ANIMATION_DOWN, ANIMATION_STATE_DASH_LEFT_LOOP))
			{
				if (!IsDoingAttack())
					ChangeAnimation(ANIMATION_UP, 0, ANIMATION_STATE_DASH_LEFT_LOOP);
			}
			else if (AnimationIs(ANIMATION_DOWN, ANIMATION_STATE_DASH_RIGHT_LOOP))
			{
				if (!IsDoingAttack())
					ChangeAnimation(ANIMATION_UP, 0, ANIMATION_STATE_DASH_RIGHT_LOOP);
			}
		}
		else
		{
			DeactivationDash();
		}
	}
}

void CPlayer::ProcessJumpAnimation()
{
	if (IsAnimationEnd(ANIMATION_DOWN, 0))
	{
		ChangeAnimation(ANIMATION_DOWN, 0, ANIMATION_STATE_JUMP_LOOP);

		if (!IsDoingAttack())
			ChangeAnimation(ANIMATION_UP, 0, ANIMATION_STATE_JUMP_LOOP);

		m_bChangedSpaceStart = true;

		m_nState &= ~OBJECT_STATE_JUMP;
	}

	if (m_bChangedSpaceStart)
	{
		if (m_bSpaceDown || m_bVDown)
		{
			if (AnimationIs(ANIMATION_DOWN, ANIMATION_STATE_JUMP_LOOP))
			{
				if (!IsDoingAttack())
					ChangeAnimation(ANIMATION_UP, 0, ANIMATION_STATE_JUMP_LOOP);
			}
		}
		else
		{
			DeactivationBooster();
		}
	}

	if (AnimationIs(ANIMATION_DOWN, ANIMATION_STATE_JUMP_LOOP))
	{
		if (!m_bSpaceDown && !m_bVDown)
		{
			DeactivationBooster();
		}
	}
}

void CPlayer::Attack(CWeapon *pWeapon)
{
	if (m_bDie) return;

	m_LButtonDown = true;

	if (IsDoingAttack()) return;

	if (pWeapon)
	{
		int nType = pWeapon->GetType();

		if (nType & WEAPON_TYPE_OF_GUN)
		{
			CGun *pGun = (CGun*)pWeapon;

			if (!m_bReloading)
			{
				pGun->PrepareShot();
				m_nState |= OBJECT_STATE_SHOOTING;

				if (IsDash()) ChangeAnimation(ANIMATION_UP, 0, ANIMATION_STATE_DASH_SHOOT_START_ONCE, true);
				else ChangeAnimation(ANIMATION_UP, 0, ANIMATION_STATE_GM_GUN_SHOOT_START, true);

				if (IsUnderBodyChangeable())
					ChangeAnimation(ANIMATION_DOWN, 0, ANIMATION_STATE_GM_GUN_SHOOT_START, true);
			}
		}
		else if (nType & WEAPON_TYPE_OF_SWORD)
		{
			m_nState |= OBJECT_STATE_SWORDING;

			ChangeAnimation(ANIMATION_UP, 0, m_nAnimationList[m_nSaberAnimationIndex], true);

			m_nSaberAnimationIndex = (m_nSaberAnimationIndex + 1) % 3;
		}
	}
}

void CPlayer::PickUpAmmo(int nType, int nAmmo)
{
	if (nType & WEAPON_TYPE_OF_GM_GUN) m_nGmGunAmmo += nAmmo;
	else if (nType & WEAPON_TYPE_OF_BAZOOKA) m_nBazookaAmmo += nAmmo;
	else if (nType & WEAPON_TYPE_OF_MACHINEGUN) m_nMachineGunAmmo += nAmmo;

	if(m_pUI) m_pUI->ChangeAmmoText(m_nEquipWeaponIndex);
}

void CPlayer::PrepareAttack(CWeapon *pWeapon)
{
	if (m_bDie) return;

	if (pWeapon)
	{
		int nType = pWeapon->GetType();

		if (nType & WEAPON_TYPE_OF_GUN)
		{
			CGun *pGun = (CGun*)pWeapon;

			if (pGun->GetReloadedAmmo() == 0)
			{
				if (nType & WEAPON_TYPE_OF_GM_GUN)
				{
					if (m_nGmGunAmmo > 0) Reload(pGun);
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
	if (m_bDie) return;

	if (pWeapon)
	{
		if (pWeapon->GetType() & WEAPON_TYPE_OF_BEAM_GUN) return;
		if (pWeapon->GetType() & WEAPON_TYPE_OF_SWORD) return;

		CGun *pGun = (CGun*)pWeapon;

		if (pGun->GetReloadedAmmo() < pGun->GetMaxReloadAmmo())
		{
			if (!m_bReloading)
			{
				int nType = pGun->GetType();
			
				if (nType & WEAPON_TYPE_OF_BAZOOKA)
				{
					if (m_nBazookaAmmo == 0) return;
				}
				else if (nType & WEAPON_TYPE_OF_GM_GUN)
				{
					if (m_nGmGunAmmo == 0) return;
				}
				else if (nType & WEAPON_TYPE_OF_MACHINEGUN)
				{
					if (m_nMachineGunAmmo == 0) return;
				}

				m_bReloading = true;
				m_fReloadTime = pGun->GetReloadTime();
			}
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
					if (nType & WEAPON_TYPE_OF_GM_GUN)
					{
						if (m_nGmGunAmmo > 0)
						{
							pGun->Reload(m_nGmGunAmmo);
							ChangeUIAmmo();
						}
					}
					else if (nType & WEAPON_TYPE_OF_BAZOOKA)
					{
						if (m_nBazookaAmmo > 0)
						{
							pGun->Reload(m_nBazookaAmmo);
							ChangeUIAmmo();
						}
					}
					else if (nType & WEAPON_TYPE_OF_MACHINEGUN)
					{
						if (m_nMachineGunAmmo > 0)
						{
							pGun->Reload(m_nMachineGunAmmo);
							ChangeUIAmmo();
						}
					}

					m_bReloading = false;
				}
			}
		}
	}

	ProcessMouseUpTime(fTimeElapsed);
}

void CPlayer::ChangeUIAmmo()
{
	if (m_pUI) 
		m_pUI->ChangeAmmoText(m_nEquipWeaponIndex); 
}

void CPlayer::ChangeWeapon(int nIndex)
{
	if (IsShooting()) return;
	if (IsSwording()) return;
	if (m_bDie) return;

	CRobotObject::ChangeWeapon(nIndex);

	m_bReloading = false;
	m_bWeaponChanged = TRUE; 
	m_nEquipWeaponIndex = nIndex;
	if (m_pUI)
	{
		m_pUI->ChangeWeapon(nIndex);

		m_pUI->ChangeAmmoText(nIndex);
	}
}

WEAPON_TYPE CPlayer::GetWeaponType()
{
	if (m_pRHWeapon)
	{
		int nType = m_pRHWeapon->GetType();

		if (nType & WEAPON_TYPE_OF_GM_GUN)
			return WEAPON_TYPE::WEAPON_TYPE_GM_GUN;
		else if (nType & WEAPON_TYPE_OF_MACHINEGUN)
			return WEAPON_TYPE::WEAPON_TYPE_MACHINE_GUN;
		else if (nType & WEAPON_TYPE_OF_BAZOOKA)
			return WEAPON_TYPE::WEAPON_TYPE_BAZOOKA;
		else if (nType & WEAPON_TYPE_OF_SABER)
			return WEAPON_TYPE::WEAPON_TYPE_SABER;
		else if (nType & WEAPON_TYPE_OF_TOMAHAWK)
			return WEAPON_TYPE::WEAPON_TYPE_TOMAHAWK;
		else if (nType & WEAPON_TYPE_OF_BEAM_RIFLE)
			return WEAPON_TYPE::WEAPON_TYPE_BEAM_RIFLE;
		else if (nType & WEAPON_TYPE_OF_BEAM_SNIPER)
			return WEAPON_TYPE::WEAPON_TYPE_BEAM_SNIPER;
	}

	return WEAPON_TYPE::WEAPON_TYPE_MACHINE_GUN;
}

void CPlayer::AddWeapon(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, CWeapon *pWeapon)
{
	pWeapon->Initialize();
	pWeapon->SetForCreate(pd3dDevice, pd3dCommandList);
	pWeapon->AddPrepareRotate(180.0f, 90.0f, -90.0f);

	if (!m_pRHWeapon) EquipOnRightHand(pWeapon);

	m_vpWeapon.emplace_back(pWeapon);
}

void CPlayer::GenerateViewMatrix()
{
	m_xmf3Look = Vector3::Normalize(m_xmf3Look);

	m_xmf3Right = Vector3::CrossProduct(m_xmf3Up, m_xmf3Look, true);

	m_xmf3Up = Vector3::CrossProduct(m_xmf3Look, m_xmf3Right, true);

	m_xmf4x4View._11 = m_xmf3Right.x; m_xmf4x4View._12 = m_xmf3Up.x; m_xmf4x4View._13 = m_xmf3Look.x;
	m_xmf4x4View._21 = m_xmf3Right.y; m_xmf4x4View._22 = m_xmf3Up.y; m_xmf4x4View._23 = m_xmf3Look.y;
	m_xmf4x4View._31 = m_xmf3Right.z; m_xmf4x4View._32 = m_xmf3Up.z; m_xmf4x4View._33 = m_xmf3Look.z;
	m_xmf4x4View._41 = -Vector3::DotProduct(m_xmf3Position, m_xmf3Right);
	m_xmf4x4View._42 = -Vector3::DotProduct(m_xmf3Position, m_xmf3Up);
	m_xmf4x4View._43 = -Vector3::DotProduct(m_xmf3Position, m_xmf3Look);
}

void CPlayer::SendShootPacket()
{
	//총알 생성 패킷 보내기
	PKT_SHOOT pktShoot;
	//PKT_ID id = PKT_ID_SHOOT;
	pktShoot.PktId = (char)PKT_ID_SHOOT;
	pktShoot.PktSize = sizeof(PKT_SHOOT);
	pktShoot.ID = gClientIndex;
	pktShoot.Player_Weapon = GetWeaponType();

	CGun *pGun = (CGun*)GetRHWeapon();
	pktShoot.BulletWorldMatrix = GetToTarget(pGun->GetMuzzlePos());

	if (send(gSocket, (char*)&pktShoot, pktShoot.PktSize, 0) == SOCKET_ERROR)
	{
		printf("Send Shoot Error\n");
	}
}

void CPlayer::ZoomIn()
{
	if (!(m_pRHWeapon->GetType() & WEAPON_TYPE_OF_BEAM_SNIPER)) return;

	if (AnimationIsShootStart())
	{
		if (IsAnimationEnd(ANIMATION_UP, 0))
		{
			m_bZoomIn = true;
			m_pUI->ZoomIn();
			m_pCamera->ZoomIn(10);
		}
	}
}

void CPlayer::ZoomOut()
{
	m_bZoomIn = false;
	m_pCamera->ZoomOut();
	m_pUI->ZoomOut();
}

void CPlayer::TakeAim()
{
	if (m_bDie) return;

	m_RButtonDown = true;

	if (!(m_pRHWeapon->GetType() & WEAPON_TYPE_OF_GUN)) return;

	if (m_bZoomIn)
	{
		ZoomOut();
	}
	else
	{
		if (!IsShooting())
		{
			if (!m_bReloading)
			{
				m_nState |= OBJECT_STATE_SHOOTING;
				m_bAim = true;

				if (IsDash()) ChangeAnimation(ANIMATION_UP, 0, ANIMATION_STATE_DASH_SHOOT_START_ONCE, true);
				else ChangeAnimation(ANIMATION_UP, 0, ANIMATION_STATE_GM_GUN_SHOOT_START, true);

				if (IsUnderBodyChangeable())
					ChangeAnimation(ANIMATION_DOWN, 0, ANIMATION_STATE_GM_GUN_SHOOT_START, true);
			}
		}
	}
}

void CPlayer::ProcessDie(float fRespawnTime)
{
	CRobotObject::ProcessDie(fRespawnTime);

	if (m_pUI) m_pUI->ClientDie();

	m_nBoosterGauge = 100;
	m_fVelocityY = 0.0f;
	m_fTimeForConsumeBoosterGauge = 0.0f;
	m_fTimeForChargeBoosterGauge = 0.0f;
	m_fTimeForBoostUp = 0.0f;
	m_fTimeForBoostDown = 0.0f;
	m_bReloading = false;
	m_fReloadTime = 0.0f;
	m_bZoomIn = false;
	m_bAim = false;
	m_nDashDirection = 0;
	m_bShiftDown = false;
	m_bChangedDashStart = false;
	m_bSpaceDown = false;
	m_bChangedSpaceStart = false;
	m_bVDown = false;
	m_bShootable = false;
	m_nSaberAnimationIndex = 0;
	m_LButtonDown = false;
	m_RButtonDown = false;
	m_fMouseUpTime = 0.0f;
	ChangeWeapon(0);

	for (auto& Weapon : m_vpWeapon)
	{
		if (!(Weapon->GetType() & WEAPON_TYPE_OF_GUN)) continue;

		CGun *pGun = (CGun*)Weapon;
		pGun->Reset();
	}
}

void CPlayer::ProcessRespawn(int nHP, XMFLOAT3 xmf3Position)
{
	CRobotObject::ProcessRespawn(nHP, xmf3Position);

	if (m_pUI) m_pUI->ClientRespawn();
}