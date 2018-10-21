#include "stdafx.h"
#include "Player.h"
#include "Shader.h"

#define CAMERA_POSITION XMFLOAT3(0.0f, 30.0f, -50.0f)

CPlayer::CPlayer(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature, void *pContext) : CGameObject()
{
	//플레이어의 카메라를 3인칭 카메라로 변경(생성)한다.
	m_pCamera = SetCamera(0.0f);

	CreateShaderVariables(pd3dDevice, pd3dCommandList);

	// 플레이어 쉐이더 생성
	m_pShader = new CShader();
	m_pShader->CreateShader(pd3dDevice, pd3dGraphicsRootSignature);
	
	UINT nTextures;
	CTexture **ppTexture;

	::CreateRobotObjectMesh(pd3dDevice, pd3dCommandList, m_ppMeshes, m_ppCubeMeshes, m_nMeshes);
	::CreateRobotObjectTexture(pd3dDevice, pd3dCommandList, ppTexture, nTextures);
	::CreateRobotObjectShader(pd3dDevice, pd3dCommandList, ppTexture, m_pShader);

	UINT nMaterials = nTextures;
	CMaterial **ppMaterials = new CMaterial*[nMaterials];

	for (UINT i = 0; i < nTextures; i++)
	{
		ppMaterials[i] = new CMaterial();
		ppMaterials[i]->SetTexture(ppTexture[i]);
	}

	m_pShader->SetMaterial(ppMaterials, nMaterials);
	SetMaterial(ppMaterials, nMaterials);

	//플레이어의 위치를 설정한다. 
	SetPosition(XMFLOAT3(0.0f, 0.0f, -50.0f));

	m_ShotTime = 0;
}

CPlayer::~CPlayer()
{
	ReleaseShaderVariables();

	if (m_ppMeshes)
	{
		for (UINT j = 0; j < m_nMeshes; j++)
		{
			if (m_ppMeshes[j]) delete m_ppMeshes[j];
			if (m_ppCubeMeshes[j]) delete m_ppCubeMeshes[j];
		}

		delete[] m_ppMeshes;
		delete[] m_ppCubeMeshes;
	}

	if (m_pShader) delete m_pShader;

	if (m_pCamera) delete m_pCamera;
}

void CPlayer::CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList)
{
	if (m_pCamera) m_pCamera->CreateShaderVariables(pd3dDevice, pd3dCommandList);

	UINT ncbElementBytes = ((sizeof(CB_PLAYER_INFO) + 255) & ~255); //256의 배수
	m_pd3dcbPlayer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes, 
		D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);

	m_pd3dcbPlayer->Map(0, NULL, (void **)&m_pcbMappedPlayer);
}

void CPlayer::ReleaseShaderVariables()
{
	if (m_pCamera) m_pCamera->ReleaseShaderVariables();

	if (m_pd3dcbPlayer)
	{
		m_pd3dcbPlayer->Unmap(0, NULL);
		m_pd3dcbPlayer->Release();
	}

	if (m_pShader)
	{
		m_pShader->ReleaseShaderVariables();
	}

	CGameObject::ReleaseShaderVariables();
}

void CPlayer::UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList)
{
	XMStoreFloat4x4(&m_pcbMappedPlayer->m_xmf4x4World, XMMatrixTranspose(XMLoadFloat4x4(&m_xmf4x4World)));

	D3D12_GPU_VIRTUAL_ADDRESS d3dGpuVirtualAddress = m_pd3dcbPlayer->GetGPUVirtualAddress();
	pd3dCommandList->SetGraphicsRootConstantBufferView(0, d3dGpuVirtualAddress);
}

void CPlayer::ReleaseUploadBuffers()
{
	if (m_pShader) m_pShader->ReleaseUploadBuffers();

	CGameObject::ReleaseUploadBuffers();
}

