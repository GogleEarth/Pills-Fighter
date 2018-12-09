#include "stdafx.h"
#include "GameObject.h"
#include "Shader.h"

CTexture::CTexture(int nTextures, UINT nTextureType, int nSamplers)
{
	m_nTextureType = nTextureType;
	m_nTextures = nTextures;

	if (m_nTextures > 0)
	{
		m_pRootArgumentInfos = new SRVROOTARGUMENTINFO[m_nTextures];
		m_ppd3dTextureUploadBuffers = new ID3D12Resource*[m_nTextures];
		m_ppd3dTextures = new ID3D12Resource*[m_nTextures];
	}

	m_nSamplers = nSamplers;
	if (m_nSamplers > 0) m_pd3dSamplerGpuDescriptorHandles = new D3D12_GPU_DESCRIPTOR_HANDLE[m_nSamplers];
}

CTexture::~CTexture()
{
	if (m_ppd3dTextures)
	{
		for (int i = 0; i < m_nTextures; i++) if (m_ppd3dTextures[i]) m_ppd3dTextures[i]->Release();
	}

	if (m_pRootArgumentInfos) delete[] m_pRootArgumentInfos;

	if (m_pd3dSamplerGpuDescriptorHandles) delete[] m_pd3dSamplerGpuDescriptorHandles;
}

void CTexture::SetRootArgument(int nIndex, UINT nRootParameterIndex, D3D12_GPU_DESCRIPTOR_HANDLE d3dSrvGpuDescriptorHandle)
{
	m_pRootArgumentInfos[nIndex].m_nRootParameterIndex = nRootParameterIndex;
	m_pRootArgumentInfos[nIndex].m_d3dSrvGpuDescriptorHandle = d3dSrvGpuDescriptorHandle;
}

void CTexture::SetSampler(int nIndex, D3D12_GPU_DESCRIPTOR_HANDLE d3dSamplerGpuDescriptorHandle)
{
	m_pd3dSamplerGpuDescriptorHandles[nIndex] = d3dSamplerGpuDescriptorHandle;
}

void CTexture::UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList)
{
	if (m_nTextureType == RESOURCE_TEXTURE2D_ARRAY)
	{
		pd3dCommandList->SetGraphicsRootDescriptorTable(m_pRootArgumentInfos[0].m_nRootParameterIndex, m_pRootArgumentInfos[0].m_d3dSrvGpuDescriptorHandle);
	}
	else
	{
		for (int i = 0; i < m_nTextures; i++)
		{
			pd3dCommandList->SetGraphicsRootDescriptorTable(m_pRootArgumentInfos[i].m_nRootParameterIndex, m_pRootArgumentInfos[i].m_d3dSrvGpuDescriptorHandle);
		}
	}
}

void CTexture::UpdateShaderVariable(ID3D12GraphicsCommandList *pd3dCommandList, int nIndex)
{
	pd3dCommandList->SetGraphicsRootDescriptorTable(m_pRootArgumentInfos[nIndex].m_nRootParameterIndex, m_pRootArgumentInfos[nIndex].m_d3dSrvGpuDescriptorHandle);
}

void CTexture::ReleaseUploadBuffers()
{
	if (m_ppd3dTextureUploadBuffers)
	{
		for (int i = 0; i < m_nTextures; i++) if (m_ppd3dTextureUploadBuffers[i]) m_ppd3dTextureUploadBuffers[i]->Release();
		delete[] m_ppd3dTextureUploadBuffers;
		m_ppd3dTextureUploadBuffers = NULL;
	}
}

void CTexture::ReleaseShaderVariables()
{
}

void CTexture::LoadTextureFromFile(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, wchar_t *pszFileName, UINT nIndex)
{
	m_ppd3dTextures[nIndex] = ::CreateTextureResourceFromFile(pd3dDevice, pd3dCommandList, pszFileName, &m_ppd3dTextureUploadBuffers[nIndex], D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
}

///////////////////////////////////////////////////////////////////////////////

CMaterial::CMaterial()
{
}

CMaterial::~CMaterial()
{
	if (m_pTexture) delete m_pTexture;
}

void CMaterial::SetTexture(CTexture *pTexture)
{
	m_pTexture = pTexture;
}

void CMaterial::UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList)
{
	if (m_pTexture) m_pTexture->UpdateShaderVariables(pd3dCommandList);
}

