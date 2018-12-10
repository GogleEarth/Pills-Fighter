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

struct MATERIAL
{
	XMFLOAT4 m_xmf4Ambient;
	XMFLOAT4 m_xmf4Diffuse;
	XMFLOAT4 m_xmf4Specular; //(r,g,b,a=power)
	XMFLOAT4 m_xmf4Emissive;
};

struct MATERIALS
{
	MATERIAL m_pReflections[MAX_MATERIALS];
};


class CScene
{
public:
	CScene();
	~CScene();

	//씬에서 마우스와 키보드 메시지를 처리한다. 
	bool OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM	lParam);
	bool OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);

	void BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	void ReleaseObjects();

	//그래픽 루트 시그너쳐를 생성한다. 
	ID3D12RootSignature *CreateGraphicsRootSignature(ID3D12Device *pd3dDevice);
	ID3D12RootSignature *GetGraphicsRootSignature() { return(m_pd3dGraphicsRootSignature); }
	void SetGraphicsRootSignature(ID3D12GraphicsCommandList *pd3dCommandList) { pd3dCommandList->SetGraphicsRootSignature(m_pd3dGraphicsRootSignature); }

	bool ProcessInput(UCHAR *pKeysBuffer);
	void AnimateObjects(float fTimeElapsed, CCamera *pCamera);
	void Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera);
	void RenderWire(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera);

	void ReleaseUploadBuffers();

	CHeightMapTerrain *GetTerrain() { return(m_pTerrain); }

protected:
	CPlayer						*m_pPlayer = NULL;

	int							m_nShaders = 0;
	CObjectsShader				**m_ppShaders = NULL;

	CHeightMapTerrain			*m_pTerrain = NULL;
	CSkyBox						*m_pSkyBox = NULL;

	ID3D12RootSignature			*m_pd3dGraphicsRootSignature = NULL;

public:
	// 충돌 체크를 검사한다.
	void CheckCollision();

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

	MATERIALS		*m_pMaterials = NULL;

	ID3D12Resource	*m_pd3dcbMaterials = NULL;
	MATERIAL		*m_pcbMappedMaterials = NULL;

public: // For Network
	CGameObject* m_pObjects[MAX_NUM_OBJECT];

	void InsertObject(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, PKT_CREATE_OBJECT CreateObjectInfo);
	void DeleteObject(PKT_DELETE_OBJECT DeleteObjectInfo);
	void CreateEffect(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, PKT_CREATE_EFFECT CreateEffectInfo);
	void ApplyRecvInfo(PKT_ID pktID, LPVOID pktData);
};
