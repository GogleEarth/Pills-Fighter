#pragma once

#include "Shader.h"
#include "Player.h"
#include "Sound.h"
#include "Font.h"

class CRepository;
class CSound;

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

class CScene
{
public:
	CScene();
	virtual ~CScene();

	virtual void BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, CRepository *pRepository);
	virtual void ReleaseObjects();
	virtual void SetAfterBuildObject(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, void *pContext);
	virtual void ReleaseUploadBuffers();

	virtual void OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM	lParam);
	virtual void OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	virtual void ProcessInput(UCHAR *pKeysBuffer, float fElapsedTime);

	virtual void CheckCollision() {}
	virtual void CheckCollisionPlayer() {}
	virtual void FindAimToTargetDistance() {}

	virtual void AnimateObjects(float fTimeElapsed, CCamera *pCamera);
	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera);
	virtual void RenderWire(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera);
	virtual void PrepareRender(ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void AfterRender(ID3D12GraphicsCommandList *pd3dCommandList);

	virtual void BuildLightsAndMaterials() {}

	virtual void CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList) {};
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList) {};
	virtual void ReleaseShaderVariables() {};

public:
	static void CreateGraphicsRootSignature(ID3D12Device *pd3dDevice);
	static void CreateDescriptorHeaps(ID3D12Device *pd3dDevice, int nViews);
	static D3D12_GPU_DESCRIPTOR_HANDLE CreateShaderResourceViews(ID3D12Device *pd3dDevice, CTexture *pTexture, UINT nRootParameter, bool bAutoIncrement);
	static D3D12_GPU_DESCRIPTOR_HANDLE CreateShaderResourceViews(ID3D12Device *pd3dDevice, ID3D12Resource *pd3dResource, UINT nSrvType);
	static void ReleaseDescHeapsAndGraphicsRootSign();
	static void SetDescHeapsAndGraphicsRootSignature(ID3D12GraphicsCommandList *pd3dCommandList);

	static void CreateRtvAndDsvDescriptorHeaps(ID3D12Device *pd3dDevice);
	static void CreateRenderTargetView(ID3D12Device *pd3dDevice, ID3D12Resource *pd3dResource, D3D12_RTV_DIMENSION d3dRtvDimension, int nViews, D3D12_CPU_DESCRIPTOR_HANDLE *pd3dSaveCPUHandle);
	static void CreateDepthStencilView(ID3D12Device *pd3dDevice, ID3D12Resource *pd3dResource, D3D12_CPU_DESCRIPTOR_HANDLE *pd3dSaveCPUHandle);
	static void ResetDescriptorHeapHandles();

protected:
	static ID3D12DescriptorHeap				*m_pd3dDescriptorHeap;

	static D3D12_CPU_DESCRIPTOR_HANDLE		m_d3dSrvCPUDescriptorStartHandle;
	static D3D12_GPU_DESCRIPTOR_HANDLE		m_d3dSrvGPUDescriptorStartHandle;

	static ID3D12RootSignature				*m_pd3dGraphicsRootSignature;

	static ID3D12DescriptorHeap				*m_pd3dRtvDescriptorHeap;
	static D3D12_CPU_DESCRIPTOR_HANDLE		m_d3dRtvCPUDesciptorStartHandle;
	static D3D12_GPU_DESCRIPTOR_HANDLE		m_d3dRtvGPUDesciptorStartHandle;

	static ID3D12DescriptorHeap				*m_pd3dDsvDescriptorHeap;
	static D3D12_CPU_DESCRIPTOR_HANDLE		m_d3dDsvCPUDesciptorStartHandle;
	static D3D12_GPU_DESCRIPTOR_HANDLE		m_d3dDsvGPUDesciptorStartHandle;

	int										m_nFPS = 0;

