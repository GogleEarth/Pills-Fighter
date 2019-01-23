#include "stdafx.h"
#include "Camera.h"
#include "Player.h"


CCamera::CCamera()
{
	m_xmf4x4View = Matrix4x4::Identity();
	m_xmf4x4Projection = Matrix4x4::Identity();
	m_d3dViewport = { 0, 0, FRAME_BUFFER_WIDTH , FRAME_BUFFER_HEIGHT, 0.0f, 1.0f };
	m_d3dScissorRect = { 0, 0, FRAME_BUFFER_WIDTH , FRAME_BUFFER_HEIGHT };
	m_xmf3Position = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_xmf3Right = XMFLOAT3(1.0f, 0.0f, 0.0f);
	m_xmf3Look = XMFLOAT3(0.0f, 0.0f, 1.0f);
	m_xmf3Up = XMFLOAT3(0.0f, 1.0f, 0.0f);
	m_fPitch = 0.0f;
	m_fRoll = 0.0f;
	m_fYaw = 0.0f;
	m_xmf3Offset = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_fTimeLag = 0.0f;
	m_pPlayer = NULL;
}

CCamera::CCamera(CCamera *pCamera)
{
	if (pCamera)
	{
		//카메라가 이미 있으면 기존 카메라의 정보를 새로운 카메라에 복사한다. 
		*this = *pCamera;
	}
	else
	{
		//카메라가 없으면 기본 정보를 설정한다. 
		m_xmf4x4View = Matrix4x4::Identity();
		m_xmf4x4Projection = Matrix4x4::Identity();
		m_d3dViewport = { 0, 0, FRAME_BUFFER_WIDTH , FRAME_BUFFER_HEIGHT, 0.0f, 1.0f };
		m_d3dScissorRect = { 0, 0, FRAME_BUFFER_WIDTH , FRAME_BUFFER_HEIGHT };
		m_xmf3Position = XMFLOAT3(0.0f, 0.0f, 0.0f);
		m_xmf3Right = XMFLOAT3(1.0f, 0.0f, 0.0f);
		m_xmf3Look = XMFLOAT3(0.0f, 0.0f, 1.0f);
		m_xmf3Up = XMFLOAT3(0.0f, 1.0f, 0.0f);
		m_fPitch = 0.0f;
		m_fRoll = 0.0f;
		m_fYaw = 0.0f;
		m_xmf3Offset = XMFLOAT3(0.0f, 0.0f, 0.0f);
		m_fTimeLag = 0.0f;
		m_pPlayer = NULL;
	}
}

CCamera::~CCamera()
{
}

void CCamera::SetViewport(int xTopLeft, int yTopLeft, int nWidth, int nHeight, float fMinZ, float fMaxZ)
{
	m_d3dViewport.TopLeftX = float(xTopLeft);
	m_d3dViewport.TopLeftY = float(yTopLeft);
	m_d3dViewport.Width = float(nWidth);
	m_d3dViewport.Height = float(nHeight);
	m_d3dViewport.MinDepth = fMinZ;
	m_d3dViewport.MaxDepth = fMaxZ;
}

void CCamera::SetScissorRect(LONG xLeft, LONG yTop, LONG xRight, LONG yBottom)
{
	m_d3dScissorRect.left = xLeft;
	m_d3dScissorRect.top = yTop;
	m_d3dScissorRect.right = xRight;
	m_d3dScissorRect.bottom = yBottom;
}

void CCamera::GenerateProjectionMatrix(float fNearPlaneDistance, float fFarPlaneDistance, float fAspectRatio, float fFOVAngle)
{
	m_xmf4x4Projection = Matrix4x4::PerspectiveFovLH(XMConvertToRadians(fFOVAngle),	fAspectRatio, fNearPlaneDistance, fFarPlaneDistance);
}

void CCamera::GenerateViewMatrix()
{
	//카메라의 z-축을 기준으로 카메라의 좌표축들이 직교하도록 카메라 변환 행렬을 갱신한다. 
	//카메라의 z-축 벡터를 정규화한다. 
	m_xmf3Look = Vector3::Normalize(m_xmf3Look);

	//카메라의 z-축과 y-축에 수직인 벡터를 x-축으로 설정한다. 
	m_xmf3Right = Vector3::CrossProduct(m_xmf3Up, m_xmf3Look, true);

	//카메라의 z-축과 x-축에 수직인 벡터를 y-축으로 설정한다. 
	m_xmf3Up = Vector3::CrossProduct(m_xmf3Look, m_xmf3Right, true);

	m_xmf4x4View._11 = m_xmf3Right.x; m_xmf4x4View._12 = m_xmf3Up.x; m_xmf4x4View._13 =	m_xmf3Look.x;
	m_xmf4x4View._21 = m_xmf3Right.y; m_xmf4x4View._22 = m_xmf3Up.y; m_xmf4x4View._23 =	m_xmf3Look.y;
	m_xmf4x4View._31 = m_xmf3Right.z; m_xmf4x4View._32 = m_xmf3Up.z; m_xmf4x4View._33 =	m_xmf3Look.z;
	m_xmf4x4View._41 = -Vector3::DotProduct(m_xmf3Position, m_xmf3Right);
	m_xmf4x4View._42 = -Vector3::DotProduct(m_xmf3Position, m_xmf3Up);
	m_xmf4x4View._43 = -Vector3::DotProduct(m_xmf3Position, m_xmf3Look);
}

