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
#define SCENE_TYPE_LOBBY_MAIN 1
#define SCENE_TYPE_LOBBY_ROOM 2
#define SCENE_TYPE_COLONY 3
#define SCENE_TYPE_SPACE 4

class CScene
{
public:
	CScene();
	virtual ~CScene();

	virtual void BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, CRepository *pRepository);
	virtual void ReleaseObjects();
	virtual void SetAfterBuildObject(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, void *pContext);
	virtual void ReleaseUploadBuffers();

	virtual int OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM	lParam);
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
	virtual void RenderOffScreen(ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void RenderTestTexture(ID3D12GraphicsCommandList *pd3dCommandList, D3D12_GPU_DESCRIPTOR_HANDLE d3dSrvGPUHandle);
	virtual void PrepareRender(ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void AfterRender(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera);

	virtual void BuildLightsAndMaterials() {}

	virtual void CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList) {};
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList) {};
	virtual void ReleaseShaderVariables() {};

public:
	static void CreateGraphicsRootSignature(ID3D12Device *pd3dDevice);
	static void CreateComputeRootSignature(ID3D12Device *pd3dDevice);
	static void CreateDescriptorHeaps(ID3D12Device *pd3dDevice, int nViews);
	static D3D12_GPU_DESCRIPTOR_HANDLE CreateShaderResourceViews(ID3D12Device *pd3dDevice, CTexture *pTexture, UINT nRootParameter, bool bAutoIncrement, bool bIsModelTexture);
	static D3D12_GPU_DESCRIPTOR_HANDLE CreateShaderResourceViews(ID3D12Device *pd3dDevice, ID3D12Resource *pd3dResource, UINT nSrvType, bool bIsDS = false);
	static D3D12_GPU_DESCRIPTOR_HANDLE CreateUnorderedAccessViews(ID3D12Device *pd3dDevice, ID3D12Resource *pd3dResource);
	static void ReleaseDescHeapsAndRootSignature();
	static void SetDescHeapsAndGraphicsRootSignature(ID3D12GraphicsCommandList *pd3dCommandList);

	static void CreateRtvAndDsvDescriptorHeaps(ID3D12Device *pd3dDevice);
	static void CreateRenderTargetView(ID3D12Device *pd3dDevice, ID3D12Resource *pd3dResource, D3D12_RTV_DIMENSION d3dRtvDimension, int nViews, D3D12_CPU_DESCRIPTOR_HANDLE *pd3dSaveCPUHandle);
	static void CreateDepthStencilView(ID3D12Device *pd3dDevice, ID3D12Resource *pd3dResource, D3D12_CPU_DESCRIPTOR_HANDLE *pd3dSaveCPUHandle);
	static void ResetDescriptorHeapHandles();

protected:
	static ID3D12DescriptorHeap				*m_pd3dSrvUavDescriptorHeap;
	static D3D12_CPU_DESCRIPTOR_HANDLE		m_d3dSrvUavTextureCPUDescStartHandle;
	static D3D12_GPU_DESCRIPTOR_HANDLE		m_d3dSrvUavTextureGPUDescStartHandle;
	static D3D12_CPU_DESCRIPTOR_HANDLE		m_d3dSrvModelCPUDescStartHandle;
	static D3D12_GPU_DESCRIPTOR_HANDLE		m_d3dSrvModelGPUDescStartHandle;

	static ID3D12RootSignature				*m_pd3dGraphicsRootSignature;
	static ID3D12RootSignature				*m_pd3dComputeRootSignature;

	static ID3D12DescriptorHeap				*m_pd3dRtvDescriptorHeap;
	static D3D12_CPU_DESCRIPTOR_HANDLE		m_d3dRtvCPUDesciptorStartHandle;
	static D3D12_GPU_DESCRIPTOR_HANDLE		m_d3dRtvGPUDesciptorStartHandle;

	static ID3D12DescriptorHeap				*m_pd3dDsvDescriptorHeap;
	static D3D12_CPU_DESCRIPTOR_HANDLE		m_d3dDsvCPUDesciptorStartHandle;
	static D3D12_GPU_DESCRIPTOR_HANDLE		m_d3dDsvGPUDesciptorStartHandle;

