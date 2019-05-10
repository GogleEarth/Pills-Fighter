#include "stdafx.h"
#include "Player.h"
#include "Shader.h"
#include "Repository.h"
#include "Scene.h"
#include "Animation.h"
#include "Weapon.h"

#define CAMERA_POSITION XMFLOAT3(0.0f, 30.0f, -35.0f)

CPlayer::CPlayer(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature, CRepository *pRepository, void *pContext, int nRobotType) : CRobotObject()
{
	m_pCamera = SetCamera(0.0f);

	m_nHitPoint = m_nMaxHitPoint = 100;
	SetPosition(XMFLOAT3(0.0f, 0.0f, -50.0f));

	AddPrepareRotate(0.0f, 180.0f, 0.0f);

	CModel *pModel;

	switch (nRobotType)
	{
	case SKINNED_OBJECT_INDEX_GM:
		pModel = pRepository->GetModel(pd3dDevice, pd3dCommandList, "./Resource/Robot/GM.bin", "./Resource/Animation/UpperBody.bin", "./Resource/Animation/UnderBody.bin");
		break;
	case SKINNED_OBJECT_INDEX_GUNDAM:
		pModel = pRepository->GetModel(pd3dDevice, pd3dCommandList, "./Resource/Robot/Gundam.bin", "./Resource/Animation/UpperBody.bin", "./Resource/Animation/UnderBody.bin");
		break;
	default:
		pModel = pRepository->GetModel(pd3dDevice, pd3dCommandList, "./Resource/Robot/GM.bin", "./Resource/Animation/UpperBody.bin", "./Resource/Animation/UnderBody.bin");
		break;
	}

	int nAnimationControllers = 2;
	CAnimationController **ppAnimationControllers = new CAnimationController*[nAnimationControllers];
	ppAnimationControllers[ANIMATION_UP] = new CAnimationController(1, pModel->GetAnimationSet(ANIMATION_UP));
	ppAnimationControllers[ANIMATION_DOWN] = new CAnimationController(1, pModel->GetAnimationSet(ANIMATION_DOWN));

	ppAnimationControllers[ANIMATION_UP]->SetTrackAnimation(0, ANIMATION_STATE_IDLE);
	ppAnimationControllers[ANIMATION_DOWN]->SetTrackAnimation(0, ANIMATION_STATE_IDLE);

	CAnimationCallbackHandler *pAnimationCallbackHandler = new CSoundCallbackHandler();

	ppAnimationControllers[ANIMATION_DOWN]->SetCallbackKeys(ANIMATION_STATE_WALK_FORWARD, 2);
	ppAnimationControllers[ANIMATION_DOWN]->SetCallbackKey(ANIMATION_STATE_WALK_FORWARD, 0, 0.1f, (void*)&CALLBACK_TYPE_SOUND_MOVE);
	ppAnimationControllers[ANIMATION_DOWN]->SetCallbackKey(ANIMATION_STATE_WALK_FORWARD, 1, 0.6f, (void*)&CALLBACK_TYPE_SOUND_MOVE);
	ppAnimationControllers[ANIMATION_DOWN]->SetAnimationCallbackHandler(ANIMATION_STATE_WALK_FORWARD, pAnimationCallbackHandler);

	pAnimationCallbackHandler = new CSoundCallbackHandler();
	ppAnimationControllers[ANIMATION_DOWN]->SetCallbackKeys(ANIMATION_STATE_WALK_RIGHT, 2);
	ppAnimationControllers[ANIMATION_DOWN]->SetCallbackKey(ANIMATION_STATE_WALK_RIGHT, 0, 0.2f, (void*)& CALLBACK_TYPE_SOUND_MOVE);
	ppAnimationControllers[ANIMATION_DOWN]->SetCallbackKey(ANIMATION_STATE_WALK_RIGHT, 1, 0.5f, (void*)&CALLBACK_TYPE_SOUND_MOVE);
	ppAnimationControllers[ANIMATION_DOWN]->SetAnimationCallbackHandler(ANIMATION_STATE_WALK_RIGHT, pAnimationCallbackHandler);

	pAnimationCallbackHandler = new CSoundCallbackHandler();
	ppAnimationControllers[ANIMATION_DOWN]->SetCallbackKeys(ANIMATION_STATE_WALK_LEFT, 2);
	ppAnimationControllers[ANIMATION_DOWN]->SetCallbackKey(ANIMATION_STATE_WALK_LEFT, 0, 0.2f, (void*)&CALLBACK_TYPE_SOUND_MOVE);
	ppAnimationControllers[ANIMATION_DOWN]->SetCallbackKey(ANIMATION_STATE_WALK_LEFT, 1, 0.5f, (void*)&CALLBACK_TYPE_SOUND_MOVE);
	ppAnimationControllers[ANIMATION_DOWN]->SetAnimationCallbackHandler(ANIMATION_STATE_WALK_LEFT, pAnimationCallbackHandler);

	pAnimationCallbackHandler = new CSoundCallbackHandler();
	ppAnimationControllers[ANIMATION_DOWN]->SetCallbackKeys(ANIMATION_STATE_WALK_BACKWARD, 2);
	ppAnimationControllers[ANIMATION_DOWN]->SetCallbackKey(ANIMATION_STATE_WALK_BACKWARD, 0, 0.13f, (void*)&CALLBACK_TYPE_SOUND_MOVE);
	ppAnimationControllers[ANIMATION_DOWN]->SetCallbackKey(ANIMATION_STATE_WALK_BACKWARD, 1, 0.5f, (void*)&CALLBACK_TYPE_SOUND_MOVE);
	ppAnimationControllers[ANIMATION_DOWN]->SetAnimationCallbackHandler(ANIMATION_STATE_WALK_BACKWARD, pAnimationCallbackHandler);

	pAnimationCallbackHandler = new CSwitchCallbackHandler();
	SWITCH *callbackSwitch = new SWITCH();
	callbackSwitch->bCondition = true;
	callbackSwitch->pbSwitch = &m_bShootStartEndPoint;
	ppAnimationControllers[ANIMATION_UP]->SetCallbackKeys(ANIMATION_STATE_GM_GUN_SHOOT_START, 1);
	ppAnimationControllers[ANIMATION_UP]->SetCallbackKey(ANIMATION_STATE_GM_GUN_SHOOT_START, 0, FLT_MAX, (void*)callbackSwitch);
	ppAnimationControllers[ANIMATION_UP]->SetAnimationCallbackHandler(ANIMATION_STATE_GM_GUN_SHOOT_START, pAnimationCallbackHandler);

	pAnimationCallbackHandler = new CSwitchCallbackHandler();
	callbackSwitch = new SWITCH();
	callbackSwitch->bCondition = true;
	callbackSwitch->pbSwitch = &m_bShootReturnEndPoint;
	ppAnimationControllers[ANIMATION_UP]->SetCallbackKeys(ANIMATION_STATE_GM_GUN_SHOOT_RETURN, 1);
	ppAnimationControllers[ANIMATION_UP]->SetCallbackKey(ANIMATION_STATE_GM_GUN_SHOOT_RETURN, 0, FLT_MAX, (void*)callbackSwitch);
	ppAnimationControllers[ANIMATION_UP]->SetAnimationCallbackHandler(ANIMATION_STATE_GM_GUN_SHOOT_RETURN, pAnimationCallbackHandler);

	pAnimationCallbackHandler = new CSwitchCallbackHandler();
	callbackSwitch = new SWITCH();
	callbackSwitch->bCondition = true;
	callbackSwitch->pbSwitch = &m_bShootOnceEndPoint;
	ppAnimationControllers[ANIMATION_UP]->SetCallbackKeys(ANIMATION_STATE_SHOOT_ONCE, 1);
	ppAnimationControllers[ANIMATION_UP]->SetCallbackKey(ANIMATION_STATE_SHOOT_ONCE, 0, FLT_MAX, (void*)callbackSwitch);
	ppAnimationControllers[ANIMATION_UP]->SetAnimationCallbackHandler(ANIMATION_STATE_SHOOT_ONCE, pAnimationCallbackHandler);

	pAnimationCallbackHandler = new CSwitchCallbackHandler();
	callbackSwitch = new SWITCH();
	callbackSwitch->bCondition = true;
	callbackSwitch->pbSwitch = &m_bSwordingEndPoint;
	ppAnimationControllers[ANIMATION_UP]->SetCallbackKeys(ANIMATION_STATE_BEAM_SABER_1_ONE, 1);
	ppAnimationControllers[ANIMATION_UP]->SetCallbackKey(ANIMATION_STATE_BEAM_SABER_1_ONE, 0, FLT_MAX, (void*)callbackSwitch);
	ppAnimationControllers[ANIMATION_UP]->SetAnimationCallbackHandler(ANIMATION_STATE_BEAM_SABER_1_ONE, pAnimationCallbackHandler);

	pAnimationCallbackHandler = new CSwitchCallbackHandler();
	callbackSwitch = new SWITCH();
	callbackSwitch->bCondition = true;
	callbackSwitch->pbSwitch = &m_bSwordingEndPoint;
	ppAnimationControllers[ANIMATION_UP]->SetCallbackKeys(ANIMATION_STATE_BEAM_SABER_2_ONE, 1);
	ppAnimationControllers[ANIMATION_UP]->SetCallbackKey(ANIMATION_STATE_BEAM_SABER_2_ONE, 0, FLT_MAX, (void*)callbackSwitch);
	ppAnimationControllers[ANIMATION_UP]->SetAnimationCallbackHandler(ANIMATION_STATE_BEAM_SABER_2_ONE, pAnimationCallbackHandler);

	pAnimationCallbackHandler = new CSwitchCallbackHandler();
	callbackSwitch = new SWITCH();
	callbackSwitch->bCondition = true;
	callbackSwitch->pbSwitch = &m_bSwordingEndPoint;
	ppAnimationControllers[ANIMATION_UP]->SetCallbackKeys(ANIMATION_STATE_BEAM_SABER_3_ONE, 1);
	ppAnimationControllers[ANIMATION_UP]->SetCallbackKey(ANIMATION_STATE_BEAM_SABER_3_ONE, 0, FLT_MAX, (void*)callbackSwitch);
	ppAnimationControllers[ANIMATION_UP]->SetAnimationCallbackHandler(ANIMATION_STATE_BEAM_SABER_3_ONE, pAnimationCallbackHandler);

	pAnimationCallbackHandler = new CSwitchCallbackHandler();
	callbackSwitch = new SWITCH();
	callbackSwitch->bCondition = true;
	callbackSwitch->pbSwitch = &m_bJumpEndPoint;
	ppAnimationControllers[ANIMATION_DOWN]->SetCallbackKeys(ANIMATION_STATE_JUMP, 1);
	ppAnimationControllers[ANIMATION_DOWN]->SetCallbackKey(ANIMATION_STATE_JUMP, 0, FLT_MAX, (void*)callbackSwitch);
	ppAnimationControllers[ANIMATION_DOWN]->SetAnimationCallbackHandler(ANIMATION_STATE_JUMP, pAnimationCallbackHandler);

	SetAnimationController(ppAnimationControllers[ANIMATION_UP], ANIMATION_UP);
	SetAnimationController(ppAnimationControllers[ANIMATION_DOWN], ANIMATION_DOWN);

	SetModel(pModel);
	
	CreateShaderVariables(pd3dDevice, pd3dCommandList);

	CHeightMapTerrain *pTerrain = (CHeightMapTerrain *)pContext;

	SetPlayerUpdatedContext(pTerrain);
	SetCameraUpdatedContext(pTerrain);

	CSkinnedAnimationShader *pPlayerShader = new CSkinnedAnimationShader();
	pPlayerShader->CreateShader(pd3dDevice, pd3dGraphicsRootSignature);
	SetShader(pPlayerShader);

	CShader *pShader = new CShader();
	pShader->CreateShader(pd3dDevice, pd3dGraphicsRootSignature);
	SetWeaponShader(pShader);
}