void CMaterial::ReleaseShaderVariables()
{
	if (m_pTexture) m_pTexture->ReleaseShaderVariables();
}

void CMaterial::ReleaseUploadBuffers()
{
	if (m_pTexture) m_pTexture->ReleaseUploadBuffers();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CGameObject::CGameObject()
{
	XMStoreFloat4x4(&m_xmf4x4World, XMMatrixIdentity());
	m_xmf3Position = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_xmf3PrevPosition = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_xmf3Right = XMFLOAT3(1.0f, 0.0f, 0.0f);
	m_xmf3Up = XMFLOAT3(0.0f, 1.0f, 0.0f);
	m_xmf3Look = XMFLOAT3(0.0f, 0.0f, 1.0f);
	m_xmf3Direction = XMFLOAT3(0.0f, 0.0f, 0.0f);

	m_fPitch = 0.0f;
	m_fRoll = 0.0f;
	m_fYaw = 0.0f;

	m_PPitch = 0.0f;
	m_PRoll = 0.0f;
	m_PYaw = 0.0f;
}

CGameObject::~CGameObject()
{
	if (m_xmOOBB) delete[] m_xmOOBB;
}

void CGameObject::SetWorldTransf(XMFLOAT4X4& xmf4x4World)
{ 
	m_xmf3Right = XMFLOAT3(xmf4x4World._11, xmf4x4World._12, xmf4x4World._13);
	m_xmf3Up = XMFLOAT3(xmf4x4World._21, xmf4x4World._22, xmf4x4World._23);
	m_xmf3Look = XMFLOAT3(xmf4x4World._31, xmf4x4World._32, xmf4x4World._33);
	m_xmf3Position = XMFLOAT3(xmf4x4World._41, xmf4x4World._42, xmf4x4World._43);
}

XMFLOAT4X4 CGameObject::GetWorldTransf()
{
	XMFLOAT4X4 xmf4x4World;
	xmf4x4World = XMFLOAT4X4{
		m_xmf3Right.x,		m_xmf3Right.y,		m_xmf3Right.z, 0.0f,
		m_xmf3Up.x,			m_xmf3Up.y,			m_xmf3Up.z, 0.0f,
		m_xmf3Look.x,		m_xmf3Look.y,		m_xmf3Look.z, 0.0f,
		m_xmf3Position.x,	m_xmf3Position.y,	m_xmf3Position.z, 0.0f,
	};

	return xmf4x4World;
}

void CGameObject::SetMesh(CMesh **ppMeshes, CCubeMesh **ppCubeMeshes, UINT nMeshes)
{
	m_ppMeshes = ppMeshes;
	m_ppCubeMeshes = ppCubeMeshes;
	m_nMeshes = nMeshes;

	if (m_xmOOBB) delete[] m_xmOOBB;

	m_xmOOBB = new BoundingOrientedBox[m_nMeshes];
	for (int i = 0; i < m_nMeshes; i++)
	{
		m_xmOOBB[i] = ppMeshes[i]->m_xmOOBB;
	}
}

void CGameObject::CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList)
{
	UINT ncbElementBytes = ((sizeof(CB_GAMEOBJECT_INFO) + 255) & ~255); //256의 배수
	m_pd3dcbGameObject = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes, 
		D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);

	m_pd3dcbGameObject->Map(0, NULL, (void **)&m_pcbMappedGameObject);
}

void CGameObject::UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList)
{
	XMStoreFloat4x4(&m_pcbMappedGameObject->m_xmf4x4World, XMMatrixTranspose(XMLoadFloat4x4(&m_xmf4x4World)));
	m_pcbMappedGameObject->m_nMaterial = m_ppMaterials[0]->m_nReflection;

	D3D12_GPU_VIRTUAL_ADDRESS d3dGpuVirtualAddress = m_pd3dcbGameObject->GetGPUVirtualAddress();
	pd3dCommandList->SetGraphicsRootConstantBufferView(0, d3dGpuVirtualAddress);
}

