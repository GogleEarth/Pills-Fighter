#pragma once

#include "Shader.h"
#include "Player.h"
#include "Sound.h"
#include "Font.h"

class CRepository;
class CSound;

// for lights
#define MAX_LIGHTS			3
#define MAX_MATERIALS		8

#define POINT_LIGHT			1
#define SPOT_LIGHT			2
#define DIRECTIONAL_LIGHT	3

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

// Scene Type
#define SCENE_TYPE_LOBBY 1
#define SCENE_TYPE_COLONY 2

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
	virtual void RenderEffects(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera);
	virtual void RenderUI(ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void PrepareRender(ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void AfterRender(ID3D12GraphicsCommandList *pd3dCommandList);

	virtual void BuildLightsAndMaterials() {}

	virtual void CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList) {};
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList) {};
	virtual void ReleaseShaderVariables() {};

public:
	static void CreateGraphicsRootSignature(ID3D12Device *pd3dDevice);
	static void CreateDescriptorHeaps(ID3D12Device *pd3dDevice, int nViews);
	static D3D12_GPU_DESCRIPTOR_HANDLE CreateShaderResourceViews(ID3D12Device *pd3dDevice, CTexture *pTexture, UINT nRootParameter, bool bAutoIncrement, bool bIsModelTexture);
	static D3D12_GPU_DESCRIPTOR_HANDLE CreateShaderResourceViews(ID3D12Device *pd3dDevice, ID3D12Resource *pd3dResource, UINT nSrvType);
	static void ReleaseDescHeapsAndGraphicsRootSign();
	static void SetDescHeapsAndGraphicsRootSignature(ID3D12GraphicsCommandList *pd3dCommandList);

	static void CreateRtvAndDsvDescriptorHeaps(ID3D12Device *pd3dDevice);
	static void CreateRenderTargetView(ID3D12Device *pd3dDevice, ID3D12Resource *pd3dResource, D3D12_RTV_DIMENSION d3dRtvDimension, int nViews, D3D12_CPU_DESCRIPTOR_HANDLE *pd3dSaveCPUHandle);
	static void CreateDepthStencilView(ID3D12Device *pd3dDevice, ID3D12Resource *pd3dResource, D3D12_CPU_DESCRIPTOR_HANDLE *pd3dSaveCPUHandle);
	static void ResetDescriptorHeapHandles();

protected:
	static ID3D12DescriptorHeap				*m_pd3dDescriptorHeap;
	static D3D12_CPU_DESCRIPTOR_HANDLE		m_d3dSrvTextureCPUDescStartHandle;
	static D3D12_GPU_DESCRIPTOR_HANDLE		m_d3dSrvTextureGPUDescStartHandle;
	static D3D12_CPU_DESCRIPTOR_HANDLE		m_d3dSrvModelCPUDescStartHandle;
	static D3D12_GPU_DESCRIPTOR_HANDLE		m_d3dSrvModelGPUDescStartHandle;

	static ID3D12RootSignature				*m_pd3dGraphicsRootSignature;

	static ID3D12DescriptorHeap				*m_pd3dRtvDescriptorHeap;
	static D3D12_CPU_DESCRIPTOR_HANDLE		m_d3dRtvCPUDesciptorStartHandle;
	static D3D12_GPU_DESCRIPTOR_HANDLE		m_d3dRtvGPUDesciptorStartHandle;

	static ID3D12DescriptorHeap				*m_pd3dDsvDescriptorHeap;
	static D3D12_CPU_DESCRIPTOR_HANDLE		m_d3dDsvCPUDesciptorStartHandle;
	static D3D12_GPU_DESCRIPTOR_HANDLE		m_d3dDsvGPUDesciptorStartHandle;

	static int								m_nPlayerRobotType;
	int										m_nFPS = 0;

public:
	void SetPlayer(CPlayer* pPlayer) { m_pPlayer = pPlayer; }
	CPlayer *GetPlayer() { return m_pPlayer; }
	CHeightMapTerrain *GetTerrain() { return(m_pTerrain); }

	ID3D12RootSignature *GetGraphicsRootSignature() { return(m_pd3dGraphicsRootSignature); }
	int GetPlayerRobotType() { return m_nPlayerRobotType; }

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
	CTextObject* AddText(const char *pstrFont, const char *pstrText, XMFLOAT2 xmf2Position, XMFLOAT2 xmf2Scale, XMFLOAT2 xmf2Padding, XMFLOAT4 xmf4Color, int nType);
	void ChangeText(CTextObject *pTextObject, const char *pstrFont, const char *pstrText, XMFLOAT2 xmf2Position, XMFLOAT2 xmf2Scale, XMFLOAT2 xmf2Padding, XMFLOAT4 xmf4Color, int nType);

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

	virtual void EndScene() {};

public: // Network
	virtual void InsertObject(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, PKT_CREATE_OBJECT *pCreateObjectInfo) {}
	virtual void DeleteObject(int nIndex) {}
	virtual void CreateEffect(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, PKT_CREATE_EFFECT *pCreateEffectInfo) {}
	virtual void ApplyRecvInfo(PKT_ID pktID, LPVOID pktData) {}
	virtual void JoinPlayer(int nServerIndex, const char *pstrPlayerName) {};
	virtual void LeavePlayer(int nServerIndex) {};
	virtual void SetPlayerIndex(int nServerIndex) {}
	virtual void ChangeSelectRobot(int nServerIndex, int nRobotType) {}

protected:
	int	m_nMyIndex = 0;
};

//////////////////////////////////////////////////////////////////////////////////////////////////

struct SERVERINFO
{
	int nSlotIndex;
	int nRobotType;
};

// Lobby Scene's Shader Index
#define LOBBY_SHADER_INDEX 1

#define INDEX_LOBBY_SHADER_UI 0