public:
	void SetPlayer(CPlayer* pPlayer) { m_pPlayer = pPlayer; }
	CPlayer *GetPlayer() { return m_pPlayer; }
	CHeightMapTerrain *GetTerrain() { return(m_pTerrain); }

	ID3D12RootSignature *GetGraphicsRootSignature() { return(m_pd3dGraphicsRootSignature); }

	CShader* GetBulletShader(UINT index) { return m_ppShaders[index]; }

	float GetToTargetDistance() { return m_fCameraToTarget; }

protected:
	LIGHTS								*m_pLights = NULL;
	ID3D12Resource						*m_pd3dcbLights = NULL;
	LIGHTS								*m_pcbMappedLights = NULL;

	CPlayer								*m_pPlayer = NULL;
	CHeightMapTerrain					*m_pTerrain = NULL;
	CSkyBox								*m_pSkyBox = NULL;

	int									m_nShaders = 0;
	CShader								**m_ppShaders = NULL;

	CShader								*m_pWireShader = NULL;
	CParticleShader						*m_pParticleShader = NULL;

	int									m_nEffectShaders = 0;
	CEffectShader						**m_ppEffectShaders = NULL;

	CUserInterface						*m_pUserInterface = NULL;

protected:
	float			m_fGravAcc = 9.8f;
	float			m_fCameraToTarget = 0.0f;

public:
	virtual void StartScene() {};

public:
	void AddFont(ID3D12Device *pd3dDevice, CFont *pFont);
	CTextObject* AddText(const char *pstrFont, const char *pstrText, XMFLOAT2 xmf2Position, XMFLOAT2 xmf2Scale, XMFLOAT2 xmf2Padding, XMFLOAT4 xmf4Color);
	void ChangeText(CTextObject *pTextObject, const char *pstrFont, const char *pstrText, XMFLOAT2 xmf2Position, XMFLOAT2 xmf2Scale, XMFLOAT2 xmf2Padding, XMFLOAT4 xmf4Color);

protected:
	CFontShader						*m_pFontShader = NULL;
	std::vector<CFont*>				m_vpFonts;

protected:
	BOOL							m_LButtonDown = FALSE;
	BOOL							m_bRenderWire = FALSE;

protected:
	CCursor							*m_pCursor = NULL;

public:
	void MoveCursor(float x, float y);
	virtual int MouseClick() { return 0; };

public: // Network
	virtual void InsertObject(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, PKT_CREATE_OBJECT *pCreateObjectInfo) {}
	virtual void DeleteObject(PKT_DELETE_OBJECT *pDeleteObjectInfo) {}
	virtual void CreateEffect(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, PKT_CREATE_EFFECT *pCreateEffectInfo) {}
	virtual void ApplyRecvInfo(PKT_ID pktID, LPVOID pktData) {}
	virtual void JoinPlayer(int nIndex, const char *pstrPlayerName) {};
	virtual void LeavePlayer(int nSlotIndex, bool isPlayerIndex) {};
	virtual void SetPlayerIndex(int nIndex) {}

};

//////////////////////////////////////////////////////////////////////////////////////////////////

class CLobbyScene : public CScene
{
public:
	CLobbyScene();
	virtual ~CLobbyScene();

	virtual void OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM	lParam);

	virtual void BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, CRepository *pRepository);
	virtual void SetAfterBuildObject(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, void *pContext);
	virtual void ReleaseObjects();
	virtual void ReleaseUploadBuffers();

	virtual void CheckCollision();
	virtual void StartScene();

	virtual void JoinPlayer(int nIndex, const char *pstrPlayerName);
	virtual void LeavePlayer(int nSlotIndex, bool isPlayerIndex);
	virtual void SetPlayerIndex(int nIndex);
	virtual int MouseClick();

	XMFLOAT2 GetPlayerTextPosition();

	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera);

protected:
	CLobbyShader					*m_pLobbyShader = NULL;

	int								m_nMyIndex = 0;
	int								m_nChoiceCharactor = 0;
	int								m_nPlayers = 0;
	CTextObject						*m_ppTextObjects[8] = { NULL };
	std::unordered_map<int, int>	m_umPlayerInfo; // SlotIndex, ServerIndex