void CGameObject::ReleaseShaderVariables()
{
	if (m_pd3dcbGameObject)
	{
		m_pd3dcbGameObject->Unmap(0, NULL);
		m_pd3dcbGameObject->Release();
	}
}

void CGameObject::OnPrepareRender()
{
	m_xmf4x4World._11 = m_xmf3Right.x;
	m_xmf4x4World._12 = m_xmf3Right.y;
	m_xmf4x4World._13 = m_xmf3Right.z;
	m_xmf4x4World._21 = m_xmf3Up.x;
	m_xmf4x4World._22 = m_xmf3Up.y;
	m_xmf4x4World._23 = m_xmf3Up.z;
	m_xmf4x4World._31 = m_xmf3Look.x;
	m_xmf4x4World._32 = m_xmf3Look.y;
	m_xmf4x4World._33 = m_xmf3Look.z;
	m_xmf4x4World._41 = m_xmf3Position.x;
	m_xmf4x4World._42 = m_xmf3Position.y;
	m_xmf4x4World._43 = m_xmf3Position.z;

	XMMATRIX mtxRotate = XMMatrixRotationRollPitchYaw(XMConvertToRadians(m_PPitch), XMConvertToRadians(m_PYaw), XMConvertToRadians(m_PRoll));
	m_xmf4x4World = Matrix4x4::Multiply(mtxRotate, m_xmf4x4World);
}

void CGameObject::Animate(float fTimeElapsed)
{
	for (UINT i = 0; i < m_nMeshes; i++)
	{
		if (m_ppMeshes[i])
		{
			//XMMATRIX mtxRotate = XMMatrixRotationRollPitchYaw(XMConvertToRadians(m_fPitch), XMConvertToRadians(m_fYaw), XMConvertToRadians(m_fRoll));
			//XMFLOAT4X4 xmf4x4World = Matrix4x4::Multiply(mtxRotate, m_xmf4x4World);

			OnPrepareRender();

			m_ppMeshes[i]->m_xmOOBB.Transform(m_xmOOBB[i], XMLoadFloat4x4(&m_xmf4x4World));
			XMStoreFloat4(&m_xmOOBB[i].Orientation, XMQuaternionNormalize(XMLoadFloat4(&m_xmOOBB[i].Orientation)));
		}
	}
}

void CGameObject::Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera* pCamera, UINT nInstances)
{
	OnPrepareRender();

	if(nInstances <= 1) // 인스턴싱을 하지 않는 경우 ( 쉐이더 클래스에서 정보를 갱신하지 않음 )
		UpdateShaderVariables(pd3dCommandList);

	for (UINT i = 0; i < m_nMeshes; i++)
	{
		if (i < m_nMaterials)
		{
			if (m_ppMaterials)
			{
				if (m_ppMaterials[i]) m_ppMaterials[i]->UpdateShaderVariables(pd3dCommandList);
			}
		}

		if (m_ppMeshes)
		{
			if (m_ppMeshes[i]) m_ppMeshes[i]->Render(pd3dCommandList, nInstances);
		}

	}
}

void CGameObject::RenderWire(ID3D12GraphicsCommandList *pd3dCommandList, CCamera* pCamera, UINT nInstances)
{
	OnPrepareRender();

	if (nInstances <= 1) // 인스턴싱을 하지 않는 경우 ( 쉐이더 클래스에서 정보를 갱신하지 않음 )
		UpdateShaderVariables(pd3dCommandList);

	for (UINT i = 0; i < m_nMeshes; i++)
	{
		if (m_ppCubeMeshes)
		{
			if (m_ppCubeMeshes[i]) m_ppCubeMeshes[i]->Render(pd3dCommandList);
		}
	}
}

void CGameObject::ReleaseUploadBuffers()
{
	if (m_ppMeshes)
	{
		for (UINT i = 0; i < m_nMeshes; i++)
		{
			if (m_ppMeshes[i]) m_ppMeshes[i]->ReleaseUploadBuffers();
			if(m_ppCubeMeshes) if (m_ppCubeMeshes[i]) m_ppCubeMeshes[i]->ReleaseUploadBuffers();
		}
	}
}

