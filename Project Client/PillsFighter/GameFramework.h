#pragma once

#include "Timer.h"
#include "Scene.h"
#include "Player.h"
#include "Repository.h"

class CGameFramework;

class CGameFramework
{
private:
	HINSTANCE						m_hInstance;
	HWND							m_hWnd;

	IDXGIFactory4					*m_pdxgiFactory = NULL;
	IDXGISwapChain3					*m_pdxgiSwapChain = NULL;
	ID3D12Device					*m_pd3dDevice = NULL;

	bool							m_bMsaa4xEnable = false;
	UINT							m_nMsaa4xQualityLevels = 0;

	static const UINT				m_nSwapChainBuffers = 2;
	UINT							m_nSwapChainBufferIndex;

	ID3D12Resource					*m_ppd3dRenderTargetBuffers[m_nSwapChainBuffers];

	ID3D12Resource					*m_pd3dDepthStencilBuffer = NULL;

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
	POINT							m_ptCursorPos;

	CScene							*m_pScene = NULL;

	CPlayer							*m_pPlayer = NULL;
	CCamera							*m_pCamera = NULL;

	ID3D12DescriptorHeap			*m_pd3dRtvDescriptorHeap;
	ID3D12DescriptorHeap			*m_pd3dDsvDescriptorHeap;

public:
	CGameFramework();
	~CGameFramework();

	//프레임워크를 초기화하는 함수이다(주 윈도우가 생성되면 호출된다).
	bool OnCreate(HINSTANCE hInstance, HWND hMainWnd, SOCKET sock);
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
	void BuildScene(int nSceneType = 0);
	void BuildBattleScene(int nType);
	void BuildLobbyMainScene();
	void BuildLobbyRoomScene();
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
	void ProcessSceneReturnVal(int n);

protected:
	CRepository						*m_pRepository = NULL;
	
protected:
	CFont							m_Font;
	bool							m_bWireRender = false;

protected: // for Network
	SOCKET	m_Socket;

	char	m_RecvBuf[MAX_BUFFER];

	int		m_nPrevSize;
	char	m_pPacketBuffer[MAX_PACKET];

	int		m_nClinetIndex;
	int		m_nClientSlot;
	bool	m_bDrawScene;
	bool	m_bSend_Complete;

	float	m_fElapsedTime;
	int		m_nFrameRate;
	float	m_fFPSTimeElapsed;
	int		m_nFramePerSecond;

public:
	void recvn();
	void ProcessPacket();
	void SendToServer();
	void SendToServer(PKT_ID pktID, void *pData);
	void OnProcessingSocketMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	void CreateObject(PKT_CREATE_OBJECT *pCreateObjectInfo);
	void CreateEffect(PKT_CREATE_EFFECT *pCreateEffectInfo);
};