protected:
	BoundingBox		m_StartButton;
	bool			m_bHLStartButton = false;

	BoundingBox		m_SelectLeft;
	bool			m_bHLSelectLeft = false;

	BoundingBox		m_SelectRight;
	bool			m_bHLSelectRight = false;
	
	BoundingBox		m_PlayerSlots[8];

	std::vector<char*> m_vstrPlayerNames;

protected:
	int								m_nUIRect = 0;
	CRect							**m_ppUIRects = NULL;

	int								m_nTextures;
	CTexture						**m_ppTextures = NULL;
};

//////////////////////////////////////////////////////////////////////////////////////////////////

class CColonyScene : public CScene
{
public:
	CColonyScene();
	virtual ~CColonyScene();

	virtual void OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM	lParam);
	virtual void OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	virtual void ProcessInput(UCHAR *pKeysBuffer, float fElapsedTime);

	virtual void BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, CRepository *pRepository);
	virtual void ReleaseObjects();
	virtual void SetAfterBuildObject(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, void *pContext);
	virtual void ReleaseUploadBuffers();

	virtual void CheckCollision();
	virtual void CheckCollisionPlayer();
	virtual void FindAimToTargetDistance();

	virtual void BuildLightsAndMaterials();

	virtual void CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void ReleaseShaderVariables();

	virtual void StartScene();

	virtual void PrepareRender(ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera);
	virtual void RenderCubeMap(ID3D12GraphicsCommandList *pd3dCommandList);

	void CreateEnvironmentMap(ID3D12Device *pd3dDevice);
	void CreateCubeMapCamera(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	void CreateRtvDsvSrvEnvironmentMap(ID3D12Device *pd3dDevice);

protected:
	ID3D12Resource					*m_pd3dEnvirCube = NULL;
	ID3D12Resource					*m_pd3dEnvirCubeDSBuffer = NULL;
	D3D12_CPU_DESCRIPTOR_HANDLE		m_d3dRrvEnvirCubeMapCPUHandle[6];
	D3D12_CPU_DESCRIPTOR_HANDLE		m_d3dDsvEnvirCubeMapCPUHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE		m_d3dSrvEnvirCubeMapGPUHandle;

	CCamera							*m_pCubeMapCamera[6];

	D3D12_VIEWPORT 					m_d3dEMViewport;
	D3D12_RECT						m_d3dEMScissorRect;

public:
	void CreateMinimapMap(ID3D12Device *pd3dDevice);
	void CreateMiniMapCamera(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	void CreateRtvDsvSrvMiniMap(ID3D12Device *pd3dDevice);
	void MinimapRender(ID3D12GraphicsCommandList *pd3dCommandList);

protected:
	ID3D12Resource					*m_pd3dMinimapRsc = NULL;
	ID3D12Resource					*m_pd3dMinimapDepthStencilBuffer = NULL;
	CTexture						*screenCaptureTexture = NULL;
	D3D12_CPU_DESCRIPTOR_HANDLE		m_d3dRtvMinimapCPUHandle;
	D3D12_CPU_DESCRIPTOR_HANDLE		m_d3dDsvMinimapCPUHandle;

	CCamera							*m_pMiniMapCamera = NULL;

	D3D12_VIEWPORT 					m_d3dMMViewport;
	D3D12_RECT						m_d3dMMScissorRect;

public: // Network
	virtual void InsertObject(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, PKT_CREATE_OBJECT *pCreateObjectInfo);
	virtual void DeleteObject(PKT_DELETE_OBJECT *pDeleteObjectInfo);
	virtual void CreateEffect(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, PKT_CREATE_EFFECT *pCreateEffectInfo);
	virtual void ApplyRecvInfo(PKT_ID pktID, LPVOID pktData);

protected:
	CGameObject* m_pObjects[MAX_NUM_OBJECT];

	CModel		*m_pGimGun = NULL;
	CModel		*m_pBazooka = NULL;
	CModel		*m_pMachineGun = NULL;
};