void CGameObject::Rotate(float fPitch, float fYaw, float fRoll)
{
	if (fPitch != 0.0f)
	{
		XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3Right),
			XMConvertToRadians(fPitch));
		m_xmf3Look = Vector3::TransformNormal(m_xmf3Look, xmmtxRotate);
		m_xmf3Up = Vector3::TransformNormal(m_xmf3Up, xmmtxRotate);
	}
	if (fYaw != 0.0f)
	{
		XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3Up),
			XMConvertToRadians(fYaw));
		m_xmf3Look = Vector3::TransformNormal(m_xmf3Look, xmmtxRotate);
		m_xmf3Right = Vector3::TransformNormal(m_xmf3Right, xmmtxRotate);
	}
	if (fRoll != 0.0f)
	{
		XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3Look),
			XMConvertToRadians(fRoll));
		m_xmf3Right = Vector3::TransformNormal(m_xmf3Right, xmmtxRotate);
		m_xmf3Up = Vector3::TransformNormal(m_xmf3Up, xmmtxRotate);
	}

	m_xmf3Look = Vector3::Normalize(m_xmf3Look);
	m_xmf3Right = Vector3::CrossProduct(m_xmf3Up, m_xmf3Look, true);
	m_xmf3Up = Vector3::CrossProduct(m_xmf3Look, m_xmf3Right, true);
}

void CGameObject::SetPosition(float x, float y, float z)
{
	m_xmf3PrevPosition.x = m_xmf3Position.x;
	m_xmf3PrevPosition.y = m_xmf3Position.y;
	m_xmf3PrevPosition.z = m_xmf3Position.z;

	m_xmf3Position.x = x;
	m_xmf3Position.y = y;
	m_xmf3Position.z = z;
}

void CGameObject::SetPosition(XMFLOAT3& xmf3Position)
{
	SetPosition(xmf3Position.x, xmf3Position.y, xmf3Position.z);
}

//게임 객체를 로컬 x-축 방향으로 이동한다. 
void CGameObject::MoveStrafe(float fDistance)
{
	XMFLOAT3 xmf3Position = GetPosition();
	XMFLOAT3 xmf3Right = GetRight();
	xmf3Position = Vector3::Add(xmf3Position, xmf3Right, fDistance);
	CGameObject::SetPosition(xmf3Position);
}

//게임 객체를 로컬 y-축 방향으로 이동한다. 
void CGameObject::MoveUp(float fDistance)
{
	XMFLOAT3 xmf3Position = GetPosition();
	XMFLOAT3 xmf3Up = GetUp();
	xmf3Position = Vector3::Add(xmf3Position, xmf3Up, fDistance);
	CGameObject::SetPosition(xmf3Position);
}

//게임 객체를 로컬 z-축 방향으로 이동한다. 
void CGameObject::MoveForward(float fDistance)
{
	XMFLOAT3 xmf3Position = GetPosition();
	XMFLOAT3 xmf3Look = GetLook();
	xmf3Position = Vector3::Add(xmf3Position, xmf3Look, fDistance);
	CGameObject::SetPosition(xmf3Position);
}

//게임 객체를 정해진 방향으로 이동한다. 
void CGameObject::Move(XMFLOAT3 xmf3Direction, float fDistance)
{
	XMFLOAT3 xmf3Position = GetPosition();
	xmf3Position = Vector3::Add(xmf3Position, xmf3Direction, fDistance);
	CGameObject::SetPosition(xmf3Position);
}
///////////////////////////////////////////////////////////////////////////////////

RandomMoveObject::RandomMoveObject() : CGameObject()
{
	InitRandomRotate();

	m_Time = 5.0f;
	m_MovingSpeed = 10.0f;
}

RandomMoveObject::~RandomMoveObject()
{
}

void RandomMoveObject::InitRandomRotate()
{
	m_RotateAngle = (rand() % 360) + 45.0f;
	m_RotateSpeed = m_RotateAngle / ((float)(rand() % 2) + 1.0f);

	m_Angle = 0.0f;
	m_ElapsedTime = 0.0f;
}

