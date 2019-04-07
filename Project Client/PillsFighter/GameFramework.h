#pragma once

#include "Timer.h"
#include "Scene.h"
#include "Player.h"
#include "Repository.h"

class CGameFramework;

struct FrameworkThread
{
	CGameFramework* pGFW;
	SOCKET			sock;
};

class CGameFramework
{
private:
	HINSTANCE						m_hInstance;
	HWND							m_hWnd;

	int								m_nWndClientWidth;
	int								m_nWndClientHeight;

	IDXGIFactory4					*m_pdxgiFactory = NULL;
	IDXGISwapChain3					*m_pdxgiSwapChain = NULL;
	ID3D12Device					*m_pd3dDevice = NULL;

	bool							m_bMsaa4xEnable = false;
	UINT							m_nMsaa4xQualityLevels = 0;

	static const UINT				m_nSwapChainBuffers = 2;
	UINT							m_nSwapChainBufferIndex;

	ID3D12Resource					*m_ppd3dRenderTargetBuffers[m_nSwapChainBuffers];
	ID3D12DescriptorHeap			*m_pd3dRtvDescriptorHeap = NULL;
	UINT							m_nRtvDescriptorIncrementSize;

	ID3D12Resource					*m_pd3dDepthStencilBuffer = NULL;
	ID3D12DescriptorHeap			*m_pd3dDsvDescriptorHeap = NULL;
	UINT							m_nDsvDescriptorIncrementSize;

	ID3D12CommandQueue				*m_pd3dCommandQueue = NULL;
	ID3D12CommandAllocator			*m_pd3dCommandAllocator = NULL;
	ID3D12GraphicsCommandList		*m_pd3dCommandList = NULL;

	ID3D12Fence						*m_pd3dFence = NULL;
	UINT64							m_nFenceValues[m_nSwapChainBuffers];
	HANDLE							m_hFenceEvent;

#if defined(_DEBUG)
	ID3D12Debug						*m_pd3dDebugController;
#endif

	CGameTimer						m_GameTimer;
	_TCHAR							m_pszCaption[70];

	POINT							m_ptOldCursorPos;
	RECT							m_wndRect;
	POINT							m_ptCursorPos;
	BOOL							m_LButtonDown = FALSE;
	BOOL							m_bRenderWire = FALSE;

	CScene							*m_pScene = NULL;
	CPlayer							*m_pPlayer = NULL;
	CCamera							*m_pCamera = NULL;

public:
	CGameFramework();
	~CGameFramework();

	//프레임워크를 초기화하는 함수이다(주 윈도우가 생성되면 호출된다).
	bool OnCreate(HINSTANCE hInstance, HWND hMainWnd);
	void OnDestroy();

	//스왑 체인, 디바이스, 서술자 힙, 명령 큐/할당자/리스트를 생성하는 함수이다. 
	void CreateSwapChain();
	void CreateDirect3DDevice();
	void CreateRtvAndDsvDescriptorHeaps();
	void CreateRenderTargetViews();
	void CreateDepthStencilView();
	void CreateCommandQueueAndList();

	void OnResizeBackBuffers();

	//렌더링할 메쉬와 게임 객체를 생성하고 소멸하는 함수이다. 
	void BuildObjects();
	void BuildScene(SCENEINFO *pSI = NULL);
	void ReleaseObjects();

	//프레임워크의 핵심(사용자 입력, 애니메이션, 렌더링)을 구성하는 함수이다. 
	void ProcessInput();
	void AnimateObjects(float fElapsedTime);
	void FrameAdvance();

	//CPU와 GPU를 동기화하는 함수이다. 
	void WaitForGpuComplete();
	void MoveToNextFrame();

	//윈도우의 메시지(키보드, 마우스 입력)를 처리하는 함수이다.
	void OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	void OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	LRESULT CALLBACK OnProcessingWindowMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);

protected:
	CRepository	 *m_pRepository = NULL;

	int								m_nCubeMapWidth;
	int								m_nCubeMapHeight;
	ID3D12Resource					*m_pd3dEnvirCube;
	ID3D12Resource					*m_pd3dEnvirCubeDSBuffer = NULL;
	D3D12_CPU_DESCRIPTOR_HANDLE		m_d3dRrvEnvirCubeMapCPUHandle[6];
	D3D12_CPU_DESCRIPTOR_HANDLE		m_d3dDsvEnvirCubeMapCPUHandle;

	CCamera							*m_pCubeMapCamera[6];

public:
	void CreateEnvironmentMap();
	void CreateCubeMapCamera();

public: // for Network
	HANDLE m_hThread;
	SOCKET m_sock;

	std::mutex m_Mutex;
	std::vector<PKT_PLAYER_INFO*> m_vMsgPlayerInfo;
	std::vector<PKT_PLAYER_LIFE*> m_vMsgPlayerLife;
	std::vector<PKT_CREATE_OBJECT*> m_vMsgCreateObject;
	std::vector<PKT_DELETE_OBJECT*> m_vMsgDeleteObject;
	std::vector<PKT_TIME_INFO*> m_vMsgTimeInfo;
	std::vector<PKT_UPDATE_OBJECT*> m_vMsgUpdateInfo;
	std::vector<PKT_CREATE_EFFECT*> m_vMsgCreateEffect;

	//통신
	CLIENTID m_Client_Info;
	PKT_PLAYER_INFO m_Client_Player_Info;
	float m_fElapsedTime = 0.0f;
	HANDLE hEvent;
	bool SendComplete = false;
	bool initsend = false;
	void CloseNetwork();

	void err_quit(char* msg);
	void err_display(char* msg);
	int recvn(SOCKET s, char * buf, int len, int flags);

	void InitNetwork();
	static DWORD WINAPI recvThread(LPVOID arg);
	DWORD ThreadFunc(LPVOID arg);

	void CreateObject(PKT_CREATE_OBJECT CreateObjectInfo);
	void CreateEffect(PKT_CREATE_EFFECT CreateEffectInfo);

	char *buf;

	float m_fFrameRate = 0.0f;
	float m_fFPSTimeElapsed = 0.0f;
	int m_nFramePerSecond = 0;
	volatile bool gamestart = false;
};