CPlayer::~CPlayer()
{
	ReleaseShaderVariables();

	if (m_pCamera) delete m_pCamera;

}

void CPlayer::CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList)
{
	if (m_pCamera) m_pCamera->CreateShaderVariables(pd3dDevice, pd3dCommandList);

	CGameObject::CreateShaderVariables(pd3dDevice, pd3dCommandList);
}

void CPlayer::ReleaseShaderVariables()
{
	if (m_pCamera) m_pCamera->ReleaseShaderVariables();
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

	m_pCamera->SetTimeLag(0.0f);
	m_pCamera->SetOffset(CAMERA_POSITION);
	m_pCamera->GenerateProjectionMatrix(1.01f, 5000.0f, ASPECT_RATIO, 60.0f);

	return(m_pCamera);
}

void CPlayer::Move(ULONG dwDirection, float fDistance)
{
	XMFLOAT3 xmf3Shift = XMFLOAT3(0, 0, 0);

	if (dwDirection & DIR_FORWARD)
	{
		if (m_nState & OBJECT_STATE_ONGROUND)
		{
			ChangeAnimation(ANIMATION_DOWN, 0, ANIMATION_STATE_WALK_FORWARD);

			if (!(m_nState & OBJECT_STATE_SHOOTING) && !(m_nState & OBJECT_STATE_SWORDING)) 
				ChangeAnimation(ANIMATION_UP, 0, ANIMATION_STATE_WALK_FORWARD);
		}

		xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Look, fDistance);
	}
	if (dwDirection & DIR_BACKWARD)
	{
		if (m_nState & OBJECT_STATE_ONGROUND)
		{
			ChangeAnimation(ANIMATION_DOWN, 0, ANIMATION_STATE_WALK_BACKWARD);

			if (!(m_nState & OBJECT_STATE_SHOOTING) && !(m_nState & OBJECT_STATE_SWORDING)) 
				ChangeAnimation(ANIMATION_UP, 0, ANIMATION_STATE_WALK_BACKWARD);
		}

		xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Look, -fDistance);
	}
	if (dwDirection & DIR_RIGHT)
	{
		if (m_nState & OBJECT_STATE_ONGROUND)
		{
			ChangeAnimation(ANIMATION_DOWN, 0, ANIMATION_STATE_WALK_RIGHT);

			if (!(m_nState & OBJECT_STATE_SHOOTING) && !(m_nState & OBJECT_STATE_SWORDING)) 
				ChangeAnimation(ANIMATION_UP, 0, ANIMATION_STATE_WALK_RIGHT);
		}

		xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Right, fDistance);
	}
	if (dwDirection & DIR_LEFT)
	{
		if (m_nState & OBJECT_STATE_ONGROUND)
		{
			ChangeAnimation(ANIMATION_DOWN, 0, ANIMATION_STATE_WALK_LEFT);

			if (!(m_nState & OBJECT_STATE_SHOOTING) && !(m_nState & OBJECT_STATE_SWORDING)) 
				ChangeAnimation(ANIMATION_UP, 0, ANIMATION_STATE_WALK_LEFT);
		}

		xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Right, -fDistance);
	}

	m_nState |= OBJECT_STATE_MOVING;
	Move(xmf3Shift);
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
	ProcessAnimation();

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
	CRobotObject::Render(pd3dCommandList, pCamera, true);
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