void RandomMoveObject::Animate(float ElapsedTime)
{
	if (m_ElapsedTime >= m_Time)
	{
		InitRandomRotate();
	}
	else
	{
		if (m_Angle < m_RotateAngle)
		{
			m_Angle += m_RotateSpeed * ElapsedTime;

			if (m_Angle > m_RotateAngle) m_Angle = m_RotateAngle;
			else Rotate(0.0f, m_RotateSpeed * ElapsedTime, 0.0f);
		}
		else
			m_ElapsedTime += ElapsedTime;
	}

	MoveForward(m_MovingSpeed * ElapsedTime);

	CGameObject::Animate(ElapsedTime);
}

///////////////////////////////////////////////////////////////////////////////////

Bullet::Bullet() : CGameObject()
{
	m_ElapsedTime = 0;
	m_DurationTime = 3.0f;
	m_MovingSpeed = 500.0f;
	m_RotationSpeed = 1440.0f;
}

Bullet::~Bullet()
{
}

void Bullet::Animate(float ElapsedTime)
{
	if (m_ElapsedTime >= m_DurationTime)
	{
		DeleteObject();
	}
	else
	{
		CGameObject::Rotate(0.0f, 0.0f, m_RotationSpeed * ElapsedTime);
		MoveForward(m_MovingSpeed * ElapsedTime);

		m_ElapsedTime += ElapsedTime;
	}

	CGameObject::Animate(ElapsedTime);
}

////////////////////////////////////////////////////////////////////////////////////////////////

void CreateRobotObjectMesh(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, CMesh**& ppMeshes, CCubeMesh**& ppCubeMeshes, UINT& nMeshes)
{
	ppMeshes = new CMesh*[7];
	::ZeroMemory(ppMeshes, sizeof(CMesh*) * 7);
	ppCubeMeshes = new CCubeMesh*[7];
	::ZeroMemory(ppCubeMeshes, sizeof(CCubeMesh*) * 7);	

	XMFLOAT3 Extents;
	XMFLOAT3 Center;

	ppMeshes[0] = new CStandardMesh(pd3dDevice, pd3dCommandList, "./Resource/GM/Head/Head.FBX");
	Extents = ppMeshes[0]->GetExtents();
	Center = ppMeshes[0]->GetCenter();
	ppCubeMeshes[0] = new CCubeMesh(pd3dDevice, pd3dCommandList, Center, Extents.x, Extents.y, Extents.z);

	ppMeshes[1] = new CStandardMesh(pd3dDevice, pd3dCommandList, "./Resource/GM/Body/UpperBody.FBX");
	Extents = ppMeshes[1]->GetExtents();
	Center = ppMeshes[1]->GetCenter();
	ppCubeMeshes[1] = new CCubeMesh(pd3dDevice, pd3dCommandList, Center, Extents.x, Extents.y, Extents.z);

	ppMeshes[2] = new CStandardMesh(pd3dDevice, pd3dCommandList, "./Resource/GM/Body/LowerBody.FBX");
	Extents = ppMeshes[2]->GetExtents();
	Center = ppMeshes[2]->GetCenter();
	ppCubeMeshes[2] = new CCubeMesh(pd3dDevice, pd3dCommandList, Center, Extents.x, Extents.y, Extents.z);

	ppMeshes[3] = new CStandardMesh(pd3dDevice, pd3dCommandList, "./Resource/GM/Arm/Arm-Left.FBX");
	Extents = ppMeshes[3]->GetExtents();
	Center = ppMeshes[3]->GetCenter();
	ppCubeMeshes[3] = new CCubeMesh(pd3dDevice, pd3dCommandList, Center, Extents.x, Extents.y, Extents.z);

	ppMeshes[4] = new CStandardMesh(pd3dDevice, pd3dCommandList, "./Resource/GM/Arm/Arm-Right.FBX");
	Extents = ppMeshes[4]->GetExtents();
	Center = ppMeshes[4]->GetCenter();
	ppCubeMeshes[4] = new CCubeMesh(pd3dDevice, pd3dCommandList, Center, Extents.x, Extents.y, Extents.z);

	ppMeshes[5] = new CStandardMesh(pd3dDevice, pd3dCommandList, "./Resource/GM/Leg/Leg-Left.FBX");
	Extents = ppMeshes[5]->GetExtents();
	Center = ppMeshes[5]->GetCenter();
	ppCubeMeshes[5] = new CCubeMesh(pd3dDevice, pd3dCommandList, Center, Extents.x, Extents.y, Extents.z);

	ppMeshes[6] = new CStandardMesh(pd3dDevice, pd3dCommandList, "./Resource/GM/Leg/Leg-Right.FBX");
	Extents = ppMeshes[6]->GetExtents();
	Center = ppMeshes[6]->GetCenter();
	ppCubeMeshes[6] = new CCubeMesh(pd3dDevice, pd3dCommandList, Center, Extents.x, Extents.y, Extents.z);

	nMeshes = 7;
}