void CCamera::CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList)
{
	UINT ncbElementBytes = ((sizeof(VS_CB_CAMERA_INFO) + 255) & ~255); //256의 배수
	m_pd3dcbCamera = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);

	m_pd3dcbCamera->Map(0, NULL, (void **)&m_pcbMappedCamera);
}

void CCamera::UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList)
{
	XMStoreFloat4x4(&m_pcbMappedCamera->m_xmf4x4View, XMMatrixTranspose(XMLoadFloat4x4(&m_xmf4x4View)));
	XMStoreFloat4x4(&m_pcbMappedCamera->m_xmf4x4Projection, XMMatrixTranspose(XMLoadFloat4x4(&m_xmf4x4Projection)));
	::memcpy(&m_pcbMappedCamera->m_xmf3Position, &m_xmf3Position, sizeof(XMFLOAT3));

	D3D12_GPU_VIRTUAL_ADDRESS d3dGpuVirtualAddress = m_pd3dcbCamera->GetGPUVirtualAddress();
	pd3dCommandList->SetGraphicsRootConstantBufferView(1, d3dGpuVirtualAddress);
}

void CCamera::ReleaseShaderVariables()
{
	if (m_pd3dcbCamera)
	{
		m_pd3dcbCamera->Unmap(0, NULL);
		m_pd3dcbCamera->Release();
	}
}

void CCamera::SetViewportsAndScissorRects(ID3D12GraphicsCommandList *pd3dCommandList)
{
	pd3dCommandList->RSSetViewports(1, &m_d3dViewport);
	pd3dCommandList->RSSetScissorRects(1, &m_d3dScissorRect);
}

void CCamera::Rotate(float x, float y, float z)
{
	if (!IsZero(x))
	{
		m_fPitch += x;
		if (m_fPitch > +15.0f) m_fPitch = +15.0f;
		
		if (m_fPitch < -75.0f) m_fPitch = -75.0f;
	}

	if (!IsZero(y))
	{
		m_fYaw += y;
		if (m_fYaw > 360.0f) m_fYaw -= 360.0f;
		if (m_fYaw < 0.0f) m_fYaw += 360.0f;
	}
}

void CCamera::Update(float fTimeElapsed)
{
	if (m_pPlayer)
	{
		m_xmf3Right = XMFLOAT3(1.0f, 0.0f, 0.0f);
		m_xmf3Up = XMFLOAT3(0.0f, 1.0f, 0.0f);

		XMFLOAT4X4 xmf4x4Rotate = Matrix4x4::Identity();
		XMFLOAT4X4 xmf4x4Rotate2 = Matrix4x4::Identity();

		XMStoreFloat4x4(&xmf4x4Rotate, XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3Right), XMConvertToRadians(m_fPitch)));
		XMStoreFloat4x4(&xmf4x4Rotate2, XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3Up), XMConvertToRadians(m_fYaw)));
		xmf4x4Rotate = Matrix4x4::Multiply(xmf4x4Rotate, xmf4x4Rotate2);

		XMFLOAT4X4 xmf4x4Position = Matrix4x4::Identity();
		xmf4x4Position._41 = m_xmf3Offset.x;
		xmf4x4Position._42 = m_xmf3Offset.y;
		xmf4x4Position._43 = m_xmf3Offset.z;

		xmf4x4Rotate = Matrix4x4::Multiply(XMLoadFloat4x4(&xmf4x4Position), xmf4x4Rotate);
		m_xmf3Right = XMFLOAT3(xmf4x4Rotate._11, xmf4x4Rotate._12, xmf4x4Rotate._13);
		m_xmf3Up = XMFLOAT3(xmf4x4Rotate._21, xmf4x4Rotate._22, xmf4x4Rotate._23);
		m_xmf3Look = XMFLOAT3(xmf4x4Rotate._31, xmf4x4Rotate._32, xmf4x4Rotate._33);

		XMFLOAT3 xmf3PlayerPos = m_pPlayer->GetPosition();
		m_xmf3Position.x = xmf4x4Rotate._41 + xmf3PlayerPos.x;
		m_xmf3Position.y = xmf4x4Rotate._42 + xmf3PlayerPos.y;
		m_xmf3Position.z = xmf4x4Rotate._43 + xmf3PlayerPos.z;
	}
}

void CCamera::SetLookAt(XMFLOAT3& xmf3LookAt)
{
	m_xmf3Look = Vector3::Normalize(Vector3::Subtract(xmf3LookAt, m_xmf3Position));
	m_xmf3Up = Vector3::CrossProduct(m_xmf3Look, m_xmf3Right, true);
	m_xmf3Right = Vector3::CrossProduct(m_xmf3Look, m_xmf3Up, true);
}

void CCamera::OnPrepareRender()
{
	XMFLOAT3 xmf3LookAt = Vector3::Add(m_pPlayer->GetPosition(), XMFLOAT3(0.0f, 20.0f, 0.0f));
	SetLookAt(xmf3LookAt);

	GenerateViewMatrix();
}