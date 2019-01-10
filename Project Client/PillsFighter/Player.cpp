#include "stdafx.h"
#include "Player.h"
#include "Shader.h"
#include "Repository.h"

#define CAMERA_POSITION XMFLOAT3(0.0f, 30.0f, -35.0f)

CPlayer::CPlayer(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature, CRepository *pRepository, void *pContext) : CGameObject()
{
	m_nHitPoint = m_nMaxHitPoint = 100;
	m_ShotTime = 0;
	SetPosition(XMFLOAT3(0.0f, 0.0f, -50.0f));

	m_pCamera = SetCamera(0.0f);
	CreateShaderVariables(pd3dDevice, pd3dCommandList);

	m_pShader = new CShader();
	m_pShader->CreateShader(pd3dDevice, pd3dGraphicsRootSignature);

	m_pModel = pRepository->GetModel(pd3dDevice, pd3dCommandList, "./Resource/GM/GM2.fbx");
	m_pModel->CreateShaderResourceViews(pd3dDevice, pd3dCommandList, m_pShader);

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
	m_pModel = NULL;

	ReleaseShaderVariables();

	if (m_pUserInterface) delete m_pUserInterface;

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

	if (m_pUserInterface) m_pUserInterface->ReleaseShaderVariables();

	CGameObject::ReleaseShaderVariables();
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
	m_pCamera->GenerateProjectionMatrix(1.0f, 5000.0f, ASPECT_RATIO, 60.0f);
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

		if (dwDirection & DIR_FORWARD) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Look, fDistance);
		if (dwDirection & DIR_BACKWARD) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Look, -fDistance);
		if (dwDirection & DIR_RIGHT) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Right, fDistance);
		if (dwDirection & DIR_LEFT) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Right, -fDistance);
		if (dwDirection & DIR_UP)
		{
			if (m_nBoosterGauge > 0)
			{
				if (!(m_nState & OBJECT_STATE_BOOSTERING))
				{
					m_nState |= OBJECT_STATE_BOOSTERING;
					m_nState &= ~OBJECT_STATE_ONGROUND;
				}

				SetBoosterPower(0.5f);
			}
		}
		if ( (m_nState & OBJECT_STATE_BOOSTERING) && (dwDirection & DIR_DOWN) )	SetBoosterPower(-0.5f);

		Move(xmf3Shift);
	}
}

void CPlayer::Move(const XMFLOAT3& xmf3Shift)
{
	m_xmf3PrevPosition = m_xmf3Position;
	m_xmf3Position = Vector3::Add(m_xmf3Position, xmf3Shift);

	m_pCamera->Move(xmf3Shift);
}

void CPlayer::Update(float fTimeElapsed)
{
	m_fGravity = m_fGravAcc * m_fMass * fTimeElapsed;
	m_fAccelerationY -= m_fGravity;

	if (m_nState & OBJECT_STATE_BOOSTERING)
	{
		if (!IsZero(m_fBoosterPower))
		{
			bool bIsNegativeNum = false;
			if (m_fBoosterPower < FLT_EPSILON) bIsNegativeNum = true;

			m_fBoosterPower = fabsf(m_fBoosterPower);
			float fNormaize = m_fBoosterPower / m_fBoosterPower;
			m_fBoosterPower -= fNormaize * fTimeElapsed;
			if (m_fBoosterPower < FLT_EPSILON)
				m_fBoosterPower = 0.0f;
			else
			{
				if (bIsNegativeNum) m_fBoosterPower *= -1.0f;
			}
		}
		float fHoldPower = m_fGravity;
		m_fAccelerationY += m_fBoosterPower + fHoldPower;

		if (fabsf(m_fAccelerationY) < FLT_EPSILON) m_fVelocityY = 0.0f;
	}

	m_fVelocityY += (m_fAccelerationY * fTimeElapsed);
	Move(XMFLOAT3(0.0f, m_fVelocityY, 0.0f));
	m_fAccelerationY = 0.0f;

	if (m_pPlayerUpdatedContext) OnPlayerUpdateCallback(fTimeElapsed);
	m_pCamera->Update(m_xmf3Position, fTimeElapsed);
	if (m_pCameraUpdatedContext) OnCameraUpdateCallback(fTimeElapsed);

	XMFLOAT3 xmf3LookAt = Vector3::Add(m_xmf3Position, XMFLOAT3(0.0f, 20.0f, 0.0f));
	//m_pCamera->SetLookAt(m_xmf3Position);
	m_pCamera->SetLookAt(xmf3LookAt);

	m_pCamera->RegenerateViewMatrix();

	ProcessBooster(fTimeElapsed);
	ProcessOnGround(fTimeElapsed);
	ProcessHitPoint();

	CGameObject::Animate(fTimeElapsed);

	CheckElapsedTime(fTimeElapsed);

	printf("HP : %d, BG : %d\n", m_nHitPoint, m_nBoosterGauge);
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

	m_pCamera->Rotate(x, 0.0f, 0.0f);

	m_xmf3Look = Vector3::Normalize(m_xmf3Look);
	m_xmf3Right = Vector3::CrossProduct(m_xmf3Up, m_xmf3Look, true);
	m_xmf3Up = Vector3::CrossProduct(m_xmf3Look, m_xmf3Right, true);
}

