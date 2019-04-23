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

	//�����ӿ�ũ�� �ʱ�ȭ�ϴ� �Լ��̴�(�� �����찡 �����Ǹ� ȣ��ȴ�).
	bool OnCreate(HINSTANCE hInstance, HWND hMainWnd, SOCKET sock);
	void OnDestroy();

	//���� ü��, ����̽�, ������ ��, ��� ť/�Ҵ���/����Ʈ�� �����ϴ� �Լ��̴�. 
	void CreateSwapChain();
	void CreateDirect3DDevice();
	void CreateRtvAndDsvDescriptorHeaps();
	void CreateRenderTargetViews();
	void CreateDepthStencilView();
	void CreateCommandQueueAndList();

	void OnResizeBackBuffers();

	//�������� �޽��� ���� ��ü�� �����ϰ� �Ҹ��ϴ� �Լ��̴�. 
	void BuildObjects();
	void BuildScene(int nSceneType = 0);
	void BuildColonyScene();
	void BuildLobbyScene();
	void ReleaseObjects();

	//�����ӿ�ũ�� �ٽ�(����� �Է�, �ִϸ��̼�, ������)�� �����ϴ� �Լ��̴�. 
	void ProcessInput();
	void AnimateObjects(float fElapsedTime);
	void FrameAdvance();

	//CPU�� GPU�� ����ȭ�ϴ� �Լ��̴�. 
	void WaitForGpuComplete();
	void MoveToNextFrame();

	//�������� �޽���(Ű����, ���콺 �Է�)�� ó���ϴ� �Լ��̴�.
	void OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	void OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	LRESULT CALLBACK OnProcessingWindowMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);

protected:
	CRepository						*m_pRepository = NULL;

	D3D12_VIEWPORT 					m_d3dViewport;
	D3D12_RECT						m_d3dScissorRect;
	
protected:
	CFont							m_Arial;
	CFont							m_HumanMagic;

	int	m_nCharacterIndex = 0;

protected: // for Network
	SOCKET	m_Socket;

	char	m_RecvBuf[MAX_BUFFER];

	int		m_nPacketSize = 0;
	char	m_pPacketBuffer[MAX_PACKET];

	int		m_nClinetIndex = -1;
	bool	m_bDrawScene = true;

	float	m_fElapsedTime = 0.0f;
	float	m_fFrameRate = 0.0f;
	float	m_fFPSTimeElapsed = 0.0f;
	int		m_nFramePerSecond = 0;

public:
	void recvn();
	void ProcessPacket();
	void SendToServer();
	void SendToServer(PKT_ID pktID);
	void OnProcessingSocketMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	void CreateObject(PKT_CREATE_OBJECT *pCreateObjectInfo);
	void CreateEffect(PKT_CREATE_EFFECT *pCreateEffectInfo);

	//���
	PKT_PLAYER_INFO m_Client_Player_Info;
	HANDLE hEvent;

	void InitNetwork();
};