// Lobby UI Click Type
#define MOUSE_CLICK_TYPE_START 1
#define MOUSE_CLICK_SELECT_ROBOT 2

// UI Texture Index
#define UI_TEXTURE_COUNT 13

#define UI_TEXTURE_BASE 0
#define UI_TEXTURE_GAMESTART 1
#define UI_TEXTURE_HL_GAMESTART 2
#define UI_TEXTURE_READY 3
#define UI_TEXTURE_HL_READY 4
#define UI_TEXTURE_SELECT_LEFT 5
#define UI_TEXTURE_HL_SELECT_LEFT 6
#define UI_TEXTURE_SELECT_RIGHT 7
#define UI_TEXTURE_HL_SELECT_RIGHT 8
#define UI_TEXTURE_GM_TEXT 9
#define UI_TEXTURE_HL_GM_TEXT 10
#define UI_TEXTURE_GUNDAM_TEXT 11
#define UI_TEXTURE_HL_GUNDAM_TEXT 12

// UI Rect Index
#define UI_RECT_BASE 0
#define UI_RECT_START_BUTTON 1
#define UI_RECT_SELECT_LEFT 2
#define UI_RECT_SELECT_RIGHT 3
#define UI_RECT_GM_TEXT 4
#define UI_RECT_GUNDAM_TEXT 5

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

	virtual void JoinPlayer(int nServerIndex, const char *pstrPlayerName);
	virtual void LeavePlayer(int nServerIndex);
	virtual void SetPlayerIndex(int nServerIndex);
	virtual void ChangeSelectRobot(int nServerIndex, int nRobotType);
	virtual int MouseClick();

	XMFLOAT2 GetPlayerTextPosition(int nServerIndex);

	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera);

protected:
	CLobbyShader						*m_pLobbyShader = NULL;

	int									m_nChoiceCharactor = 0;
	int									m_nCurrentSlotIndex = 0;
	CTextObject							*m_ppTextObjects[8] = { NULL }; // ServerIndex
	std::unordered_map<int, SERVERINFO>	m_umPlayerInfo;			// ServerIndex, Info

	CRect								*m_ppPlayerRobotRects[8] = { NULL }; // SlotIndex

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

// Scene's Shader Index
#define SHADER_INDEX 3

#define INDEX_SHADER_STANDARD_OBJECTS 0
#define INDEX_SHADER_INSTANCING_OBJECTS 1
#define INDEX_SHADER_SKINND_OBJECTS 2

// Standard Shader's Object Group
#define STANDARD_OBJECT_GROUP 5

#define STANDARD_OBJECT_INDEX_GG_BULLET 0
#define STANDARD_OBJECT_INDEX_BZK_BULLET 1
#define STANDARD_OBJECT_INDEX_MG_BULLET 2
#define STANDARD_OBJECT_INDEX_REPAIR_ITEM 3
#define STANDARD_OBJECT_INDEX_AMMO_ITEM 4

// Instancing Shader's Object Group
#define INSTANCING_OBJECT_GROUP 10

#define INSTANCING_OBJECT_INDEX_HANGAR 0
#define INSTANCING_OBJECT_INDEX_DOUBLESQUARE 1
#define INSTANCING_OBJECT_INDEX_OCTAGON 2
#define INSTANCING_OBJECT_INDEX_OCTAGONLONGTIER 3
#define INSTANCING_OBJECT_INDEX_SLOPETOP 4
#define INSTANCING_OBJECT_INDEX_SQUARE 5
#define INSTANCING_OBJECT_INDEX_STEEPLETOP 6
#define INSTANCING_OBJECT_INDEX_WALL 7
#define INSTANCING_OBJECT_INDEX_BRICKGARAGE 8
#define INSTANCING_OBJECT_INDEX_FENCE 9

// Standard Shader's Object Group
#define SKINNED_OBJECT_GROUP 2

#define SKINNED_OBJECT_INDEX_GM 0
#define SKINNED_OBJECT_INDEX_GUNDAM 1


// Scene's Effect Shader Index
#define EFFECT_SHADER_INDEX 3

#define INDEX_SHADER_TIMED_EEFECTS 0
#define INDEX_SHADER_SPRITE_EFFECTS 1
#define INDEX_SHADER_TEXT_EEFECTS 2


class CColonyScene : public CScene
{
public:
	CColonyScene();
	virtual ~CColonyScene();

	virtual void OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM	lParam);
	virtual void OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);

	virtual void BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, CRepository *pRepository);
	virtual void ReleaseObjects();
	virtual void ReleaseUploadBuffers();
	virtual void SetAfterBuildObject(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, void *pContext);

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
	virtual void RenderCubeMap(ID3D12GraphicsCommandList *pd3dCommandList, CGameObject *pMainObject);

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

protected:
	int								m_nRedScore = 0;
	int								m_nBlueScore = 0;

	CTextObject						*m_pRedScoreText = NULL;
	CTextObject						*m_pBlueScoreText = NULL;

public:
	void AddParticle(int nType, XMFLOAT3 xmf3Position, int nNum);
	virtual void EndScene();

public: // Network
	virtual void InsertObject(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, PKT_CREATE_OBJECT *pCreateObjectInfo);
	virtual void DeleteObject(int nIndex);
	virtual void CreateEffect(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, PKT_CREATE_EFFECT *pCreateEffectInfo);
	virtual void ApplyRecvInfo(PKT_ID pktID, LPVOID pktData);
	virtual void LeavePlayer(int nServerIndex);

protected:
	CGameObject* m_pObjects[MAX_NUM_OBJECT];

	CModel		*m_pGimGun = NULL;
	CModel		*m_pBazooka = NULL;
	CModel		*m_pMachineGun = NULL;
	CModel		*m_pSaber = NULL;
};