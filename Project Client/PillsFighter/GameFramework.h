#pragma once

#include "Timer.h"
#include "Scene.h"
#include "Player.h"

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

	//렌더 타겟 버퍼, 서술자 힙 인터페이스 포인터, 렌더 타겟 서술자 원소의 크기이다. 
	ID3D12Resource					*m_ppd3dRenderTargetBuffers[m_nSwapChainBuffers];
	ID3D12DescriptorHeap			*m_pd3dRtvDescriptorHeap = NULL;
	UINT							m_nRtvDescriptorIncrementSize;

	//깊이-스텐실 버퍼, 서술자 힙 인터페이스 포인터, 깊이-스텐실 서술자 원소의 크기이다. 
	ID3D12Resource					*m_pd3dDepthStencilBuffer = NULL;
	ID3D12DescriptorHeap			*m_pd3dDsvDescriptorHeap = NULL;
	UINT							m_nDsvDescriptorIncrementSize;

	//명령 큐, 명령 할당자, 명령 리스트 인터페이스 포인터이다. 
	ID3D12CommandQueue				*m_pd3dCommandQueue = NULL;
	ID3D12CommandAllocator			*m_pd3dCommandAllocator = NULL;
	ID3D12GraphicsCommandList		*m_pd3dCommandList = NULL;

	//펜스 인터페이스 포인터, 펜스의 값, 이벤트 핸들이다.
	ID3D12Fence						*m_pd3dFence = NULL;
	UINT64							m_nFenceValues[m_nSwapChainBuffers];
	HANDLE							m_hFenceEvent;

#if defined(_DEBUG)
	ID3D12Debug						*m_pd3dDebugController;
#endif

	CGameTimer						m_GameTimer;
	_TCHAR							m_pszFrameRate[50];

	POINT							m_ptOldCursorPos;

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
	void ReleaseObjects();

	//프레임워크의 핵심(사용자 입력, 애니메이션, 렌더링)을 구성하는 함수이다. 
	void ProcessInput();
	void AnimateObjects();
	void FrameAdvance();

	//CPU와 GPU를 동기화하는 함수이다. 
	void WaitForGpuComplete();
	void MoveToNextFrame();

	//윈도우의 메시지(키보드, 마우스 입력)를 처리하는 함수이다.
	void OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	void OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	LRESULT CALLBACK OnProcessingWindowMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
};