void CreateRobotObjectTexture(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, CTexture**& ppTextures, UINT& nTextures)
{
	ppTextures = new CTexture*[7];
	::ZeroMemory(ppTextures, sizeof(CTexture*) * 7);

	for (int i = 0; i < 7; i++)
		ppTextures[i] = new CTexture(1, RESOURCE_TEXTURE2D, 0);
	ppTextures[0]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"./Resource/GM/Head/Head.dds", 0);
	ppTextures[1]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"./Resource/GM/Body/UpperBody.dds", 0);
	ppTextures[2]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"./Resource/GM/Body/LowerBody.dds", 0);
	ppTextures[3]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"./Resource/GM/Arm/Arm.dds", 0);
	ppTextures[4]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"./Resource/GM/Arm/Arm.dds", 0);
	ppTextures[5]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"./Resource/GM/Leg/Leg.dds", 0);
	ppTextures[6]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"./Resource/GM/Leg/Leg.dds", 0);

	nTextures = 7;
}

void CreateRobotObjectShader(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, CTexture**& ppTextures, CShader* pShader)
{
	pShader->CreateSrvDescriptorHeaps(pd3dDevice, pd3dCommandList, 7);

	for (int i = 0; i < 7; i++)
		pShader->CreateShaderResourceViews(pd3dDevice, pd3dCommandList, ppTextures[i], 2, false);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
CHeightMapTerrain::CHeightMapTerrain(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature, LPCTSTR pFileName, int nWidth, int nLength, int nBlockWidth, int nBlockLength, XMFLOAT3 xmf3Scale, XMFLOAT4 xmf4Color) : CGameObject()
{
	//m_xmf3Position = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_xmf3Position = XMFLOAT3(-(nWidth *xmf3Scale.x/2), 0.0f, -(nLength *xmf3Scale.z / 2));

	m_nWidth = nWidth;
	m_nLength = nLength;

	int cxQuadsPerBlock = nBlockWidth - 1;
	int czQuadsPerBlock = nBlockLength - 1;

	m_xmf3Scale = xmf3Scale;

	m_pHeightMapImage = new CHeightMapImage(pFileName, nWidth, nLength, xmf3Scale);

	long cxBlocks = (m_nWidth - 1) / cxQuadsPerBlock;
	long czBlocks = (m_nLength - 1) / czQuadsPerBlock;

	m_nMeshes = cxBlocks * czBlocks;
	m_ppMeshes = new CMesh*[m_nMeshes];
	for (int i = 0; i < m_nMeshes; i++)	m_ppMeshes[i] = NULL;

	CHeightMapGridMesh *pHeightMapGridMesh = NULL;
	for (int z = 0, zStart = 0; z < czBlocks; z++)
	{
		for (int x = 0, xStart = 0; x < cxBlocks; x++)
		{
			xStart = x * (nBlockWidth - 1);
			zStart = z * (nBlockLength - 1);
			pHeightMapGridMesh = new CHeightMapGridMesh(pd3dDevice, pd3dCommandList, xStart, zStart, nBlockWidth, nBlockLength, xmf3Scale, xmf4Color, m_pHeightMapImage);
			m_ppMeshes[x + (z*cxBlocks)] = pHeightMapGridMesh;
		}
	}

	CreateShaderVariables(pd3dDevice, pd3dCommandList);

	CTexture *pTerrainTexture = new CTexture(1, RESOURCE_TEXTURE2D, 0);
	pTerrainTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"./Resource/Tile.dds", 0);

	m_pTerrainShader = new CTerrainShader();
	m_pTerrainShader->CreateShader(pd3dDevice, pd3dGraphicsRootSignature);
	m_pTerrainShader->CreateSrvDescriptorHeaps(pd3dDevice, pd3dCommandList, 1);
	m_pTerrainShader->CreateShaderResourceViews(pd3dDevice, pd3dCommandList, pTerrainTexture, 7, false);
	
	CMaterial **ppTerrainMaterial = new CMaterial*[1];
	ppTerrainMaterial[0] = new CMaterial();
	ppTerrainMaterial[0]->SetTexture(pTerrainTexture);
	ppTerrainMaterial[0]->m_nReflection = 0;

	SetMaterial(ppTerrainMaterial, 1);
	
	std::cout << m_xmf3Position.x << ", " << m_xmf3Position.y << ", " << m_xmf3Position.z << std::endl;
}