#define SELECT_CHARACTER_GM 0
#define SELECT_CHARACTER_GUNDAM 1
#define SELECT_CHARACTER_ZAKU 2
	static int								m_nPlayerRobotType;
	int										m_nFPSCount = 0;

public:
	void SetPlayer(CPlayer* pPlayer) { m_pPlayer = pPlayer; }
	CPlayer *GetPlayer() { return m_pPlayer; }
	CHeightMapTerrain *GetTerrain() { return(m_pTerrain); }

	ID3D12RootSignature *GetGraphicsRootSignature() { return(m_pd3dGraphicsRootSignature); }
	int GetPlayerRobotType() { return m_nPlayerRobotType; }
	virtual int GetSelectedMap() { return 0; }
	virtual void ChangeMap(int nMap) {}
	virtual int GetSelectRoom() { return -1; }

	CShader* GetBulletShader(UINT index) { return m_ppShaders[index]; }

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

	XMFLOAT4X4							m_xmf4x4PrevViewProjection;
	XMFLOAT4X4							m_xmf4x4CurrViewProjection;
	XMFLOAT3							m_xmf3PrevPlayerPosition;
	float								m_fFPS = 0.0f;

	bool								m_bSelfIllumination = true;
	bool								m_bBloom = true;
	bool								m_bMotionBlur = true;
	bool								m_bMotionBlurred = false;

public:
	virtual void StartScene() {};
	virtual void MoveCursor(float x, float y) {}
	virtual void SetCursorPosition(XMFLOAT2 xmf2Position) {}
	virtual XMFLOAT2 GetCursorPos() { return XMFLOAT2(0.0f, 0.0f); }
	virtual void AddRoom(int n) {}
	virtual void DeleteRoom(int n) {}
	virtual void ChangeRoomInfo(int index, int map, int people) {}

public:
	void SetFont(ID3D12Device *pd3dDevice, CFont *pFont);
	CTextObject* AddText(const wchar_t *pstrText, XMFLOAT2 xmf2Position, XMFLOAT2 xmf2Scale, XMFLOAT2 xmf2Padding, XMFLOAT4 xmf4Color, int nType);
	void ChangeText(CTextObject *pTextObject,const wchar_t *pstrText, XMFLOAT2 xmf2Position, XMFLOAT2 xmf2Scale, XMFLOAT2 xmf2Padding, XMFLOAT4 xmf4Color, int nType);

protected:
	CFontShader						*m_pFontShader = NULL;
	CFont*							m_pFont = NULL;

protected:
	BOOL							m_LButtonDown = FALSE;
	//BOOL							m_bRenderWire = FALSE;

public:
	virtual void EndScene() {};

public:
	void CreateOffScreenTextures(ID3D12Device *pd3dDevice);
	void CreateRtvDsvSrvUavOffScreens(ID3D12Device *pd3dDevice);
	void PreparePostProcessing(ID3D12GraphicsCommandList *pd3dCommandList);
	void MotionBlur(ID3D12GraphicsCommandList *pd3dCommandList, int nWidth, int nHeight);
	void Bloom(ID3D12GraphicsCommandList *pd3dCommandList, int nWidth, int nHeight);
	void Blurring(ID3D12GraphicsCommandList *pd3dCommandList, int nWidth, int nHeight);
	void Combine(ID3D12GraphicsCommandList *pd3dCommandList, int nWidth, int nHeight);