void CPlayer::OnPlayerUpdateCallback(float fTimeElapsed)
{
	//if(m_pScene) m_pScene->CheckCollisionPlayer();

	CHeightMapTerrain *pTerrain = (CHeightMapTerrain *)m_pPlayerUpdatedContext;
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

		if(!(m_nState & OBJECT_STATE_BOOSTERING))
			m_nState &= ~OBJECT_STATE_FLYING;
	}
	else
	{
		m_nState &= ~OBJECT_STATE_ONGROUND;
		m_nState |= OBJECT_STATE_FLYING;
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
	// Active by Space Bar
	if (!(m_nState & OBJECT_STATE_BOOSTERING) && m_nBoosterGauge > 0)
	{
		if (!(m_nState & OBJECT_STATE_FLYING))
		{
			ChangeAnimation(ANIMATION_DOWN, 0, ANIMATION_STATE_JUMP, true);

			if (!(m_nState & OBJECT_STATE_SHOOTING) && !(m_nState & OBJECT_STATE_SWORDING))
				ChangeAnimation(ANIMATION_UP, 0, ANIMATION_STATE_JUMP, true);

			m_nState |= OBJECT_STATE_JUMPING;
		}

		m_nState |= OBJECT_STATE_BOOSTERING;

		m_bChargeBG = false;
		SetElapsedBGConsumeTime();
	}
}

