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

	CTextSystem						m_TextSystem;

public:
	CGameFramework();
	~CGameFramework();

	bool OnCreate(HINSTANCE hInstance, HWND hMainWnd);
	void OnDestroy();

	void CreateSwapChain();
	void CreateDirect3DDevice();
	void CreateRtvAndDsvDescriptorHeaps();
	void CreateRenderTargetViews();
	void CreateDepthStencilView();
	void CreateCommandQueueAndList();

	void OnResizeBackBuffers();

	void BuildObjects();
	void BuildScene(int nSceneType = 0);
	void BuildBattleScene(int nType);
	void BuildTitleScene();
	void BuildLobbyMainScene();
	void BuildLobbyRoomScene();
	void ReleaseObjects();
	void ReleaseScene();

	void ProcessInput();
	void AnimateObjects(float fElapsedTime);
	void FrameAdvance();

	void WaitForGpuComplete();
	void MoveToNextFrame();

	void OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	void OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	LRESULT CALLBACK OnProcessingWindowMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	void ProcessSceneReturnVal(int n);

protected:
	CRepository						*m_pRepository = NULL;
	
protected:
	CFont							m_Font;
	bool							m_bWireRender = false;
	bool							m_bBGMStop = true;

protected: // for Network
	char	m_RecvBuf[MAX_BUFFER];

	int		m_nPrevSize;
	char	m_pPacketBuffer[MAX_PACKET];

	int		m_nClientSlot;

	float	m_fElapsedTime;
	int		m_nFrameRate;
	float	m_fFPSTimeElapsed;
	int		m_nFramePerSecond;

	std::vector<int> m_vnIndices;
	std::vector<int> m_vnEnemyIndices;
	std::vector<wchar_t*> m_vpwstrNames;

public:
	void recvn();
	void ProcessPacket();
	void SendToServer();
	void SendToServer(PKT_ID pktID, void *pData);
	void OnProcessingSocketMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	void CreateObject(PKT_CREATE_OBJECT *pCreateObjectInfo);
	void CreateEffect(PKT_CREATE_EFFECT *pCreateEffectInfo);
};