protected:
	ID3D12Resource					*m_pd3dOffScreenTexture = NULL;
	ID3D12Resource					*m_pd3dDepthStencilBuffer = NULL;
	D3D12_CPU_DESCRIPTOR_HANDLE		m_d3dRrvOffScreenCPUHandle;
	D3D12_CPU_DESCRIPTOR_HANDLE		m_d3dDsvOffScreenCPUHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE		m_d3dSrvOffScreenGPUHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE		m_d3dUavOffScreenGPUHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE		m_d3dSrvDepthStencilGPUHandle;

	ID3D12Resource					*m_pd3dGlowScreenTexture = NULL;
	D3D12_CPU_DESCRIPTOR_HANDLE		m_d3dRrvGlowScreenCPUHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE		m_d3dSrvGlowScreenGPUHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE		m_d3dUavGlowScreenGPUHandle;

	ID3D12Resource					*m_pd3dMotionBlurScreenTexture = NULL;
	D3D12_GPU_DESCRIPTOR_HANDLE		m_d3dSrvMotionBlurScreenGPUHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE		m_d3dUavMotionBlurScreenGPUHandle;

	ID3D12Resource					*m_pd3dTempTexture = NULL;
	D3D12_GPU_DESCRIPTOR_HANDLE		m_d3dSrvTempTextureGPUHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE		m_d3dUavTempTextureGPUHandle;

	ID3D12Resource					*m_pd3dMaskTexture = NULL;
	D3D12_CPU_DESCRIPTOR_HANDLE		m_d3dRrvMaskTextureCPUHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE		m_d3dSrvMaskTextureGPUHandle;

	CComputeShader					*m_pComputeShader = NULL;
	CPostProcessingShader			*m_pPostProcessingShader = NULL;
	CTestShader						*m_pTestShader = NULL;

public: // Network
	virtual void InsertObject(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, PKT_CREATE_OBJECT *pCreateObjectInfo) {}
	virtual void DeleteObject(int nIndex) {}
	virtual void CreateEffect(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, PKT_CREATE_EFFECT *pCreateEffectInfo) {}
	virtual void ApplyRecvInfo(PKT_ID pktID, LPVOID pktData) {}
	virtual void JoinPlayer(int nIndex, int nSlot, const wchar_t *pstrPlayerName, int nRobotType) {}
	virtual void LeavePlayer(int nIndex) {}
	virtual void SetClientIndex(int nIndex, int nSlot) {}
	virtual void SetMap(int nMap) {}
	virtual void ChangeSelectRobot(int nIndex, int nRobotType) {}
	virtual void ChangeSlot(int nIndex, int nChangeSlot) {}
	virtual void SetMyTeam(int nTeam) { CScene::m_nMyTeam = nTeam; }
	virtual int GetMyTeam() { return m_nMyTeam; }
	virtual void AddTeam(int nIndex, wchar_t *pstrName) {}
	virtual void GetTeamsInfo(int nTeam, std::vector<int> &vnIndices, std::vector<wchar_t*> &vpwstrNames) {}

protected:
	static int m_nMyIndex;
	static int m_nMyTeam;
};

//////////////////////////////////////////////////////////////////////////////////////////////////

class CLobbyScene : public CScene
{
public:
	CLobbyScene();
	virtual ~CLobbyScene();

	virtual void ReleaseObjects();
	virtual void ReleaseUploadBuffers();

	virtual int OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM	lParam);
	virtual void BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, CRepository *pRepository);

	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera);

	virtual void MoveCursor(float x, float y);
	virtual void SetCursorPosition(XMFLOAT2 xmf2Position) { m_pCursor->SetCursorPos(xmf2Position); }
	virtual int MouseClick() { return 0; };
	virtual XMFLOAT2 GetCursorPos() { return m_pCursor->GetPosition(); }

protected:
	CLobbyShader					*m_pLobbyShader = NULL;

	int								m_nUIRect = 0;
	CRect							**m_ppUIRects = NULL;

	int								m_nTextures;
	CTexture						**m_ppTextures = NULL;

	CCursor							*m_pCursor = NULL;

	D3D12_VIEWPORT					m_d3dViewport;
	D3D12_RECT						m_d3dScissorRect;
};
//////////////////////////////////////////////////////////////////////////////////////////////////

// UI Texture Index
#define LOBBY_MAIN_UI_TEXTURE_COUNT 8

#define LOBBY_MAIN_UI_TEXTURE_BASE 0
#define LOBBY_MAIN_UI_TEXTURE_CREATE_ROOM 1
#define LOBBY_MAIN_UI_TEXTURE_HL_CREATE_ROOM 2
#define LOBBY_MAIN_UI_TEXTURE_UP 3
#define LOBBY_MAIN_UI_TEXTURE_HL_UP 4
#define LOBBY_MAIN_UI_TEXTURE_DOWN 5
#define LOBBY_MAIN_UI_TEXTURE_HL_DOWN 6
#define LOBBY_MAIN_UI_TEXTURE_HL_ROOM 7

