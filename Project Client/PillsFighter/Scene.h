#pragma once

#include "Shader.h"
#include "Player.h"

struct LIGHT
{
	XMFLOAT4 m_xmf4Ambient;
	XMFLOAT4 m_xmf4Diffuse;
	XMFLOAT4 m_xmf4Specular;
	XMFLOAT3 m_xmf3Position;
	float m_fFalloff;
	XMFLOAT3 m_xmf3Direction;
	float m_fTheta; //cos(m_fTheta)
	XMFLOAT3 m_xmf3Attenuation;
	float m_fPhi; //cos(m_fPhi)
	bool m_bEnable;
	int m_nType;
	float m_fRange;
	float padding;
};

struct LIGHTS
{
	LIGHT m_pLights[MAX_LIGHTS];
	XMFLOAT4 m_xmf4GlobalAmbient;
};

class CRepository;

class CScene
{
public:
	CScene();
	~CScene();

	bool OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM	lParam);
	bool OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);

	void BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, CRepository *pRepository);
	void ReleaseObjects();

	ID3D12RootSignature *CreateGraphicsRootSignature(ID3D12Device *pd3dDevice);
	ID3D12RootSignature *GetGraphicsRootSignature() { return(m_pd3dGraphicsRootSignature); }

	bool ProcessInput(UCHAR *pKeysBuffer);
	void AnimateObjects(float fTimeElapsed, CCamera *pCamera);
	void Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera);
	void RenderWire(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera);

	void ReleaseUploadBuffers();

	CHeightMapTerrain *GetTerrain() { return(m_pTerrain); }

protected:
	CPlayer						*m_pPlayer = NULL;

	int							m_nShaders = 0;
	CShader						**m_ppShaders = NULL;

	CHeightMapTerrain			*m_pTerrain = NULL;
	CSkyBox						*m_pSkyBox = NULL;

	ID3D12RootSignature			*m_pd3dGraphicsRootSignature = NULL;

	CShader						*m_pWireShader = NULL;

public:
	// 충돌 체크를 검사한다.
	virtual void CheckCollision();
	virtual void CheckCollisionPlayer();
	virtual void FindAimToTargetDistance();

	void SetPlayer(CPlayer* pPlayer);

	CShader* GetBulletShader(UINT index) { return m_ppShaders[index]; }

public: //For Lights
	void BuildLightsAndMaterials();

	virtual void CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void ReleaseShaderVariables();

protected:
	//씬의 조명
	LIGHTS			*m_pLights = NULL;

	ID3D12Resource	*m_pd3dcbLights = NULL;
	LIGHTS			*m_pcbMappedLights = NULL;

public: // For Network
	CGameObject* m_pObjects[MAX_NUM_OBJECT];

	void InsertObject(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, PKT_CREATE_OBJECT CreateObjectInfo);
	void DeleteObject(PKT_DELETE_OBJECT DeleteObjectInfo);
	void CreateEffect(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, PKT_CREATE_EFFECT CreateEffectInfo);
	void ApplyRecvInfo(PKT_ID pktID, LPVOID pktData);

	//////
public:
	float		m_fGravAcc = 9.8f;

public:
	static void CreateDescriptorHeaps(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, int nViews);
	static D3D12_GPU_DESCRIPTOR_HANDLE CreateShaderResourceViews(ID3D12Device *pd3dDevice, CTexture *pTexture, UINT nRootParameter, bool bAutoIncrement);

	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandleForHeapStart() { return(m_pd3dDescriptorHeap->GetCPUDescriptorHandleForHeapStart()); }
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandleForHeapStart() { return(m_pd3dDescriptorHeap->GetGPUDescriptorHandleForHeapStart()); }

	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUSrvDescriptorStartHandle() { return(m_d3dSrvCPUDescriptorStartHandle); }
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUSrvDescriptorStartHandle() { return(m_d3dSrvGPUDescriptorStartHandle); }

protected:
	static ID3D12DescriptorHeap		*m_pd3dDescriptorHeap;

	static D3D12_CPU_DESCRIPTOR_HANDLE		m_d3dSrvCPUDescriptorStartHandle;
	static D3D12_GPU_DESCRIPTOR_HANDLE		m_d3dSrvGPUDescriptorStartHandle;


public:
	float GetToTargetDistance() { return m_fCameraToTarget; }

protected:
	float m_fCameraToTarget = 0.0f;

protected:
	CModel		*m_pGimGun = NULL;
	CShader		*m_pGimGunBulletShader = NULL;

	CModel		*m_pBazooka = NULL;
	CShader		*m_pBazookaBulletShader = NULL;

	CModel		*m_pMachineGun = NULL;
	CShader		*m_pMachineGunBulletShader = NULL;

public:
	CModel* GetGimGun() { return m_pGimGun; }
	CShader* GetGimGunBullet() { return m_pGimGunBulletShader; }

	CModel* GetBazooka() { return m_pBazooka; }
	CShader* GetBazookaBullet() { return m_pBazookaBulletShader; }

	CModel* GetMachineGun() { return m_pMachineGun; }
	CShader* GetMachineGunBullet() { return m_pMachineGunBulletShader; }

};