CCamera *CPlayer::SetCamera(float fTimeElapsed)
{
	//플레이어의 특성을 3인칭 카메라 모드에 맞게 변경한다. 지연 효과와 카메라 오프셋을 설정한다. 
	m_pCamera = new CCamera();

	m_xmf3Right = m_pCamera->GetRightVector();
	m_xmf3Up = m_pCamera->GetUpVector();
	m_xmf3Look = m_pCamera->GetLookVector();

	m_pCamera->SetPlayer(this);

	//3인칭 카메라의 지연 효과를 설정한다.
	m_pCamera->SetTimeLag(0.0f);

	m_pCamera->SetOffset(CAMERA_POSITION);
	m_pCamera->GenerateProjectionMatrix(1.0f, 5000.0f, ASPECT_RATIO, 60.0f);
	m_pCamera->SetViewport(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 0.0f, 1.0f);
	m_pCamera->SetScissorRect(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT);

	//플레이어를 시간의 경과에 따라 갱신(위치와 방향을 변경: 속도, 마찰력, 중력 등을 처리)한다. 
	Update(fTimeElapsed);


	return(m_pCamera);
}
/*플레이어의 위치를 변경하는 함수이다. 플레이어의 위치는 기본적으로 사용자가 플레이어를 이동하기 위한 키보드를
누를 때 변경된다. 플레이어의 이동 방향(dwDirection)에 따라 플레이어를 fDistance 만큼 이동한다.*/
void CPlayer::Move(ULONG dwDirection, float fDistance)
{
	if (dwDirection)
	{
		XMFLOAT3 xmf3Shift = XMFLOAT3(0, 0, 0);

		//화살표 키 ‘↑’를 누르면 로컬 z-축 방향으로 이동(전진)한다. ‘↓’를 누르면 반대 방향으로 이동한다. 
		if (dwDirection & DIR_FORWARD) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Look, fDistance);
		if (dwDirection & DIR_BACKWARD) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Look,	-fDistance);

		//화살표 키 ‘→’를 누르면 로컬 x-축 방향으로 이동한다. ‘←’를 누르면 반대 방향으로 이동한다. 
		if (dwDirection & DIR_RIGHT) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Right, fDistance);
		if (dwDirection & DIR_LEFT) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Right, -fDistance);

		//‘Page Up’을 누르면 로컬 y-축 방향으로 이동한다. ‘Page Down’을 누르면 반대 방향으로 이동한다. 
		if (dwDirection & DIR_UP) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Up, fDistance);
		if (dwDirection & DIR_DOWN) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Up, -fDistance);

		//플레이어를 현재 위치 벡터에서 xmf3Shift 벡터만큼 이동한다. 
		Move(xmf3Shift);
	}
}

void CPlayer::Move(const XMFLOAT3& xmf3Shift)
{
	//플레이어를 현재 위치 벡터에서 xmf3Shift 벡터만큼 이동한다. 
	m_xmf3Position = Vector3::Add(m_xmf3Position, xmf3Shift);

	//플레이어의 위치가 변경되었으므로 카메라의 위치도 xmf3Shift 벡터만큼 이동한다. 
	m_pCamera->Move(xmf3Shift);
}

void CPlayer::Update(float fTimeElapsed)
{
	/*플레이어의 위치가 변경될 때 추가로 수행할 작업을 수행한다. 
	플레이어의 새로운 위치가 유효한 위치가 아닐 수도 있고 또는 플레이어의 충돌 검사 등을 수행할 필요가 있다. 
	이러한 상황에서 플레이어의 위치를 유효한 위치로 다시 변경할 수 있다.*/
	if (m_pPlayerUpdatedContext) OnPlayerUpdateCallback(fTimeElapsed);

	//플레이어의 위치가 변경되었으므로 카메라를 갱신한다. 
	m_pCamera->Update(m_xmf3Position, fTimeElapsed);

	//카메라의 위치가 변경될 때 추가로 수행할 작업을 수행한다. 
	if (m_pCameraUpdatedContext) OnCameraUpdateCallback(fTimeElapsed);

	//카메라가 변경된 플레이어 위치를 바라보도록 한다. 
	XMFLOAT3 xmf3LookAt = Vector3::Add(m_xmf3Position, XMFLOAT3(0.0f, 10.0f, 0.0f));
	//m_pCamera->SetLookAt(m_xmf3Position);
	m_pCamera->SetLookAt(xmf3LookAt);

	//카메라의 카메라 변환 행렬을 다시 생성한다. 
	m_pCamera->RegenerateViewMatrix();

	CheckElapsedTime(fTimeElapsed);
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
}

void CPlayer::Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera)
{
	//플레이어 객체를 렌더링한다. 

	m_pShader->OnPrepareRender(pd3dCommandList);

	CGameObject::Render(pd3dCommandList, pCamera);
}

void CPlayer::RenderWire(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera)
{
	//플레이어 충돌박스를 렌더링한다.
	m_pShader->OnPrepareRenderWire(pd3dCommandList);

	CGameObject::RenderWire(pd3dCommandList, pCamera);
}

void CPlayer::Shot(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList)
{
	if (m_Shotable)
	{
		Bullet *pBullet = NULL;

		pBullet = new Bullet();
		pBullet->SetPrepareRotate(0.0f, 0.0f, 0.0f);
		pBullet->CreateShaderVariables(pd3dDevice, pd3dCommandList);

		XMFLOAT3 xmfPosition = GetPosition();
		xmfPosition = Vector3::Add(xmfPosition, XMFLOAT3(0.0f, 5.0f, 0.0f));
		pBullet->SetPosition(xmfPosition);
		//pBullet->SetRight(m_pCamera->GetRightVector());
		//pBullet->SetUp(m_pCamera->GetUpVector());
		//pBullet->SetLook(m_pCamera->GetLookVector());
		pBullet->SetRight(m_xmf3Right);
		pBullet->SetUp(m_xmf3Up);
		pBullet->SetLook(m_xmf3Look);

		m_pBulletShader->InsertObject(pBullet);

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