CHeightMapTerrain::~CHeightMapTerrain()
{
	ReleaseShaderVariables();

	if (m_ppMeshes)
	{
		for (UINT j = 0; j < m_nMeshes; j++)
		{
			if (m_ppMeshes[j]) delete m_ppMeshes[j];
		}

		delete[] m_ppMeshes;
	}

	if (m_pTerrainShader) delete m_pTerrainShader;

	if (m_pHeightMapImage) delete m_pHeightMapImage;
}

void CHeightMapTerrain::ReleaseUploadBuffers()
{
	if (m_pTerrainShader) m_pTerrainShader->ReleaseUploadBuffers();

	CGameObject::ReleaseUploadBuffers();
}

void CHeightMapTerrain::Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera, UINT nInstances)
{
	//플레이어 객체를 렌더링한다. 
	m_pTerrainShader->OnPrepareRender(pd3dCommandList);

	CGameObject::Render(pd3dCommandList, pCamera);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 
CSkyBox::CSkyBox(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature) : CGameObject()
{
	m_nMeshes = 1;
	m_ppMeshes = new CMesh*[m_nMeshes];
	CSkyBoxMesh *pSkyBoxMesh = new CSkyBoxMesh(pd3dDevice, pd3dCommandList, 20.0f, 20.0f, 2.0f);
	m_ppMeshes[0] = pSkyBoxMesh;

	CreateShaderVariables(pd3dDevice, pd3dCommandList);

	CTexture *pSkyBoxTexture = new CTexture(1, RESOURCE_TEXTURE_CUBE, 0);
	//pSkyBoxTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"./Resource/Skybox/SkyBox_0.dds", 0);
	pSkyBoxTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"./Resource/Skybox/SkyBox_1.dds", 0);

	m_pSkyboxShader = new CSkyBoxShader();
	m_pSkyboxShader->CreateShader(pd3dDevice, pd3dGraphicsRootSignature);
	m_pSkyboxShader->CreateShaderVariables(pd3dDevice, pd3dCommandList);

	m_pSkyboxShader->CreateSrvDescriptorHeaps(pd3dDevice, pd3dCommandList, 1);
	m_pSkyboxShader->CreateShaderResourceViews(pd3dDevice, pd3dCommandList, pSkyBoxTexture, 8, false);

	CMaterial **ppSkyBoxMaterial = new CMaterial*[1];
	ppSkyBoxMaterial[0] = new CMaterial();
	ppSkyBoxMaterial[0]->SetTexture(pSkyBoxTexture);

	SetMaterial(ppSkyBoxMaterial, 1);
}

CSkyBox::~CSkyBox()
{
}

void CSkyBox::Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera, UINT nInstances)
{
	m_pSkyboxShader->OnPrepareRender(pd3dCommandList);

	XMFLOAT3 xmf3CameraPos = pCamera->GetPosition();
	SetPosition(xmf3CameraPos.x, xmf3CameraPos.y, xmf3CameraPos.z);

	CGameObject::Render(pd3dCommandList, pCamera);
}