// UI Rect Index
#define LOBBY_MAIN_UI_RECT_COUNT 12

#define LOBBY_MAIN_UI_RECT_BASE 0
#define LOBBY_MAIN_UI_RECT_CREATE_ROOM_BUTTON 1
#define LOBBY_MAIN_UI_RECT_UP_BUTTON 2
#define LOBBY_MAIN_UI_RECT_DOWN_BUTTON 3
#define LOBBY_MAIN_UI_RECT_ROOM_BUTTON_1 4
#define LOBBY_MAIN_UI_RECT_ROOM_BUTTON_2 5
#define LOBBY_MAIN_UI_RECT_ROOM_BUTTON_3 6
#define LOBBY_MAIN_UI_RECT_ROOM_BUTTON_4 7
#define LOBBY_MAIN_UI_RECT_ROOM_BUTTON_5 8
#define LOBBY_MAIN_UI_RECT_ROOM_BUTTON_6 9
#define LOBBY_MAIN_UI_RECT_ROOM_BUTTON_7 10
#define LOBBY_MAIN_UI_RECT_ROOM_BUTTON_8 11

struct ROOM_INFO_TEXT
{
	int			nRoom_num;
	CTextObject *pRoom_num;
	CTextObject *pRoom_name;
	CTextObject *pRoom_map;
	CTextObject *pRoom_num_people;
};

class CLobbyMainScene : public CLobbyScene
{
public:
	CLobbyMainScene();
	virtual ~CLobbyMainScene();

	virtual void BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, CRepository *pRepository);
	virtual void SetAfterBuildObject(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, void *pContext);
	virtual void ReleaseObjects();
	virtual void ReleaseUploadBuffers();

	virtual void CheckCollision();
	virtual void StartScene();

	virtual int MouseClick();

	virtual void RenderUI(ID3D12GraphicsCommandList *pd3dCommandList);

	virtual void AddRoom(int n);
	virtual void DeleteRoom(int n);
	virtual void ChangeRoomInfo(int index, int map, int people);
	virtual int GetSelectRoom() { return m_Rooms[m_nSelectRoom].nRoom_num; }

protected:
	BoundingBox		m_CreateRoomButton;
	bool			m_bHLCreateRoomButton = false;

	BoundingBox		m_UpButton;
	bool			m_bHLUpButton = false;

	BoundingBox		m_DownButton;
	bool			m_bHLDownButton = false;

	BoundingBox		m_RoomButton[8];
	bool			m_bHLRoomButton[8];

	int				m_RoomStart = 0;
	std::vector<ROOM_INFO_TEXT> m_Rooms;

	int				m_nSelectRoom = -1;
};

//////////////////////////////////////////////////////////////////////////////////////////////////

struct ROOM_PLAYER_INFO
{
	CTextObject	*m_pTextObject;
	int			m_nRobotType;
	int			m_nSlot;
	bool		m_bUsed;
};

// UI Texture Index
#define LOBBY_ROOM_UI_TEXTURE_COUNT 29