void CPlayer::ProcessBoosterGauge(float fTimeElapsed)
{
	if (m_nState & OBJECT_STATE_BOOSTERING)
	{
		if (m_fElapsedBGConsumeTime <= 0.0f)
		{
			SetElapsedBGConsumeTime();
			m_nBoosterGauge -= 5;
		}
		else m_fElapsedBGConsumeTime -= fTimeElapsed;

		if (m_nBoosterGauge <= 0)
		{
			DeactivationBooster();
			m_nBoosterGauge = 0;
		}
		else m_fKeepBoosteringTime += fTimeElapsed;
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
		float fBoosterPower = m_fGravity + m_fKeepBoosteringTime * m_fBoosterBasicForce;

		m_fAccelerationY += fBoosterPower;
	}

	m_fVelocityY += (m_fAccelerationY * fTimeElapsed);
	if (m_fVelocityY > m_fMaxSpeed) m_fVelocityY = m_fMaxSpeed;

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

void CPlayer::ProcessMouseUpTime(float fTimeElapsed)
{
	if (!m_LButtonDown)
	{
		m_fMouseUpTime += fTimeElapsed;

		if (m_fMouseUpTime > 4.0f)
		{
			if (m_nState & OBJECT_STATE_SHOOTING)
				m_nState &= ~OBJECT_STATE_SHOOTING;

			if (m_nState & OBJECT_STATE_SWORDING)
				m_nState &= ~OBJECT_STATE_SWORDING;
		}

		if (m_fMouseUpTime > 1.0f)
		{
			m_nSaberAnimationIndex = 0;
		}
	}
}

void CPlayer::ProcessAnimation()
{
	if (m_nState & OBJECT_STATE_SHOOTING)
	{
		// Start 자세 End Point
		if (m_bShootStartEndPoint)
		{
			m_bShootStartEndPoint = false;
			m_bChangeableOnceAni = true;
		}

		// Return 자세 End Point
		if (m_bShootReturnEndPoint)
		{
			m_bShootReturnEndPoint = false;
			m_nState &= ~OBJECT_STATE_SHOOTING;
		}

		// Start 자세 이후
		if (m_bChangeableOnceAni)
		{
			CGun *pGun = (CGun*)m_pRHWeapon;

			if (pGun->IsShootable())
			{
				ChangeAnimation(ANIMATION_UP, 0, ANIMATION_STATE_SHOOT_ONCE, true);
				pGun->Shot();
				m_bShootable = false;
			}

			if (m_LButtonDown)
			{
				if (m_bShootOnceEndPoint)
				{
					ChangeAnimation(ANIMATION_UP, 0, ANIMATION_STATE_GM_GUN_SHOOT_START, true);
					m_ppAnimationControllers[ANIMATION_UP]->SetTrackPosition(0, m_ppAnimationControllers[ANIMATION_UP]->GetTrackLength(0));

					m_bShootable = true;
					m_bShootOnceEndPoint = false;
				}
			}

			if (!m_LButtonDown)
			{
				if ( (pGun->ShootNumber() == pGun->ShootedCount()) || (pGun->ShootedCount() == 0))
				{
					ChangeAnimation(ANIMATION_UP, 0, ANIMATION_STATE_GM_GUN_SHOOT_RETURN, true);

					if (!(m_nState & OBJECT_STATE_MOVING) && !(m_nState & OBJECT_STATE_JUMPING))
					{
						ChangeAnimation(ANIMATION_DOWN, 0, ANIMATION_STATE_GM_GUN_SHOOT_RETURN, true);
					}

					m_bShootOnceEndPoint = false;
					m_bChangeableOnceAni = false;
				}
			}
		}
	}
	
	if (m_nState & OBJECT_STATE_FLYING)
	{
		if (m_bJumpEndPoint)
		{
			ChangeAnimation(ANIMATION_DOWN, 0, ANIMATION_STATE_JUMP_LOOP);

			if (!(m_nState & OBJECT_STATE_SHOOTING) && !(m_nState & OBJECT_STATE_SWORDING))
				ChangeAnimation(ANIMATION_UP, 0, ANIMATION_STATE_JUMP_LOOP);

			m_bJumpEndPoint = false;
			m_nState &= ~OBJECT_STATE_JUMPING;
		}

		if (!(m_nState & OBJECT_STATE_JUMPING))
		{
			ChangeAnimation(ANIMATION_DOWN, 0, ANIMATION_STATE_JUMP_LOOP);
			
			if (!(m_nState & OBJECT_STATE_SHOOTING) && !(m_nState & OBJECT_STATE_SWORDING))
				ChangeAnimation(ANIMATION_UP, 0, ANIMATION_STATE_JUMP_LOOP);
		}
	}

	if (m_nState & OBJECT_STATE_SWORDING)
	{
		if (m_bSwordingEndPoint)
		{
			if (m_LButtonDown)
			{
				ChangeAnimation(ANIMATION_UP, 0, m_nAnimationList[m_nSaberAnimationIndex], true);

				m_nSaberAnimationIndex = (m_nSaberAnimationIndex + 1) % 3;
			}
			else
				m_nState &= ~OBJECT_STATE_SWORDING;

			m_bSwordingEndPoint = false;
		}
	}

	// 땅 위에서 가만히 있을 때
	// 총 쏘는 도중 or 아이들
	if ((m_nState & OBJECT_STATE_ONGROUND) && !(m_nState & OBJECT_STATE_MOVING) && !(m_nState & OBJECT_STATE_JUMPING))
	{
		// 총 쏘는 도중일 때
		if (m_nState & OBJECT_STATE_SHOOTING)
		{
			if (m_pnAnimationState[ANIMATION_UP] == ANIMATION_STATE_GM_GUN_SHOOT_START)
			{
				ChangeAnimation(ANIMATION_DOWN, 0, ANIMATION_STATE_GM_GUN_SHOOT_START);
			}
			else if(m_pnAnimationState[ANIMATION_UP] == ANIMATION_STATE_GM_GUN_SHOOT_RETURN)
			{
				ChangeAnimation(ANIMATION_DOWN, 0, ANIMATION_STATE_GM_GUN_SHOOT_RETURN);
			}
		}
		else
		{
			ChangeAnimation(ANIMATION_DOWN, 0, ANIMATION_STATE_IDLE);

			if (!(m_nState & OBJECT_STATE_SWORDING))
				ChangeAnimation(ANIMATION_UP, 0, ANIMATION_STATE_IDLE);
		}
	}
}

void CPlayer::Attack(CWeapon *pWeapon)
{
	// Active By L Button Down
	m_LButtonDown = true;

	if (pWeapon)
	{
		int nType = pWeapon->GetType();

		if (nType & WEAPON_TYPE_OF_GUN)
		{
			CGun *pGun = (CGun*)pWeapon;

			if (!m_bReloading)
			{
				if (!(m_nState & OBJECT_STATE_SHOOTING))
				{
					ChangeAnimation(ANIMATION_UP, 0, ANIMATION_STATE_GM_GUN_SHOOT_START, true);

					if (!(m_nState & OBJECT_STATE_MOVING) && (m_nState & OBJECT_STATE_ONGROUND))
						ChangeAnimation(ANIMATION_DOWN, 0, ANIMATION_STATE_GM_GUN_SHOOT_START, true);

					m_nState |= OBJECT_STATE_SHOOTING;
				}
			}
		}
		else if (nType & WEAPON_TYPE_OF_SABER)
		{
			if (!(m_nState & OBJECT_STATE_SWORDING))
			{
				ChangeAnimation(ANIMATION_UP, 0, m_nAnimationList[m_nSaberAnimationIndex], true);

				m_nSaberAnimationIndex = (m_nSaberAnimationIndex + 1) % 3;

				m_nState |= OBJECT_STATE_SWORDING;
				m_bSwordingEndPoint = false;
			}
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

		if (pGun->GetReloadedAmmo() < pGun->GetMaxReloadAmmo())
		{
			if (!m_bReloading)
			{
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

	ProcessMouseUpTime(fTimeElapsed);
}

void CPlayer::ChangeWeapon(int nIndex)
{
	if (m_nState & OBJECT_STATE_SHOOTING) return;
	if (m_nState & OBJECT_STATE_SWORDING) return;

	CRobotObject::ChangeWeapon(nIndex);

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
		else if (nType & WEAPON_TYPE_OF_SABER)
			return WEAPON_TYPE::WEAPON_TYPE_SABER;
	}

	return WEAPON_TYPE::WEAPON_TYPE_BEAM_RIFLE;
}

void CPlayer::AddWeapon(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, CModel *pWeaponModel, int nType, CShader *pBulletShader, CShader *pEffectShader, int nGroup)
{
	CWeapon *pWeapon = NULL;

	switch (nType)
	{
	case WEAPON_TYPE_OF_GIM_GUN:
		pWeapon = new CGimGun();
		if (pBulletShader) ((CGimGun*)pWeapon)->SetBullet(pBulletShader, pEffectShader, nGroup);
		break;
	case WEAPON_TYPE_OF_BAZOOKA:
		pWeapon = new CBazooka();
		if (pBulletShader) ((CBazooka*)pWeapon)->SetBullet(pBulletShader, pEffectShader, nGroup);
		break;
	case WEAPON_TYPE_OF_MACHINEGUN:
		pWeapon = new CMachineGun();
		if (pBulletShader) ((CMachineGun*)pWeapon)->SetBullet(pBulletShader, pEffectShader, nGroup);
		break;
	case WEAPON_TYPE_OF_SABER:
		pWeapon = new CSaber();
		break;
	default:
		exit(1);
		break;
	}

	pWeapon->SetModel(pWeaponModel);
	pWeapon->CreateShaderVariables(pd3dDevice, pd3dCommandList);
	pWeapon->Initialize();
	pWeapon->SetForCreate(pd3dDevice, pd3dCommandList);
	pWeapon->AddPrepareRotate(180.0f, 90.0f, -90.0f);

	if (!m_pRHWeapon) EquipOnRightHand(pWeapon);

	m_vpWeapon.emplace_back(pWeapon);
}
