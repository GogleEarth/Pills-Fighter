#pragma once

#define ASPECT_RATIO (float(FRAME_BUFFER_WIDTH) / float(FRAME_BUFFER_HEIGHT))
#define SK_CYCLE 15.0f
#define SK_SCALE 0.5f
#define SK_AMP 2.0f

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

	XMFLOAT4X4 						m_xmf4x4View;
	XMFLOAT4X4 						m_xmf4x4Projection;
	XMFLOAT4X4						m_xmf4x4ViewProjection;

	CPlayer							*m_pPlayer;

	ID3D12Resource					*m_pd3dcbCamera;
	VS_CB_CAMERA_INFO				*m_pcbMappedCamera;

	D3D12_VIEWPORT					m_d3dViewport;
	D3D12_RECT						m_d3dScissorRect;

	bool							m_bZoomIn = false;
	float							m_fNearPlaneDistance;
	float							m_fFarPlaneDistance;
	float							m_fAspectRatio;
	float							m_fFOVAngle;
	
	bool							m_bShake = false;
	float							m_fShakeScale = 0.0f;
	float							m_fShakeTime = 0.0f;

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
	virtual void SetViewPort(float fLeft, float fBottom, float fWidth, float fHeight, float fMinDepth = 0.0f, float fMaxDepth = 1.0f);
	virtual void SetScissorRect(int nLeft, int nBottom, int nRight, int nTop);
	virtual void OnPrepareRender(ID3D12GraphicsCommandList *pd3dCommandList);

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

	XMFLOAT4X4 GetViewMatrix() { return(m_xmf4x4View); }
	XMFLOAT4X4 GetProjectionMatrix() { return(m_xmf4x4Projection); }
	XMFLOAT4X4 GetViewProjMatrix() { return(m_xmf4x4ViewProjection); }

	virtual void Move(const XMFLOAT3& xmf3Shift) { m_xmf3Position.x += xmf3Shift.x; m_xmf3Position.y += xmf3Shift.y; m_xmf3Position.z += xmf3Shift.z; }

	virtual void Rotate(float fPitch = 0.0f, float fYaw = 0.0f, float fRoll = 0.0f);
	virtual void CameraReset();

	virtual void Update(float fTimeElapsed);
	virtual void UpdateForMinimap(XMFLOAT3& xmf3LookAt);

	virtual void SetLookAt(XMFLOAT3& xmf3LookAt);

	void ZoomIn(float fFOV);
	void ZoomOut();
	void SetShake() { m_bShake = true; };
};

////////////////////////////////////////////////////////////////////////////////////////////////////
//

struct VS_CB_LIGHT_CAMERA_INFO
{
	XMFLOAT4X4	m_xmf4x4ViewProjection;
	XMFLOAT4X4	m_xmf4x4ShadowTransform;
};

class CLightCamera : public CCamera
{
protected:	
	ID3D12Resource					*m_pd3dcbLightCamera;
	VS_CB_LIGHT_CAMERA_INFO			*m_pcbMappedLightCamera;

public:
	CLightCamera();
	virtual ~CLightCamera();

	virtual void CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void ReleaseShaderVariables();
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList);
};
