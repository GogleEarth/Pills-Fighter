#include "stdafx.h"
#include "Player.h"
#include "Shader.h"

#define CAMERA_POSITION XMFLOAT3(0.0f, 30.0f, -50.0f)

CPlayer::CPlayer(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature, void *pContext) : CGameObject()
{
	//�÷��̾��� ī�޶� 3��Ī ī�޶�� ����(����)�Ѵ�.
	m_pCamera = SetCamera(0.0f);

	CreateShaderVariables(pd3dDevice, pd3dCommandList);

	// �÷��̾� ���̴� ����
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

	//�÷��̾��� ��ġ�� �����Ѵ�. 
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

	UINT ncbElementBytes = ((sizeof(CB_PLAYER_INFO) + 255) & ~255); //256�� ���
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
	//�÷��̾��� Ư���� 3��Ī ī�޶� ��忡 �°� �����Ѵ�. ���� ȿ���� ī�޶� �������� �����Ѵ�. 
	m_pCamera = new CCamera();

	m_xmf3Right = m_pCamera->GetRightVector();
	m_xmf3Up = m_pCamera->GetUpVector();
	m_xmf3Look = m_pCamera->GetLookVector();

	m_pCamera->SetPlayer(this);

	//3��Ī ī�޶��� ���� ȿ���� �����Ѵ�.
	m_pCamera->SetTimeLag(0.0f);

	m_pCamera->SetOffset(CAMERA_POSITION);
	m_pCamera->GenerateProjectionMatrix(1.0f, 5000.0f, ASPECT_RATIO, 60.0f);
	m_pCamera->SetViewport(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 0.0f, 1.0f);
	m_pCamera->SetScissorRect(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT);

	//�÷��̾ �ð��� ����� ���� ����(��ġ�� ������ ����: �ӵ�, ������, �߷� ���� ó��)�Ѵ�. 
	Update(fTimeElapsed);


	return(m_pCamera);
}
/*�÷��̾��� ��ġ�� �����ϴ� �Լ��̴�. �÷��̾��� ��ġ�� �⺻������ ����ڰ� �÷��̾ �̵��ϱ� ���� Ű���带
���� �� ����ȴ�. �÷��̾��� �̵� ����(dwDirection)�� ���� �÷��̾ fDistance ��ŭ �̵��Ѵ�.*/
void CPlayer::Move(ULONG dwDirection, float fDistance)
{
	if (dwDirection)
	{
		XMFLOAT3 xmf3Shift = XMFLOAT3(0, 0, 0);

		//ȭ��ǥ Ű ���衯�� ������ ���� z-�� �������� �̵�(����)�Ѵ�. ���顯�� ������ �ݴ� �������� �̵��Ѵ�. 
		if (dwDirection & DIR_FORWARD) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Look, fDistance);
		if (dwDirection & DIR_BACKWARD) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Look,	-fDistance);

		//ȭ��ǥ Ű ���桯�� ������ ���� x-�� �������� �̵��Ѵ�. ���硯�� ������ �ݴ� �������� �̵��Ѵ�. 
		if (dwDirection & DIR_RIGHT) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Right, fDistance);
		if (dwDirection & DIR_LEFT) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Right, -fDistance);

		//��Page Up���� ������ ���� y-�� �������� �̵��Ѵ�. ��Page Down���� ������ �ݴ� �������� �̵��Ѵ�. 
		if (dwDirection & DIR_UP) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Up, fDistance);
		if (dwDirection & DIR_DOWN) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Up, -fDistance);

		//�÷��̾ ���� ��ġ ���Ϳ��� xmf3Shift ���͸�ŭ �̵��Ѵ�. 
		Move(xmf3Shift);
	}
}

void CPlayer::Move(const XMFLOAT3& xmf3Shift)
{
	//�÷��̾ ���� ��ġ ���Ϳ��� xmf3Shift ���͸�ŭ �̵��Ѵ�. 
	m_xmf3Position = Vector3::Add(m_xmf3Position, xmf3Shift);

	//�÷��̾��� ��ġ�� ����Ǿ����Ƿ� ī�޶��� ��ġ�� xmf3Shift ���͸�ŭ �̵��Ѵ�. 
	m_pCamera->Move(xmf3Shift);
}

void CPlayer::Update(float fTimeElapsed)
{
	/*�÷��̾��� ��ġ�� ����� �� �߰��� ������ �۾��� �����Ѵ�. 
	�÷��̾��� ���ο� ��ġ�� ��ȿ�� ��ġ�� �ƴ� ���� �ְ� �Ǵ� �÷��̾��� �浹 �˻� ���� ������ �ʿ䰡 �ִ�. 
	�̷��� ��Ȳ���� �÷��̾��� ��ġ�� ��ȿ�� ��ġ�� �ٽ� ������ �� �ִ�.*/
	if (m_pPlayerUpdatedContext) OnPlayerUpdateCallback(fTimeElapsed);

	//�÷��̾��� ��ġ�� ����Ǿ����Ƿ� ī�޶� �����Ѵ�. 
	m_pCamera->Update(m_xmf3Position, fTimeElapsed);

	//ī�޶��� ��ġ�� ����� �� �߰��� ������ �۾��� �����Ѵ�. 
	if (m_pCameraUpdatedContext) OnCameraUpdateCallback(fTimeElapsed);

	//ī�޶� ����� �÷��̾� ��ġ�� �ٶ󺸵��� �Ѵ�. 
	XMFLOAT3 xmf3LookAt = Vector3::Add(m_xmf3Position, XMFLOAT3(0.0f, 10.0f, 0.0f));
	//m_pCamera->SetLookAt(m_xmf3Position);
	m_pCamera->SetLookAt(xmf3LookAt);

	//ī�޶��� ī�޶� ��ȯ ����� �ٽ� �����Ѵ�. 
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
	//�÷��̾� ��ü�� �������Ѵ�. 

	m_pShader->OnPrepareRender(pd3dCommandList);

	CGameObject::Render(pd3dCommandList, pCamera);
}

void CPlayer::RenderWire(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera)
{
	//�÷��̾� �浹�ڽ��� �������Ѵ�.
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