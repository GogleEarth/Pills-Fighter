#include "stdafx.h"
#include "Camera.h"
#include "Player.h"
#include "Weapon.h"

CCamera::CCamera()
{
	m_xmf3Position = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_xmf3Right = XMFLOAT3(1.0f, 0.0f, 0.0f);
	m_xmf3Up = XMFLOAT3(0.0f, 1.0f, 0.0f);
	m_xmf3Look = XMFLOAT3(0.0f, 0.0f, 1.0f);

	m_fPitch = 0.0f;
	m_fRoll = 0.0f;
	m_fYaw = 0.0f;

	m_xmf3Offset = XMFLOAT3(0.0f, 0.0f, 0.0f);

	m_xmf4x4View = Matrix4x4::Identity();
	m_xmf4x4Projection = Matrix4x4::Identity();

	m_pPlayer = NULL;

	m_pd3dcbCamera = NULL;
	m_pcbMappedCamera = NULL;
}

CCamera::CCamera(CCamera *pCamera)
{
	if (pCamera)
	{
		*this = *pCamera;
	}
	else
	{
		m_xmf3Position = XMFLOAT3(0.0f, 0.0f, 0.0f);
		m_xmf3Right = XMFLOAT3(1.0f, 0.0f, 0.0f);
		m_xmf3Up = XMFLOAT3(0.0f, 1.0f, 0.0f);
		m_xmf3Look = XMFLOAT3(0.0f, 0.0f, 1.0f);

		m_fPitch = 0.0f;
		m_fRoll = 0.0f;
		m_fYaw = 0.0f;

		m_xmf3Offset = XMFLOAT3(0.0f, 0.0f, 0.0f);

		m_xmf4x4View = Matrix4x4::Identity();
		m_xmf4x4Projection = Matrix4x4::Identity();

		m_pPlayer = NULL;

		m_pd3dcbCamera = NULL;
		m_pcbMappedCamera = NULL;
	}
}

CCamera::~CCamera()
{
}

void CCamera::SetViewPort(float fLeft, float fBottom, float fWidth, float fHeight, float fMinDepth, float fMaxDepth)
{
	m_d3dViewport = { fLeft, fBottom, fWidth, fHeight, fMinDepth, fMaxDepth };
}

void CCamera::SetScissorRect(int nLeft, int nBottom, int nRight, int nTop)
{
	m_d3dScissorRect = { nLeft, nBottom, nRight, nTop };
}

void CCamera::GenerateProjectionMatrix(float fNearPlaneDistance, float fFarPlaneDistance, float fAspectRatio, float fFOVAngle)
{
	m_fNearPlaneDistance = fNearPlaneDistance;
	m_fFarPlaneDistance = fFarPlaneDistance;
	m_fAspectRatio = fAspectRatio;
	m_fFOVAngle = fFOVAngle;

	m_xmf4x4Projection = Matrix4x4::PerspectiveFovLH(XMConvertToRadians(fFOVAngle),	fAspectRatio, fNearPlaneDistance, fFarPlaneDistance);
}

void CCamera::GenerateOrthogonalMatrix(float fWidth, float fHeight, float fNear, float fFar)
{
	m_xmf4x4Projection = Matrix4x4::OrthogonalFovLH(fWidth, fHeight, fNear, fFar);
}

void CCamera::ZoomIn(float fFOV)
{
	m_bZoomIn = true;
	m_xmf4x4Projection = Matrix4x4::PerspectiveFovLH(XMConvertToRadians(fFOV), m_fAspectRatio, m_fNearPlaneDistance, m_fFarPlaneDistance);
}

void CCamera::ZoomOut()
{
	m_bZoomIn = false;
	m_xmf4x4Projection = Matrix4x4::PerspectiveFovLH(XMConvertToRadians(m_fFOVAngle), m_fAspectRatio, m_fNearPlaneDistance, m_fFarPlaneDistance);
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

	XMFLOAT3 xmf3Position = m_xmf3Position;

	XMFLOAT3 xmf3XMove = Vector3::ScalarProduct(m_xmf3Right, m_fShakeScale, false);
	XMFLOAT3 xmf3YMove = Vector3::ScalarProduct(m_xmf3Up, m_fShakeScale, false);
	xmf3Position = Vector3::Add(xmf3Position, xmf3XMove);
	xmf3Position = Vector3::Add(xmf3Position, xmf3YMove);

	m_xmf4x4View._41 = -Vector3::DotProduct(xmf3Position, m_xmf3Right);
	m_xmf4x4View._42 = -Vector3::DotProduct(xmf3Position, m_xmf3Up);
	m_xmf4x4View._43 = -Vector3::DotProduct(xmf3Position, m_xmf3Look);

	m_xmf4x4ViewProjection = Matrix4x4::Multiply(m_xmf4x4View, m_xmf4x4Projection);
}

void CCamera::CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList)
{
	UINT ncbElementBytes = ((sizeof(VS_CB_CAMERA_INFO) + 255) & ~255); //256의 배수
	m_pd3dcbCamera = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);

	m_pd3dcbCamera->Map(0, NULL, (void **)&m_pcbMappedCamera);
}