#define LOBBY_ROOM_UI_TEXTURE_BASE_MANAGER 0
#define LOBBY_ROOM_UI_TEXTURE_BASE_MEMBER 1
#define LOBBY_ROOM_UI_TEXTURE_START 2
#define LOBBY_ROOM_UI_TEXTURE_START_HL 3
#define LOBBY_ROOM_UI_TEXTURE_READY 4
#define LOBBY_ROOM_UI_TEXTURE_READY_HL 5
#define LOBBY_ROOM_UI_TEXTURE_LEAVE 6
#define LOBBY_ROOM_UI_TEXTURE_LEAVE_HL 7
#define LOBBY_ROOM_UI_TEXTURE_COLONY 8
#define LOBBY_ROOM_UI_TEXTURE_COLONY_HL 9
#define LOBBY_ROOM_UI_TEXTURE_COLONY_SELECT 10
#define LOBBY_ROOM_UI_TEXTURE_SPACE 11
#define LOBBY_ROOM_UI_TEXTURE_SPACE_HL 12
#define LOBBY_ROOM_UI_TEXTURE_SPACE_SELECT 13
#define LOBBY_ROOM_UI_TEXTURE_GM 14
#define LOBBY_ROOM_UI_TEXTURE_GM_HL 15
#define LOBBY_ROOM_UI_TEXTURE_GM_SELECT 16
#define LOBBY_ROOM_UI_TEXTURE_GUNDAM 17
#define LOBBY_ROOM_UI_TEXTURE_GUNDAM_HL 18
#define LOBBY_ROOM_UI_TEXTURE_GUNDAM_SELECT 19
#define LOBBY_ROOM_UI_TEXTURE_ZAKU 20
#define LOBBY_ROOM_UI_TEXTURE_ZAKU_HL 21
#define LOBBY_ROOM_UI_TEXTURE_ZAKU_SELECT 22
#define LOBBY_ROOM_UI_TEXTURE_MAP_COLONY 23
#define LOBBY_ROOM_UI_TEXTURE_MAP_SPACE 24
#define LOBBY_ROOM_UI_TEXTURE_TEAM_RED 25
#define LOBBY_ROOM_UI_TEXTURE_TEAM_RED_HL 26
#define LOBBY_ROOM_UI_TEXTURE_TEAM_BLUE 27
#define LOBBY_ROOM_UI_TEXTURE_TEAM_BLUE_HL 28

// UI Rect Index
#define LOBBY_ROOM_UI_RECT_COUNT 11

#define LOBBY_ROOM_UI_RECT_BASE 0
#define LOBBY_ROOM_UI_RECT_START_BUTTON 1
#define LOBBY_ROOM_UI_RECT_READY_BUTTON 2
#define LOBBY_ROOM_UI_RECT_LEAVE_BUTTON 3
#define LOBBY_ROOM_UI_RECT_COLONY 4
#define LOBBY_ROOM_UI_RECT_SPACE 5
#define LOBBY_ROOM_UI_RECT_GM 6
#define LOBBY_ROOM_UI_RECT_GUNDAM 7
#define LOBBY_ROOM_UI_RECT_ZAKU 8
#define LOBBY_ROOM_UI_RECT_TEAM_RED 9
#define LOBBY_ROOM_UI_RECT_TEAM_BLUE 10

class CLobbyRoomScene : public CLobbyScene
{
public:
	CLobbyRoomScene();
	virtual ~CLobbyRoomScene();

	virtual void BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, CRepository *pRepository);
	virtual void SetAfterBuildObject(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, void *pContext);
	virtual void ReleaseObjects();
	virtual void ReleaseUploadBuffers();

	virtual void CheckCollision();
	virtual void StartScene();

	virtual void JoinPlayer(int nIndex, int nSlot, const wchar_t *pstrPlayerName, int nRobotType);
	virtual void LeavePlayer(int nIndex);
	virtual void SetClientIndex(int nIndex, int nSlot);
	virtual void SetMap(int nMap);
	virtual void ChangeSelectRobot(int nIndex, int nRobotType);
	virtual void ChangeMap(int nMap) { m_nCurrentMap = nMap; }
	virtual void ChangeSlot(int nIndex, int nChangeSlot);
	virtual int MouseClick();

	XMFLOAT2 GetPlayerTextPosition(int nServerIndex);

	virtual void RenderUI(ID3D12GraphicsCommandList *pd3dCommandList);
	virtual int GetSelectedMap() { return m_nCurrentMap; }
	virtual void GetTeamsInfo(int nTeam, std::vector<int> &vnIndices, std::vector<wchar_t*> &vpwstrNames);

protected:
	int									m_nCurrentMap = SCENE_TYPE_COLONY;
	ROOM_PLAYER_INFO					m_pPlayerInfos[8];

	CRect								*m_pPlayerRobotRects[8];