void CPlayer::Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera)
{
	if (m_pUserInterface) m_pUserInterface->Render(pd3dCommandList, pCamera);

	//플레이어 객체를 렌더링한다.	
	if(m_pShader)
		m_pShader->OnPrepareRender(pd3dCommandList);

	CGameObject::Render(pd3dCommandList, pCamera, m_pShader->m_nHandleIndex);
}

void CPlayer::RenderWire(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera)
{
	//플레이어 충돌박스를 렌더링한다.
	if(m_pShader)
		m_pShader->OnPrepareRenderWire(pd3dCommandList);

	CGameObject::RenderWire(pd3dCommandList, pCamera);
}

void CPlayer::Shot(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList)
{
	if (m_Shotable)
	{
#ifndef ON_NETWORKING
		Bullet *pBullet = NULL;

		pBullet = new Bullet();

		XMFLOAT3 xmfPosition = GetPosition();
		xmfPosition = Vector3::Add(xmfPosition, XMFLOAT3(0.0f, 5.0f, 0.0f));
		pBullet->SetPosition(xmfPosition);
		//pBullet->SetRight(m_pCamera->GetRightVector());
		//pBullet->SetUp(m_pCamera->GetUpVector());
		//pBullet->SetLook(m_pCamera->GetLookVector());
		pBullet->SetRight(m_xmf3Right);
		pBullet->SetUp(m_xmf3Up);
		pBullet->SetLook(m_xmf3Look);
		pBullet->SetPrepareRotate(0.0f, 0.0f, 0.0f);

		m_pBulletShader->InsertObject(pd3dDevice, pd3dCommandList, pBullet);
#endif

		m_Shotable = FALSE;
	}
}

void CPlayer::CheckElapsedTime(float ElapsedTime)
{
	if (!m_Shotable)
	{
		if (m_ShotTime > SHOT_COOLTIME)
		{
			m_ShotTime = 0.0f;
			m_Shotable = TRUE;
		}
		else
			m_ShotTime += ElapsedTime;
	}
}

void CPlayer::OnPlayerUpdateCallback(float fTimeElapsed)
{
	CHeightMapTerrain *pTerrain = (CHeightMapTerrain *)m_pPlayerUpdatedContext;
	XMFLOAT3 xmf3Scale = pTerrain->GetScale();
	XMFLOAT3 xmf3PlayerPosition = GetPosition();
	int z = (int)(xmf3PlayerPosition.z / xmf3Scale.z);
	bool bReverseQuad = ((z % 2) != 0);
	float fHeight = pTerrain->GetHeight(xmf3PlayerPosition.x, xmf3PlayerPosition.z, bReverseQuad);
	std::cout << "Height : " << fHeight << std::endl;
	if (xmf3PlayerPosition.y < fHeight)
	{
		float fPlayerVelocity = GetVelocity();
		fPlayerVelocity = 0.0f;
		SetVelocity(fPlayerVelocity);
		xmf3PlayerPosition.y = fHeight;
		SetPosition(xmf3PlayerPosition);

		m_nState |= OBJECT_STATE_ONGROUND;
		m_fBoosteringTime = 0.0f;
		if (m_nState & OBJECT_STATE_BOOSTERING) m_nState &= ~OBJECT_STATE_BOOSTERING;
	}
}

void CPlayer::OnCameraUpdateCallback(float fTimeElapsed)
{
	CHeightMapTerrain *pTerrain = (CHeightMapTerrain *)m_pCameraUpdatedContext;
	XMFLOAT3 xmf3Scale = pTerrain->GetScale();
	XMFLOAT3 xmf3CameraPosition = m_pCamera->GetPosition();
	int z = (int)(xmf3CameraPosition.z / xmf3Scale.z);
	bool bReverseQuad = ((z % 2) != 0);
	float fHeight = pTerrain->GetHeight(xmf3CameraPosition.x, xmf3CameraPosition.z, bReverseQuad);
	if (xmf3CameraPosition.y <= fHeight)
	{
		xmf3CameraPosition.y = fHeight;
		m_pCamera->SetPosition(xmf3CameraPosition);

		CCamera *pCamera = (CCamera *)m_pCamera;
		pCamera->SetLookAt(GetPosition());
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

void CPlayer::ProcessBooster(float fTimeElapsed)
{
	if (m_nState & OBJECT_STATE_BOOSTERING)
	{
		m_fOnGroundTime = 0.0f;

		// 부스터 지속시간 처리
		if (m_fBoosteringTime > 1.0f)
		{
			m_fBoosteringTime = 0.0f;
			m_nBoosterGauge -= 5;
		}

		// 부스터 게이지 처리
		if (m_nBoosterGauge <= 0)
		{
			m_nState &= ~OBJECT_STATE_BOOSTERING;
			m_nBoosterGauge = 0;
		}

		m_fBoosteringTime += fTimeElapsed;
	}
	else if (m_nState & OBJECT_STATE_ONGROUND)
	{
		if (m_fOnGroundTime > 4.0f)
		{
			if (m_nBoosterGauge < 100)
			{
				if (m_fBoosterGaugeChargeTime > 0.5f)
				{
					m_nBoosterGauge += 20;
					m_fBoosterGaugeChargeTime = 0.0f;
				}

				m_fBoosterGaugeChargeTime += fTimeElapsed;
			}
			else
				m_nBoosterGauge = 100;
		}
	}
}

void CPlayer::ProcessOnGround(float fTimeElapsed)
{
	if(m_nState & OBJECT_STATE_ONGROUND)
	{
		m_fOnGroundTime += fTimeElapsed;
	}
}