void CCamera::UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList)
{
	XMStoreFloat4x4(&m_pcbMappedCamera->m_xmf4x4ViewProjection, XMMatrixTranspose(XMLoadFloat4x4(&m_xmf4x4ViewProjection)));
	::memcpy(&m_pcbMappedCamera->m_xmf3Position, &m_xmf3Position, sizeof(XMFLOAT3));

	D3D12_GPU_VIRTUAL_ADDRESS d3dGpuVirtualAddress = m_pd3dcbCamera->GetGPUVirtualAddress();
	pd3dCommandList->SetGraphicsRootConstantBufferView(ROOT_PARAMETER_INDEX_CAMERA, d3dGpuVirtualAddress);
}

void CCamera::OnPrepareRender(ID3D12GraphicsCommandList *pd3dCommandList)
{
	UpdateShaderVariables(pd3dCommandList);

	pd3dCommandList->RSSetViewports(1, &m_d3dViewport);
	pd3dCommandList->RSSetScissorRects(1, &m_d3dScissorRect);
}

void CCamera::ReleaseShaderVariables()
{
	if (m_pd3dcbCamera)
	{
		m_pd3dcbCamera->Unmap(0, NULL);
		m_pd3dcbCamera->Release();

		m_pd3dcbCamera = NULL;
	}
}

void CCamera::CameraReset()
{
	m_fPitch = m_fYaw = m_fRoll = 0.0f;
}

void CCamera::Rotate(float x, float y, float z)
{
	if (!IsZero(x))
	{
		m_fPitch += x;
		if (m_fPitch > +20.0f) m_fPitch = +20.0f;
		
		if (m_fPitch < -80.0f) m_fPitch = -80.0f;
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

		if (m_bShake)
		{
			m_fShakeTime += fTimeElapsed;
			float s = sin(SK_CYCLE * XM_2PI * m_fShakeTime);
			float p = pow(0.5f, m_fShakeTime * SK_AMP);
			m_fShakeScale = s * p * SK_SCALE;

			if (m_fShakeTime > 0.25f)
			{
				m_bShake = false;
				m_fShakeTime = 0.0f;
				m_fShakeScale = 0.0f;
			}
		}
	}
}

void CCamera::SetLookAt(XMFLOAT3& xmf3LookAt)
{
	m_xmf3Look = Vector3::Normalize(Vector3::Subtract(xmf3LookAt, m_xmf3Position));
	m_xmf3Up = Vector3::CrossProduct(m_xmf3Look, m_xmf3Right, true);
	m_xmf3Right = Vector3::CrossProduct(m_xmf3Look, m_xmf3Up, true);
}


void CCamera::UpdateForMinimap(XMFLOAT3& xmf3LookAt)
{
	XMFLOAT3 xmf3PlayerPos = m_pPlayer->GetPosition();
	m_xmf3Position.x = xmf3PlayerPos.x;
	m_xmf3Position.z = xmf3PlayerPos.z;

	if (m_pPlayer)
	{
		m_xmf3Up = Vector3::Normalize(xmf3LookAt);
		m_xmf3Right = Vector3::CrossProduct(m_xmf3Look, m_xmf3Up, true);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//

CLightCamera::CLightCamera() : CCamera()
{

}

CLightCamera::~CLightCamera()
{

}

void CLightCamera::CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList)
{
	UINT ncbElementBytes = ((sizeof(VS_CB_LIGHT_CAMERA_INFO) + 255) & ~255); //256의 배수
	m_pd3dcbLightCamera = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);

	m_pd3dcbLightCamera->Map(0, NULL, (void **)&m_pcbMappedLightCamera);
}

void CLightCamera::UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList)
{
	XMFLOAT4X4 T(
		0.5f, 0.0f, 0.0f, 0.0f, 
		0.0f, -0.5f, 0.0f, 0.0f, 
		0.0f, 0.0f, 1.0f, 0.0f, 
		0.5f, 0.5f, 0.0f, 1.0f);

	XMFLOAT4X4 xmf4x4ShadowTransform = Matrix4x4::Multiply(m_xmf4x4ViewProjection, T);

	XMStoreFloat4x4(&m_pcbMappedLightCamera->m_xmf4x4ViewProjection, XMMatrixTranspose(XMLoadFloat4x4(&m_xmf4x4ViewProjection)));
	XMStoreFloat4x4(&m_pcbMappedLightCamera->m_xmf4x4ShadowTransform, XMMatrixTranspose(XMLoadFloat4x4(&xmf4x4ShadowTransform)));

	D3D12_GPU_VIRTUAL_ADDRESS d3dGpuVirtualAddress = m_pd3dcbLightCamera->GetGPUVirtualAddress();
	pd3dCommandList->SetGraphicsRootConstantBufferView(ROOT_PARAMETER_INDEX_LIGHT_CAMERA_INFO, d3dGpuVirtualAddress);
}

void CLightCamera::ReleaseShaderVariables()
{
	if (m_pd3dcbLightCamera)
	{
		m_pd3dcbLightCamera->Unmap(0, NULL);
		m_pd3dcbLightCamera->Release();
		
		m_pd3dcbLightCamera = NULL;
	}
}