protected:
	BoundingBox		m_StartButton;
	bool			m_bHLStartButton = false;

	BoundingBox		m_ReadyButton;
	bool			m_bHLReadyButton = false;

	BoundingBox		m_LeaveButton;
	bool			m_bHLLeaveButton = false;

	BoundingBox		m_ColonyButton;
	bool			m_bHLColonyButton = false;

	BoundingBox		m_SpaceButton;
	bool			m_bHLSpaceButton = false;

	BoundingBox		m_GMButton;
	bool			m_bHLGMButton = false;

	BoundingBox		m_GundamButton;
	bool			m_bHLGundamButton = false;

	BoundingBox		m_ZakuButton;
	bool			m_bHLZakuButton = false;

	CRect			*m_MapRect = NULL;

	BoundingBox		m_TeamRedButton;
	bool			m_bHLTeamRedButton = false;

	BoundingBox		m_TeamBlueButton;
	bool			m_bHLTeamBlueButton = false;

	std::vector<char*> m_vstrPlayerNames;
};

//////////////////////////////////////////////////////////////////////////////////////////////////

// Scene's Shader Index
#define SHADER_INDEX 3

#define INDEX_SHADER_STANDARD_OBJECTS 0
#define INDEX_SHADER_INSTANCING_OBJECTS 1
#define INDEX_SHADER_SKINND_OBJECTS 2

// Standard Shader's Object Group
#define STANDARD_OBJECT_GROUP 6

#define STANDARD_OBJECT_INDEX_GG_BULLET 0
#define STANDARD_OBJECT_INDEX_BZK_BULLET 1
#define STANDARD_OBJECT_INDEX_MG_BULLET 2
#define STANDARD_OBJECT_INDEX_REPAIR_ITEM 3
#define STANDARD_OBJECT_INDEX_AMMO_ITEM 4
#define STANDARD_OBJECT_INDEX_METEOR 5

// Standard Shader's Object Group
#define SKINNED_OBJECT_GROUP 3

#define SKINNED_OBJECT_INDEX_GM 0
#define SKINNED_OBJECT_INDEX_GUNDAM 1
#define SKINNED_OBJECT_INDEX_ZAKU 2

// Scene's Effect Shader Index
#define EFFECT_SHADER_INDEX 3

#define INDEX_SHADER_TIMED_EEFECTS 0
#define INDEX_SHADER_SPRITE_EFFECTS 1
#define INDEX_SHADER_TEXT_EEFECTS 2

struct MOTIONBLUR
{
	XMFLOAT4X4 m_xmf4x4PrevViewProj;
	XMFLOAT4X4 m_xmf4x4InverseViewProj;
	int m_nWidth;
	int m_nHeight;
	int m_nSamples;
};

class CBattleScene : public CScene
{
public:
	CBattleScene();
	virtual ~CBattleScene();

