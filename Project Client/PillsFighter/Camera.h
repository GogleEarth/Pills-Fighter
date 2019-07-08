#pragma once

#define ASPECT_RATIO (float(FRAME_BUFFER_WIDTH) / float(FRAME_BUFFER_HEIGHT))

class CPlayer;

struct VS_CB_CAMERA_INFO
{
	XMFLOAT4X4	m_xmf4x4ViewProjection;
	XMFLOAT3	m_xmf3Position;
};

class CCamera
{
protected:
	XMFLOAT3 						m_xmf3Position;

	XMFLOAT3 						m_xmf3Right;
	XMFLOAT3 						m_xmf3Up;
	XMFLOAT3 						m_xmf3Look;

	float 							m_fPitch;
	float 							m_fRoll;
	float 							m_fYaw;

	XMFLOAT3 						m_xmf3Offset;
	float 							m_fTimeLag;

	XMFLOAT4X4 						m_xmf4x4View;
	XMFLOAT4X4 						m_xmf4x4Projection;
	XMFLOAT4X4						m_xmf4x4ViewProjection;

	CPlayer							*m_pPlayer;

	ID3D12Resource					*m_pd3dcbCamera;
	VS_CB_CAMERA_INFO				*m_pcbMappedCamera;

public:
	CCamera();
	CCamera(CCamera *pCamera);
	virtual ~CCamera();

	virtual void CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void ReleaseShaderVariables();
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList);

	virtual void GenerateViewMatrix();
	virtual void GenerateProjectionMatrix(float fNearPlaneDistance, float fFarPlaneDistance, float fAspectRatio, float fFOVAngle);
	virtual void GenerateOrthogonalMatrix(float fWidth, float fHeight, float fNear, float fFar);

	void SetPlayer(CPlayer *pPlayer) { m_pPlayer = pPlayer; }
	CPlayer *GetPlayer() { return(m_pPlayer); }

	void SetPosition(XMFLOAT3 xmf3Position) { m_xmf3Position = xmf3Position; }
	XMFLOAT3& GetPosition() { return(m_xmf3Position); }

	XMFLOAT3& GetRightVector() { return(m_xmf3Right); }
	XMFLOAT3& GetUpVector() { return(m_xmf3Up); }
	XMFLOAT3& GetLookVector() { return(m_xmf3Look); }

	void SetRight(XMFLOAT3 xmf3Vector) { m_xmf3Right = xmf3Vector; }
	void SetUp(XMFLOAT3 xmf3Vector) { m_xmf3Up = xmf3Vector; }
	void SetLook(XMFLOAT3 xmf3Vector) { m_xmf3Look = xmf3Vector; }

	float& GetPitch() { return(m_fPitch); }
	float& GetRoll() { return(m_fRoll); }
	float& GetYaw() { return(m_fYaw); }

	void SetOffset(XMFLOAT3 xmf3Offset) { m_xmf3Offset = xmf3Offset; }
	XMFLOAT3& GetOffset() { return(m_xmf3Offset); }

	void SetTimeLag(float fTimeLag) { m_fTimeLag = fTimeLag; }
	float GetTimeLag() { return(m_fTimeLag); }

	XMFLOAT4X4 GetViewMatrix() { return(m_xmf4x4View); }
	XMFLOAT4X4 GetProjectionMatrix() { return(m_xmf4x4Projection); }
	XMFLOAT4X4 GetViewProjMatrix() { return(m_xmf4x4ViewProjection); }

	virtual void Move(const XMFLOAT3& xmf3Shift) { m_xmf3Position.x += xmf3Shift.x; m_xmf3Position.y += xmf3Shift.y; m_xmf3Position.z += xmf3Shift.z; }

	virtual void Rotate(float fPitch = 0.0f, float fYaw = 0.0f, float fRoll = 0.0f);

	virtual void Update(float fTimeElapsed);
	virtual void UpdateForMinimap(XMFLOAT3& xmf3LookAt);

	virtual void SetLookAt(XMFLOAT3& xmf3LookAt);
};