	virtual int OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM	lParam);
	virtual void OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);

	virtual void BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, CRepository *pRepository);
	virtual void BuildObstacleObjetcs(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, CRepository *pRepository) {}
	virtual void BuildTerrain(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList) {}
	virtual void BuildSkybox(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList) {}
	virtual void ReleaseObjects();
	virtual void ReleaseUploadBuffers();
	virtual void AnimateObjects(float fTimeElapsed, CCamera *pCamera);
	virtual void SetAfterBuildObject(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, void *pContext);

	virtual void CheckCollision();
	virtual void CheckCollisionPlayer();
	virtual void FindAimToTargetDistance();

	virtual void BuildLightsAndMaterials() {}
	virtual void CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList) {}
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList) {}
	virtual void ReleaseShaderVariables() {}

	virtual void StartScene() {}
	virtual void EndScene() {}
	virtual void ProcessAlert(float fElapsedTime);
	virtual void Alert();

	virtual void PrepareRender(ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera);
	virtual void RenderUI(ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void RenderCubeMap(ID3D12GraphicsCommandList *pd3dCommandList, CGameObject *pMainObject);
	virtual void RenderShadowMap(ID3D12GraphicsCommandList *pd3dCommandList) {}
	virtual void RenderOffScreen(ID3D12GraphicsCommandList *pd3dCommandList);

	void CreateEnvironmentMap(ID3D12Device *pd3dDevice);
	void CreateCubeMapCamera(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	void CreateRtvDsvSrvEnvironmentMap(ID3D12Device *pd3dDevice);

	void CreateShadowMap(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, int nWidth, int nHeight);
	void CreateDsvSrvShadowMap(ID3D12Device *pd3dDevice);

	virtual void CreateLightCamera(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, int nWidth, int nHeight) {}

	void CreateNameTextures(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	void CreateNameTexture(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12Resource **pd3dResource, int nWidth, int nHeight);
	virtual void AddTeam(int nIndex, wchar_t *pstrName) { m_vwstrTeamName.emplace_back(pstrName); m_vTeamIndex.emplace_back(nIndex); }

protected:
	ID3D12Resource					*m_pd3dEnvirCube = NULL;
	ID3D12Resource					*m_pd3dEnvirCubeDSBuffer = NULL;
	D3D12_CPU_DESCRIPTOR_HANDLE		m_d3dRrvEnvirCubeMapCPUHandle[6];
	D3D12_CPU_DESCRIPTOR_HANDLE		m_d3dDsvEnvirCubeMapCPUHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE		m_d3dSrvEnvirCubeMapGPUHandle;

	CCamera							*m_pCubeMapCamera[6];

	ID3D12Resource					*m_pd3dShadowMap = NULL;
	D3D12_CPU_DESCRIPTOR_HANDLE		m_d3dDsvShadowMapCPUHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE		m_d3dSrvShadowMapGPUHandle;

	CLightCamera					*m_pLightCamera = NULL;

protected:
	int								m_nRedScore = 0;
	int								m_nBlueScore = 0;

	CTextObject						*m_pRedScoreText = NULL;
	CTextObject						*m_pBlueScoreText = NULL;

	float							m_fGravAcc = 0.0f;
	float							m_fCameraToTarget = 0.0f;

	XMFLOAT4						m_xmf4ScreenColor;
#define ALERT_COUNT 9
#define ALERT_SPEED 1.9f
	float							m_fAlertColor = 1.0f;
	float							m_fMulCalcAlertColor = 1.0f;
	int								m_fAlertCount = 0;
	bool							m_bAlert = false;

	std::vector<std::wstring>		m_vwstrTeamName;
	std::vector<int>				m_vTeamIndex;

public:
	float GetToTargetDistance() { return m_fCameraToTarget; }

	void AddParticle(int nType, XMFLOAT3 xmf3Position, int nNum);

protected:
	CMinimapShader						*m_pMinimapShader = NULL;

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
	CModel		*m_pTomahawk = NULL;
};

//////////////////////////////////////////////////////////////////////////////////////////////////

class CColonyScene : public CBattleScene
{
public:
	CColonyScene();
	virtual ~CColonyScene();

	virtual void BuildObstacleObjetcs(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, CRepository *pRepository);
	virtual void BuildTerrain(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void BuildSkybox(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void BuildLightsAndMaterials();
	virtual void CreateLightCamera(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, int nWidth, int nHeight);

	virtual void CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void ReleaseShaderVariables();

	virtual void RenderShadowMap(ID3D12GraphicsCommandList *pd3dCommandList);

	virtual void StartScene();
	virtual void EndScene();

	virtual void RenderUI(ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void ApplyRecvInfo(PKT_ID pktID, LPVOID pktData);

};

//////////////////////////////////////////////////////////////////////////////////////////////////

class CSpaceScene : public CBattleScene
{
public:
	CSpaceScene();
	virtual ~CSpaceScene();

	virtual void BuildObstacleObjetcs(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, CRepository *pRepository);
	virtual void BuildSkybox(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void BuildLightsAndMaterials();
	virtual void CreateLightCamera(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, int nWidth, int nHeight);

	virtual void CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void ReleaseShaderVariables();

	virtual void RenderShadowMap(ID3D12GraphicsCommandList *pd3dCommandList);

	virtual void StartScene();
	virtual void EndScene();

	virtual void RenderUI(ID3D12GraphicsCommandList *pd3dCommandList);

	virtual void CheckCollision();
	virtual void ApplyRecvInfo(PKT_ID pktID, LPVOID pktData);

};