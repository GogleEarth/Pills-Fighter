#include "stdafx.h"
#include "GameFramework.h"
#include "Weapon.h"

////////////////////////////////////////////////////////////////

extern CFMODSound gFmodSound;

CGameFramework::CGameFramework()
{
	m_pdxgiFactory = NULL;
	m_pdxgiSwapChain = NULL;
	m_pd3dDevice = NULL;

	for (int i = 0; i < m_nSwapChainBuffers; i++) m_ppd3dRenderTargetBuffers[i] = NULL;
	m_nSwapChainBufferIndex = 0;

	m_pd3dCommandAllocator = NULL;
	m_pd3dCommandQueue = NULL;
	m_pd3dCommandList = NULL;

	m_pd3dRtvDescriptorHeap = NULL;
	m_pd3dDsvDescriptorHeap = NULL;

	m_hFenceEvent = NULL;
	m_pd3dFence = NULL;
	for (int i = 0; i < m_nSwapChainBuffers; i++) m_nFenceValues[i] = 0;

	gnWndClientWidth = FRAME_BUFFER_WIDTH;
	gnWndClientHeight = FRAME_BUFFER_HEIGHT;

	m_pScene = NULL;
	m_pPlayer = NULL;

	_tcscpy_s(m_pszCaption, _T(GAME_TITLE));

	::ZeroMemory(m_RecvBuf, sizeof(m_RecvBuf));
	m_nPrevSize = 0;
	::ZeroMemory(m_pPacketBuffer, sizeof(m_pPacketBuffer));

	m_fElapsedTime = 0.0f;
	m_nFrameRate = 0;
	m_fFPSTimeElapsed = 0.0f;
	m_nFramePerSecond = 0;
}

CGameFramework::~CGameFramework()
{
}

bool CGameFramework::OnCreate(HINSTANCE hInstance, HWND hMainWnd)
{
	m_hInstance = hInstance;
	m_hWnd = hMainWnd;
	m_TextSystem.Initialize(m_hWnd);

	CreateDirect3DDevice();
	CreateCommandQueueAndList();
	CreateRtvAndDsvDescriptorHeaps();
	CreateSwapChain();

	BuildObjects();
	   
	POINT pt;
	pt.x = FRAME_BUFFER_WIDTH / 2;
	pt.y = FRAME_BUFFER_HEIGHT / 2;

	ClientToScreen(hMainWnd, &pt);
	SetCursorPos(pt.x, pt.y);

	::SetCapture(hMainWnd);
	::GetCursorPos(&m_ptOldCursorPos);

	return(true);
}

void CGameFramework::OnDestroy()
{
	ReleaseObjects();

	CScene::ReleaseDescHeapsAndRootSignature();
	if (m_pRepository) delete m_pRepository;
	m_Font.Destroy();

	::CloseHandle(m_hFenceEvent);

	m_TextSystem.Destroy(m_hWnd);

#if defined(_DEBUG)
	if (m_pd3dDebugController) m_pd3dDebugController->Release();
#endif

	for (int i = 0; i < m_nSwapChainBuffers; i++) if (m_ppd3dRenderTargetBuffers[i]) m_ppd3dRenderTargetBuffers[i]->Release();
	if (m_pd3dRtvDescriptorHeap) m_pd3dRtvDescriptorHeap->Release();

	if (m_pd3dDepthStencilBuffer) m_pd3dDepthStencilBuffer->Release();
	if (m_pd3dDsvDescriptorHeap) m_pd3dDsvDescriptorHeap->Release();

	if (m_pd3dCommandAllocator) m_pd3dCommandAllocator->Release();
	if (m_pd3dCommandQueue) m_pd3dCommandQueue->Release();
	if (m_pd3dCommandList) m_pd3dCommandList->Release();

	if (m_pd3dFence) m_pd3dFence->Release();

	m_pdxgiSwapChain->SetFullscreenState(FALSE, NULL);
	if (m_pdxgiSwapChain) m_pdxgiSwapChain->Release();
	if (m_pd3dDevice) m_pd3dDevice->Release();
	if (m_pdxgiFactory) m_pdxgiFactory->Release();

	::ReleaseCapture();
}

void CGameFramework::CreateSwapChain()
{
	RECT rcClient;
	::GetClientRect(m_hWnd, &rcClient);
	gnWndClientWidth = rcClient.right - rcClient.left;
	gnWndClientHeight = rcClient.bottom - rcClient.top;

	DXGI_SWAP_CHAIN_DESC dxgiSwapChainDesc;
	::ZeroMemory(&dxgiSwapChainDesc, sizeof(dxgiSwapChainDesc));
	dxgiSwapChainDesc.BufferCount = m_nSwapChainBuffers;
	dxgiSwapChainDesc.BufferDesc.Width = gnWndClientWidth;
	dxgiSwapChainDesc.BufferDesc.Height = gnWndClientHeight;
	dxgiSwapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	dxgiSwapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
	dxgiSwapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	dxgiSwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	dxgiSwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	dxgiSwapChainDesc.OutputWindow = m_hWnd;
	dxgiSwapChainDesc.SampleDesc.Count = (m_bMsaa4xEnable) ? 4 : 1;
	dxgiSwapChainDesc.SampleDesc.Quality = (m_bMsaa4xEnable) ? (m_nMsaa4xQualityLevels - 1) : 0;
	dxgiSwapChainDesc.Windowed = TRUE;
#ifdef _WITH_ONLY_RESIZE_BACKBUFFERS
	//전체화면 모드에서 바탕화면의 해상도를 바꾸지 않고 후면버퍼의 크기를 바탕화면 크기로 변경한다. 
	dxgiSwapChainDesc.Flags = 0;
#else
	//전체화면 모드에서 바탕화면의 해상도를 스왑체인(후면버퍼)의 크기에 맞게 변경한다. 
	dxgiSwapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
#endif
	HRESULT hResult = m_pdxgiFactory->CreateSwapChain(m_pd3dCommandQueue,
		&dxgiSwapChainDesc, (IDXGISwapChain **)&m_pdxgiSwapChain);

	m_pdxgiFactory->MakeWindowAssociation(m_hWnd, DXGI_MWA_NO_ALT_ENTER);

	//스왑체인의 현재 후면버퍼 인덱스를 저장한다.
	m_nSwapChainBufferIndex = m_pdxgiSwapChain->GetCurrentBackBufferIndex();
}

void CGameFramework::CreateDirect3DDevice()
{
	HRESULT hResult;

	UINT nDXGIFactoryFlags = 0;
#if defined(_DEBUG)
	ID3D12Debug *pd3dDebugController = NULL;
	hResult = D3D12GetDebugInterface(__uuidof(ID3D12Debug), (void **)&pd3dDebugController);
	if (pd3dDebugController)
	{
		pd3dDebugController->EnableDebugLayer();
		pd3dDebugController->Release();
	}
	nDXGIFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
#endif

	hResult = ::CreateDXGIFactory2(nDXGIFactoryFlags, __uuidof(IDXGIFactory4), (void **)&m_pdxgiFactory);

	IDXGIAdapter1 *pd3dAdapter = NULL;

	for (UINT i = 0; DXGI_ERROR_NOT_FOUND != m_pdxgiFactory->EnumAdapters1(i, &pd3dAdapter); i++)
	{
		DXGI_ADAPTER_DESC1 dxgiAdapterDesc;
		pd3dAdapter->GetDesc1(&dxgiAdapterDesc);
		if (dxgiAdapterDesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) continue;
		if (SUCCEEDED(D3D12CreateDevice(pd3dAdapter, D3D_FEATURE_LEVEL_12_0, _uuidof(ID3D12Device), (void **)&m_pd3dDevice))) break;
	}

	if (!m_pd3dDevice)
	{
		hResult = m_pdxgiFactory->EnumWarpAdapter(_uuidof(IDXGIAdapter1), (void **)&pd3dAdapter);
		hResult = D3D12CreateDevice(pd3dAdapter, D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), (void **)&m_pd3dDevice);
	}

	if (!m_pd3dDevice)
	{
		MessageBox(NULL, L"Direct3D 12 Device Cannot be Created.", L"Error", MB_OK);
		::PostQuitMessage(0);
		return;
	}

	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS d3dMsaaQualityLevels;
	d3dMsaaQualityLevels.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	d3dMsaaQualityLevels.SampleCount = 4;
	d3dMsaaQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
	d3dMsaaQualityLevels.NumQualityLevels = 0;
	hResult = m_pd3dDevice->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &d3dMsaaQualityLevels, sizeof(D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS));
	m_nMsaa4xQualityLevels = d3dMsaaQualityLevels.NumQualityLevels;
	m_bMsaa4xEnable = (m_nMsaa4xQualityLevels > 1) ? true : false;

	hResult = m_pd3dDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, __uuidof(ID3D12Fence), (void **)&m_pd3dFence);
	for (UINT i = 0; i < m_nSwapChainBuffers; i++) m_nFenceValues[i] = 1;
	m_hFenceEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);

	::gnCbvSrvDescriptorIncrementSize = m_pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	::gnRtvDescriptorIncrementSize = m_pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	::gnDsvDescriptorIncrementSize = m_pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

	if (pd3dAdapter) pd3dAdapter->Release();
}

void CGameFramework::CreateCommandQueueAndList()
{
	D3D12_COMMAND_QUEUE_DESC d3dCommandQueueDesc;
	::ZeroMemory(&d3dCommandQueueDesc, sizeof(D3D12_COMMAND_QUEUE_DESC));
	d3dCommandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	d3dCommandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	HRESULT hResult = m_pd3dDevice->CreateCommandQueue(&d3dCommandQueueDesc,
		_uuidof(ID3D12CommandQueue), (void **)&m_pd3dCommandQueue);

	hResult = m_pd3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
		__uuidof(ID3D12CommandAllocator), (void **)&m_pd3dCommandAllocator);

	hResult = m_pd3dDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
		m_pd3dCommandAllocator, NULL, __uuidof(ID3D12GraphicsCommandList), (void
			**)&m_pd3dCommandList);

	//명령 리스트는 생성되면 열린(Open) 상태이므로 닫힌(Closed) 상태로 만든다. 
	hResult = m_pd3dCommandList->Close();
}

void CGameFramework::CreateRtvAndDsvDescriptorHeaps()
{
	D3D12_DESCRIPTOR_HEAP_DESC d3dDescriptorHeapDesc;
	::ZeroMemory(&d3dDescriptorHeapDesc, sizeof(D3D12_DESCRIPTOR_HEAP_DESC));
	d3dDescriptorHeapDesc.NumDescriptors = m_nSwapChainBuffers;
	d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	d3dDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	d3dDescriptorHeapDesc.NodeMask = 0;
	HRESULT hResult = m_pd3dDevice->CreateDescriptorHeap(&d3dDescriptorHeapDesc, __uuidof(ID3D12DescriptorHeap), (void **)&m_pd3dRtvDescriptorHeap);

	d3dDescriptorHeapDesc.NumDescriptors = 1;
	d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	hResult = m_pd3dDevice->CreateDescriptorHeap(&d3dDescriptorHeapDesc, __uuidof(ID3D12DescriptorHeap), (void **)&m_pd3dDsvDescriptorHeap);
}

void CGameFramework::CreateRenderTargetViews()
{
	D3D12_CPU_DESCRIPTOR_HANDLE d3dHandle = m_pd3dRtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();

	//스왑체인의 각 후면 버퍼에 대한 렌더 타겟 뷰를 생성한다.
	for (UINT i = 0; i < m_nSwapChainBuffers; i++)
	{
		m_pdxgiSwapChain->GetBuffer(i, __uuidof(ID3D12Resource), (void**)&m_ppd3dRenderTargetBuffers[i]);
		m_pd3dDevice->CreateRenderTargetView(m_ppd3dRenderTargetBuffers[i], NULL, d3dHandle);

		d3dHandle.ptr += ::gnRtvDescriptorIncrementSize;
	}
}

void CGameFramework::CreateDepthStencilView()
{
	D3D12_RESOURCE_DESC d3dResourceDesc;
	d3dResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	d3dResourceDesc.Alignment = 0;
	d3dResourceDesc.Width = gnWndClientWidth;
	d3dResourceDesc.Height = gnWndClientHeight;
	d3dResourceDesc.DepthOrArraySize = 1;
	d3dResourceDesc.MipLevels = 1;
	d3dResourceDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	d3dResourceDesc.SampleDesc.Count = (m_bMsaa4xEnable) ? 4 : 1;
	d3dResourceDesc.SampleDesc.Quality = (m_bMsaa4xEnable) ? (m_nMsaa4xQualityLevels - 1) : 0;
	d3dResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	d3dResourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_HEAP_PROPERTIES d3dHeapProperties;
	::ZeroMemory(&d3dHeapProperties, sizeof(D3D12_HEAP_PROPERTIES));
	d3dHeapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
	d3dHeapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	d3dHeapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	d3dHeapProperties.CreationNodeMask = 1;
	d3dHeapProperties.VisibleNodeMask = 1;

	D3D12_CLEAR_VALUE d3dClearValue;
	d3dClearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	d3dClearValue.DepthStencil.Depth = 1.0f;
	d3dClearValue.DepthStencil.Stencil = 0;

	m_pd3dDevice->CreateCommittedResource(&d3dHeapProperties, D3D12_HEAP_FLAG_NONE, &d3dResourceDesc, D3D12_RESOURCE_STATE_DEPTH_WRITE, &d3dClearValue, __uuidof(ID3D12Resource), (void **)&m_pd3dDepthStencilBuffer);

	D3D12_DEPTH_STENCIL_VIEW_DESC d3dDepthStencilViewDesc;
	::ZeroMemory(&d3dDepthStencilViewDesc, sizeof(D3D12_DEPTH_STENCIL_VIEW_DESC));
	d3dDepthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	d3dDepthStencilViewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	d3dDepthStencilViewDesc.Flags = D3D12_DSV_FLAG_NONE;

	m_pd3dDevice->CreateDepthStencilView(m_pd3dDepthStencilBuffer, &d3dDepthStencilViewDesc, m_pd3dDsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
}

void CGameFramework::BuildScene(int nSceneType)
{
	CScene::ResetDescriptorHeapHandles();

	m_pd3dCommandList->Reset(m_pd3dCommandAllocator, NULL);

	switch (nSceneType)
	{
	case SCENE_TYPE_TITLE:
		BuildTitleScene();
		break;
	case SCENE_TYPE_LOBBY_MAIN:
		BuildLobbyMainScene();
		break;
	case SCENE_TYPE_LOBBY_ROOM:
		BuildLobbyRoomScene();
		break;
	case SCENE_TYPE_COLONY:
	case SCENE_TYPE_SPACE:
		BuildBattleScene(nSceneType);
		break;
	}
	
	m_pScene->SetFont(m_pd3dDevice, &m_Font);
	m_pScene->SetTextSystem(&m_TextSystem);

	for (int i = 0; i < m_vnIndices.size(); i++)
	{
		m_pScene->AddTeam(m_vnIndices[i], m_vpwstrNames[i]);
	}
	m_vnIndices.clear();
	m_vpwstrNames.clear();

	for (int i = 0; i < m_vnEnemyIndices.size(); i++)
	{
		m_pScene->AddEnemy(m_vnEnemyIndices[i]);
	}
	m_vnEnemyIndices.clear();

	m_pScene->SetAfterBuildObject(m_pd3dDevice, m_pd3dCommandList, NULL);
	m_pScene->StartScene(m_bBGMStop);

	m_pd3dCommandList->Close();
	ID3D12CommandList *ppd3dCommandLists[] = { m_pd3dCommandList };
	m_pd3dCommandQueue->ExecuteCommandLists(1, ppd3dCommandLists);

	WaitForGpuComplete();

	if (m_pScene) m_pScene->ReleaseUploadBuffers();
	if (m_pPlayer) m_pPlayer->ReleaseUploadBuffers();
	if (m_pRepository) m_pRepository->ReleaseUploadBuffers();
}

void CGameFramework::BuildBattleScene(int nType)
{
	if(nType == SCENE_TYPE_COLONY)
		m_pScene = new CColonyScene();
	if(nType == SCENE_TYPE_SPACE)
		m_pScene = new CSpaceScene();

	m_pScene->SetMap(nType);
	m_pScene->BuildObjects(m_pd3dDevice, m_pd3dCommandList, m_pRepository);

	CPlayer *pPlayer = new CPlayer(m_pd3dDevice, m_pd3dCommandList, m_pScene->GetGraphicsRootSignature(), m_pRepository, m_pScene->GetTerrain(), m_pScene->GetPlayerRobotType());
	pPlayer->SetMovingSpeed(80.0f);
	pPlayer->SetHitPoint(100);

	m_pPlayer = pPlayer;
	m_pScene->SetPlayer(pPlayer);

	m_pCamera = m_pPlayer->GetCamera();
}

void CGameFramework::BuildTitleScene()
{
	m_pScene = new CTitleScene();

	m_pScene->BuildObjects(m_pd3dDevice, m_pd3dCommandList, m_pRepository);
}

void CGameFramework::BuildLobbyMainScene()
{
	m_pScene = new CLobbyMainScene();

	m_pScene->BuildObjects(m_pd3dDevice, m_pd3dCommandList, m_pRepository);
}

void CGameFramework::BuildLobbyRoomScene()
{
	m_pScene = new CLobbyRoomScene();

	m_pScene->BuildObjects(m_pd3dDevice, m_pd3dCommandList, m_pRepository);
}

void CGameFramework::BuildObjects()
{
	m_pd3dCommandList->Reset(m_pd3dCommandAllocator, NULL);

	CScene::CreateGraphicsRootSignature(m_pd3dDevice);
	CScene::CreateComputeRootSignature(m_pd3dDevice);
	CScene::CreateDescriptorHeaps(m_pd3dDevice, MAX_SRV_DESCRIPTOR_HEAP_COUNT);

	m_pRepository = new CRepository();

	m_Font.Initialize(m_pd3dDevice, m_pd3dCommandList, "./Resource/Font/Font.fnt");

	m_pd3dCommandList->Close();
	ID3D12CommandList *ppd3dCommandLists[] = { m_pd3dCommandList };
	m_pd3dCommandQueue->ExecuteCommandLists(1, ppd3dCommandLists);

	WaitForGpuComplete();

	m_Font.ReleaseUploadBuffers();

	::pDevice = m_pd3dDevice;
	::pCommandList = m_pd3dCommandList;

	BuildScene(SCENE_TYPE_TITLE);
}

void CGameFramework::ReleaseObjects()
{
	if (m_pPlayer)
	{
		delete m_pPlayer;

		m_pPlayer = NULL;
		m_pCamera = NULL;
	}

	if (m_pScene)
	{
		ReleaseScene();
	}
	
	m_Font.ClearTexts();
}

void CGameFramework::ReleaseScene()
{
	m_pScene->EndScene();
	m_pScene->ReleaseShaderVariables();
	m_pScene->ReleaseObjects();
	delete m_pScene;

	m_pScene = NULL;
}

void CGameFramework::ProcessSceneReturnVal(int n)
{
	switch (n)
	{
	case LOBBY_KEYDOWN_CREATE_ROOM:
	{
#ifdef ON_NETWORKING
		SendToServer(PKT_ID_CREATE_ROOM, m_pScene->GetRoomName());
#else
		XMFLOAT2 xmf2Pos = m_pScene->GetCursorPos();
		ReleaseScene();

		BuildScene(SCENE_TYPE_LOBBY_ROOM);
		m_pScene->SetCursorPosition(xmf2Pos);
#endif
		break;
	}
	case LOBBY_MOUSE_CLICK_JOIN_ROOM:
	{
#ifdef ON_NETWORKING
		SendToServer(PKT_ID_ROOM_IN, NULL);
#else
		std::cout << m_pScene->GetSelectRoom() << "\n";
#endif
		break;
	}
	case LOBBY_MOUSE_CLICK_ROOM_START:
	{
#ifdef ON_NETWORKING
		SendToServer(PKT_ID_GAME_START, NULL);
#else
		int map = m_pScene->GetSelectedMap();

		ReleaseScene();

		BuildScene(map);
#endif
		break;
	}
	case LOBBY_MOUSE_CLICK_ROOM_LEAVE:
	{
#ifdef ON_NETWORKING
		SendToServer(PKT_ID_LEAVE_ROOM, NULL);
#endif
		XMFLOAT2 xmf2Pos = m_pScene->GetCursorPos();
		m_bBGMStop = false;
		ReleaseScene();

		BuildScene(SCENE_TYPE_LOBBY_MAIN);
		m_pScene->SetCursorPosition(xmf2Pos);
		m_pScene->InitName(CScene::GetMyName());

		break;
	}
	case LOBBY_MOUSE_CLICK_ROOM_SELECT_ROBOT:
	{
#ifdef ON_NETWORKING
		SendToServer(PKT_ID_LOBBY_PLAYER_INFO, NULL);
#endif
		break;
	}
	case LOBBY_MOUSE_CLICK_ROOM_SELECT_MAP:
	{
#ifdef ON_NETWORKING
		SendToServer(PKT_ID_CHANGE_MAP, NULL);
#endif
		break;
	}
	case LOBBY_MOUSE_CLICK_ROOM_CHANGE_TEAM_RED:
	{
		int nTeam = TEAM_TYPE::TEAM_TYPE_RED;
		SendToServer(PKT_ID_MOVE_TEAM, &nTeam);
		break;
	}
	case LOBBY_MOUSE_CLICK_ROOM_CHANGE_TEAM_BLUE:
	{
		int nTeam = TEAM_TYPE::TEAM_TYPE_BLUE;
		SendToServer(PKT_ID_MOVE_TEAM, &nTeam);
		break;
	}
	case LOBBY_KEYDOWN_CHANGE_NAME:
	{
		SendToServer(PKT_ID_CHANGE_NAME, NULL);
		break;
	}
	case LOBBY_MOUSE_CLICK_GAME_START:
	{
#ifdef ON_NETWORKING
		WSAAsyncSelect(gSocket, m_hWnd, WM_SOCKET, FD_READ | FD_CLOSE);
#endif
		XMFLOAT2 xmf2Pos = m_pScene->GetCursorPos();
		ReleaseScene();

		m_bBGMStop = true;
		BuildScene(SCENE_TYPE_LOBBY_MAIN);
		m_pScene->SetCursorPosition(xmf2Pos);

		SendToServer(PKT_ID_LOG_IN, NULL);
		break;
	}
	case LOBBY_MOUSE_CLICK_GAME_EXIT:
	{		
		::PostQuitMessage(0);
		break;
	}
	case LOBBY_MOVE:
		if (m_pPlayer)
		{
			delete m_pPlayer;

			m_pPlayer = NULL;
			m_pCamera = NULL;
		}

		ReleaseScene();

		m_bBGMStop = true;
		BuildScene(SCENE_TYPE_LOBBY_MAIN);
		m_pScene->InitName(CScene::GetMyName());

#ifdef ON_NETWORKING
		SendToServer(PKT_ID_MOVE_TO_MAIN_LOBBY, NULL);
#endif
		break;
	}
}

void CGameFramework::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	int rev = 0;
	if (m_pScene) rev = m_pScene->OnProcessingMouseMessage(hWnd, nMessageID, wParam, lParam);
	if (rev) ProcessSceneReturnVal(rev);

	switch (nMessageID)
	{
	case WM_LBUTTONDOWN:
		if (::GetCapture() != m_hWnd)
		{
			::SetCapture(m_hWnd);
			::SetCursorPos(m_ptOldCursorPos.x, m_ptOldCursorPos.y);
		}
		break;
	case WM_RBUTTONDOWN:
		break;
	case WM_RBUTTONUP:
		break;
	case WM_MOUSEMOVE:
		m_ptCursorPos.x = LOWORD(lParam);
		m_ptCursorPos.y = HIWORD(lParam);
		break;
	default:
		break;
	}
}

void CGameFramework::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	int rev = 0;
	if (m_pScene) rev = m_pScene->OnProcessingKeyboardMessage(hWnd, nMessageID, wParam, lParam);
	if (rev) ProcessSceneReturnVal(rev);

	switch (nMessageID)
	{
	case WM_KEYUP:
		switch (wParam)
		{
		case VK_F1:
			break;
		case VK_ESCAPE:
			break;
		case VK_F8:
			break;
		case VK_F9: //“F9” 키가 눌려지면 윈도우 모드와 전체화면 모드의 전환을 처리한다
		{
			BOOL bFullScreenState = FALSE;
			m_pdxgiSwapChain->GetFullscreenState(&bFullScreenState, NULL);
			m_pdxgiSwapChain->SetFullscreenState(!bFullScreenState, NULL);
			DXGI_MODE_DESC dxgiTargetParameters;
			dxgiTargetParameters.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			dxgiTargetParameters.Width = gnWndClientWidth;
			dxgiTargetParameters.Height = gnWndClientHeight;
			dxgiTargetParameters.RefreshRate.Numerator = 60;
			dxgiTargetParameters.RefreshRate.Denominator = 1;
			dxgiTargetParameters.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
			dxgiTargetParameters.ScanlineOrdering =
				DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
			m_pdxgiSwapChain->ResizeTarget(&dxgiTargetParameters);
			OnResizeBackBuffers();
			break;
		}
		case VK_TAB:
			if (::GetCapture() == m_hWnd)
			{
				::GetCursorPos(&m_ptOldCursorPos);
				::ReleaseCapture();
			}
			else
			{
				::SetCapture(m_hWnd);
				::SetCursorPos(m_ptOldCursorPos.x, m_ptOldCursorPos.y);
			}
			break;
		case 'B':
			m_bWireRender = !m_bWireRender;
			break;
		default:
			break;
		}
		break;
	default:
		break;
	}
}

LRESULT CALLBACK CGameFramework::OnProcessingWindowMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	switch (nMessageID)
	{
	case WM_ACTIVATE:
	{
		//if (LOWORD(wParam) == WA_INACTIVE)
		//	m_GameTimer.Stop();
		//else
		//	m_GameTimer.Start();
		break;
	}
	case WM_SIZE:
	{
		gnWndClientWidth = LOWORD(lParam);
		gnWndClientHeight = HIWORD(lParam);

		OnResizeBackBuffers();

		break;
	}
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
	case WM_MOUSEMOVE:
		OnProcessingMouseMessage(hWnd, nMessageID, wParam, lParam);
		break;
	case WM_KEYDOWN:
	case WM_IME_CHAR:
	case WM_IME_COMPOSITION:
	case WM_IME_STARTCOMPOSITION:
	case WM_IME_SETCONTEXT:
	case WM_IME_ENDCOMPOSITION:
	case WM_CHAR:
		OnProcessingKeyboardMessage(hWnd, nMessageID, wParam, lParam);
		break;
	case WM_KEYUP:
		OnProcessingKeyboardMessage(hWnd, nMessageID, wParam, lParam);
		break;
	}
	return(0);
}

void CGameFramework::ProcessInput()
{
	if(m_pScene) m_pScene->ProcessInput(m_fElapsedTime);

	float cxDelta = 0.0f, cyDelta = 0.0f;
	POINT ptCursorPos;

	if (::GetCapture() == m_hWnd)
	{
		::SetCursor(NULL);

		::GetCursorPos(&ptCursorPos);

		float fSensitivity = MOUSE_SENSITIVITY;

		if (m_pPlayer)
		{
			if (m_pPlayer->IsZoomIn())
				fSensitivity *= 4.0f;
		}

		cxDelta = (float)(ptCursorPos.x - m_ptOldCursorPos.x) / fSensitivity;
		cyDelta = (float)(ptCursorPos.y - m_ptOldCursorPos.y) / fSensitivity;

		if (m_pPlayer)
		{
			if (m_pPlayer->IsDash())
			{
				cxDelta *= 0.6f;
				cyDelta *= 0.8f;
			}
		}

		::SetCursorPos(m_ptOldCursorPos.x, m_ptOldCursorPos.y);
	}

	if ((cxDelta != 0.0f) || (cyDelta != 0.0f))
	{
		if (cxDelta || cyDelta)
		{
			if (m_pScene) m_pScene->MoveCursor(cxDelta, cyDelta);
			if (m_pPlayer) m_pPlayer->Rotate(cyDelta, cxDelta, 0.0f);
		}
	}
}

void CGameFramework::AnimateObjects(float fElapsedTime)
{
	if (m_pScene) m_pScene->AnimateObjects(fElapsedTime, m_pCamera);
}

void CGameFramework::WaitForGpuComplete()
{
	//CPU 펜스의 값을 증가한다. 
	UINT64 nFenceValue = ++m_nFenceValues[m_nSwapChainBufferIndex];

	//GPU가 펜스의 값을 설정하는 명령을 명령 큐에 추가한다. 
	HRESULT hResult = m_pd3dCommandQueue->Signal(m_pd3dFence, nFenceValue);

	//펜스의 현재 값이 설정한 값보다 작으면 펜스의 현재 값이 설정한 값이 될 때까지 기다린다.
	if (m_pd3dFence->GetCompletedValue() < nFenceValue)
	{
		hResult = m_pd3dFence->SetEventOnCompletion(nFenceValue, m_hFenceEvent);
		::WaitForSingleObject(m_hFenceEvent, INFINITE);
	}
}

void CGameFramework::FrameAdvance()
{
	m_GameTimer.Tick(60.0f);
	m_fElapsedTime = m_GameTimer.GetTimeElapsed();

#ifdef ON_NETWORKING
	if (m_pPlayer && m_bLoadAll)
	{
		if (IsZero(m_fElapsedTime)) return;
		SendToServer();
	}
#endif
	
	ProcessInput();

	m_Font.CheckUsingTexts();

	AnimateObjects(m_fElapsedTime);

	HRESULT hResult = m_pd3dCommandAllocator->Reset();
	hResult = m_pd3dCommandList->Reset(m_pd3dCommandAllocator, NULL);
		
	if (m_pScene)
	{
		CScene::SetDescHeapsAndGraphicsRootSignature(m_pd3dCommandList);

		m_pScene->PrepareRender(m_pd3dCommandList);

		m_pScene->Render(m_pd3dCommandList, m_pCamera);

		if (m_bWireRender)
			m_pScene->RenderWire(m_pd3dCommandList, m_pCamera);

		if (m_pPlayer) m_pPlayer->Render(m_pd3dCommandList, m_pCamera);

		m_pScene->RenderEffects(m_pd3dCommandList, m_pCamera);

		m_pScene->PostProcessing(m_pd3dCommandList);

		//////
		::TransitionResourceState(m_pd3dCommandList, m_ppd3dRenderTargetBuffers[m_nSwapChainBufferIndex], D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

		D3D12_CPU_DESCRIPTOR_HANDLE d3dRtvHandle = m_pd3dRtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
		d3dRtvHandle.ptr += m_nSwapChainBufferIndex * ::gnRtvDescriptorIncrementSize;

		m_pd3dCommandList->ClearRenderTargetView(d3dRtvHandle, Colors::Black, 0, NULL);

		D3D12_CPU_DESCRIPTOR_HANDLE d3dDsvHandle = m_pd3dDsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
		m_pd3dCommandList->ClearDepthStencilView(d3dDsvHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, NULL);

		m_pd3dCommandList->OMSetRenderTargets(1, &d3dRtvHandle, TRUE, &d3dDsvHandle);

		m_pScene->RenderOffScreen(m_pd3dCommandList);

		m_pScene->RenderUI(m_pd3dCommandList);

		::TransitionResourceState(m_pd3dCommandList, m_ppd3dRenderTargetBuffers[m_nSwapChainBufferIndex], D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);

		m_pScene->AfterRender(m_pd3dCommandList, m_pCamera);
	}

	hResult = m_pd3dCommandList->Close();

	ID3D12CommandList *ppd3dCommandLists[] = { m_pd3dCommandList };
	m_pd3dCommandQueue->ExecuteCommandLists(_countof(ppd3dCommandLists), ppd3dCommandLists);

	WaitForGpuComplete();

	HRESULT ret = m_pdxgiSwapChain->Present(0, 0);
	if (ret)
	{
		HRESULT reason = m_pd3dDevice->GetDeviceRemovedReason();
		std::cout << reason << "\n";
	}

	MoveToNextFrame();

	float Screenx = ((2.0f  * m_ptCursorPos.x) / gnWndClientWidth) - 1;
	float Screeny = -((2.0f * m_ptCursorPos.y) / gnWndClientHeight) + 1;

#ifdef ON_NETWORKING
	m_nFramePerSecond++;
	m_fFPSTimeElapsed += m_fElapsedTime;
	if (m_fFPSTimeElapsed > 1.0f)
	{
		m_nFrameRate = m_nFramePerSecond;
		m_nFramePerSecond = 0;
		m_fFPSTimeElapsed = 0.0f;
	}

	m_fElapsedTime = 0.0f;

#ifdef _DEBUG
	_itow_s(m_nFrameRate, m_pszCaption + strlen(GAME_TITLE), 37, 10);
	wcscat_s(m_pszCaption + strlen(GAME_TITLE), 37, _T(" FPS"));
#endif

#else
	m_GameTimer.GetFrameRate(m_pszCaption + strlen(GAME_TITLE), 37);
	size_t nLength = _tcslen(m_pszCaption);
	_stprintf(m_pszCaption + nLength, _T("(%f, %f)"), Screenx, Screeny);
#endif
	
	::SetWindowText(m_hWnd, m_pszCaption);

	//if(m_pPlayer) std::cout << m_pPlayer->GetPosition().x << ", " << m_pPlayer->GetPosition().y << ", " << m_pPlayer->GetPosition().z << "\n";

}

void CGameFramework::OnResizeBackBuffers()
{
	WaitForGpuComplete();

	m_pd3dCommandList->Reset(m_pd3dCommandAllocator, NULL);

	for (int i = 0; i < m_nSwapChainBuffers; i++) if (m_ppd3dRenderTargetBuffers[i]) m_ppd3dRenderTargetBuffers[i]->Release();
	if (m_pd3dDepthStencilBuffer) m_pd3dDepthStencilBuffer->Release();
#ifdef _WITH_ONLY_RESIZE_BACKBUFFERS
	DXGI_SWAP_CHAIN_DESC dxgiSwapChainDesc;
	m_pdxgiSwapChain->GetDesc(&dxgiSwapChainDesc);
	m_pdxgiSwapChain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0);
	m_nSwapChainBufferIndex = 0;
#else
	//m_pdxgiSwapChain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH);
	DXGI_SWAP_CHAIN_DESC dxgiSwapChainDesc;
	m_pdxgiSwapChain->GetDesc(&dxgiSwapChainDesc);
	m_pdxgiSwapChain->ResizeBuffers(m_nSwapChainBuffers, gnWndClientWidth, gnWndClientHeight, dxgiSwapChainDesc.BufferDesc.Format, dxgiSwapChainDesc.Flags);
	m_nSwapChainBufferIndex = 0;
#endif
	CreateRenderTargetViews();
	CreateDepthStencilView();

	m_pd3dCommandList->Close();

	ID3D12CommandList *ppd3dCommandLists[] = { m_pd3dCommandList };
	m_pd3dCommandQueue->ExecuteCommandLists(1, ppd3dCommandLists);

	WaitForGpuComplete();
}

void CGameFramework::MoveToNextFrame()
{
	m_nSwapChainBufferIndex = m_pdxgiSwapChain->GetCurrentBackBufferIndex();

	UINT64 nFenceValue = ++m_nFenceValues[m_nSwapChainBufferIndex];
	HRESULT hResult = m_pd3dCommandQueue->Signal(m_pd3dFence, nFenceValue);
	if (m_pd3dFence->GetCompletedValue() < nFenceValue)
	{
		hResult = m_pd3dFence->SetEventOnCompletion(nFenceValue, m_hFenceEvent);
		::WaitForSingleObject(m_hFenceEvent, INFINITE);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CGameFramework::ProcessPacket()
{
	//printf("Size : %d\n", m_pPacketBuffer[0]);
	//printf("Type : %d\n", m_pPacketBuffer[1]);

	char nType = m_pPacketBuffer[1];

	switch (nType)
	{
	case PKT_ID_PLAYER_INFO:
	{
		PKT_PLAYER_INFO *pPacket = (PKT_PLAYER_INFO*)m_pPacketBuffer;
		if (pPacket->ID != gClientIndex)
			m_pScene->ApplyRecvInfo(PKT_ID_PLAYER_INFO, (LPVOID)pPacket);

		break;
	}
	case PKT_ID_TIME_INFO:
	{
		PKT_TIME_INFO *pPacket = (PKT_TIME_INFO*)m_pPacketBuffer;

		//m_fElapsedTime += pPacket->elapsedtime;
		break;
	}
	case PKT_ID_PLAYER_LIFE:
	{
		PKT_PLAYER_LIFE *pPacket = (PKT_PLAYER_LIFE*)m_pPacketBuffer;

		if (pPacket->ID == gClientIndex)
		{
			m_pPlayer->SetHitPoint(m_pPlayer->GetHitPoint() - pPacket->HP);
		}
		else
			m_pScene->ApplyRecvInfo(PKT_ID_PLAYER_LIFE, (LPVOID)pPacket);
		break;
	}
	case PKT_ID_CREATE_OBJECT:
	{
		PKT_CREATE_OBJECT *pPacket = (PKT_CREATE_OBJECT*)m_pPacketBuffer;

		CreateObject(pPacket);
		break;
	}
	case PKT_ID_UPDATE_OBJECT:
	{
		m_pScene->ApplyRecvInfo(PKT_ID_UPDATE_OBJECT, (LPVOID)m_pPacketBuffer);
		break;
	}
	case PKT_ID_DELETE_OBJECT:
	{
		m_pScene->ApplyRecvInfo(PKT_ID_DELETE_OBJECT, (LPVOID)m_pPacketBuffer);
		break;
	}
	case PKT_ID_CREATE_EFFECT:
	{
		PKT_CREATE_EFFECT *pPacket = (PKT_CREATE_EFFECT*)m_pPacketBuffer;

		CreateEffect(pPacket);
		break;
	}
	case PKT_ID_PLAYER_IN:
	{
		PKT_PLAYER_IN *pPacket = (PKT_PLAYER_IN*)m_pPacketBuffer;

		m_pScene->JoinPlayer(pPacket->id, pPacket->slot, pPacket->name, pPacket->robot);
		break;
	}
	case PKT_ID_PLAYER_OUT:
	{
		PKT_PLAYER_OUT *pPacket = (PKT_PLAYER_OUT*)m_pPacketBuffer;

		m_pScene->LeavePlayer(pPacket->id);
		break;
	}
	case PKT_ID_GAME_START:
	{
		PKT_GAME_START *pPacket = (PKT_GAME_START*)m_pPacketBuffer;

		int nPlayerTeam = CScene::GetMyTeam();

		m_pScene->GetTeamsInfo(nPlayerTeam, m_vnIndices, m_vnEnemyIndices, m_vpwstrNames);

		ReleaseObjects();

		m_bBGMStop = true;
		BuildScene(pPacket->map);

		CScene::SetMyTeam(nPlayerTeam);

		SendToServer(PKT_ID_LOAD_COMPLETE, NULL);
		break;
	}
	case PKT_ID_LOAD_COMPLETE_ALL:
	{
		m_pScene->ApplyRecvInfo(PKT_ID_LOAD_COMPLETE_ALL, (LPVOID)m_pPacketBuffer);
		std::cout << "전원 로드 완료\n";
		m_bLoadAll = true;
		break;
	}
	case PKT_ID_LOBBY_PLAYER_INFO:
	{
		PKT_LOBBY_PLAYER_INFO *pPacket = (PKT_LOBBY_PLAYER_INFO*)m_pPacketBuffer;

		m_pScene->ChangeSelectRobot(pPacket->id, pPacket->selected_robot);
		m_pScene->ChangeSlot(pPacket->id, pPacket->slot);
		if (pPacket->id == gClientIndex)
			m_nClientSlot = pPacket->slot;
		break;
	}
	case PKT_ID_SCORE:
	{
		m_pScene->ApplyRecvInfo(PKT_ID_SCORE, (LPVOID)m_pPacketBuffer);
		break;
	}
	case PKT_ID_GAME_END:
	{
		m_pScene->ApplyRecvInfo(PKT_ID_GAME_END, (LPVOID)m_pPacketBuffer);
		break;
	}
	case PKT_ID_PICK_ITEM:
	{
		m_pScene->ApplyRecvInfo(PKT_ID_PICK_ITEM, (LPVOID)m_pPacketBuffer);
		break;
	}
	case PKT_ID_CHANGE_MAP:
	{
		PKT_CHANGE_MAP *pPacket = (PKT_CHANGE_MAP*)m_pPacketBuffer;

		if (m_pScene) m_pScene->ChangeMap(pPacket->map);
		break;
	}
	case PKT_ID_ADD_ROOM:
	{
		PKT_ADD_ROOM *pPacket = (PKT_ADD_ROOM*)m_pPacketBuffer;

		if (m_pScene) m_pScene->AddRoom(pPacket->Room_num, pPacket->name);
		break;
	}
	case PKT_ID_DELETE_ROOM:
	{
		PKT_ROOM_DELETE *pPacket = (PKT_ROOM_DELETE*)m_pPacketBuffer;

		if (m_pScene) m_pScene->DeleteRoom(pPacket->Room_num);
		break;
	}
	case PKT_ID_CHANGE_ROOM_INFO:
	{
		PKT_CHANGE_ROOM_INFO *pPacket = (PKT_CHANGE_ROOM_INFO*)m_pPacketBuffer;

		if (m_pScene) m_pScene->ChangeRoomInfo(pPacket->Room_num, pPacket->map, pPacket->numpeople);
		break;
	}
	case PKT_ID_CREATE_ROOM_OK:
	{
		XMFLOAT2 xmf2Pos = m_pScene->GetCursorPos();
		ReleaseScene();

		m_bBGMStop = false;
		BuildScene(SCENE_TYPE_LOBBY_ROOM);
		m_pScene->SetCursorPosition(xmf2Pos);
		m_pScene->SetClientIndex(0, 0);
		gClientIndex = 0;
		m_nClientSlot = 0;
		break;
	}
	case PKT_ID_ROOM_IN_OK:
	{
		PKT_ROOM_IN_OK *pPacket = (PKT_ROOM_IN_OK*)m_pPacketBuffer;

		XMFLOAT2 xmf2Pos = m_pScene->GetCursorPos();
		ReleaseScene();

		m_bBGMStop = false;
		BuildScene(SCENE_TYPE_LOBBY_ROOM);
		m_pScene->SetCursorPosition(xmf2Pos);
		m_pScene->SetClientIndex(pPacket->index, pPacket->slot);
		m_pScene->SetMap(pPacket->map);
		gClientIndex = pPacket->index;
		m_nClientSlot = pPacket->slot;
		break;
	}
	case PKT_ID_MAP_EVENT:
	{
		m_pScene->ApplyRecvInfo(PKT_ID_MAP_EVENT, (LPVOID)m_pPacketBuffer);
		break;
	}
	case PKT_ID_CHANGE_NAME:
	{
		PKT_CHANGE_NAME *pPacket = (PKT_CHANGE_NAME*)m_pPacketBuffer;
		m_pScene->InitName(pPacket->name);
		break;
	}
	case PKT_ID_PLAYER_DIE:
	{
		m_pScene->ApplyRecvInfo(PKT_ID_PLAYER_DIE, (LPVOID)m_pPacketBuffer);
		break;
	}
	case PKT_ID_PLAYER_RESPAWN:
	{
		m_pScene->ApplyRecvInfo(PKT_ID_PLAYER_RESPAWN, (LPVOID)m_pPacketBuffer);
		break;
	}
	case PKT_ID_KILL_MESSAGE:
	{
		m_pScene->ApplyRecvInfo(PKT_ID_KILL_MESSAGE, (LPVOID)m_pPacketBuffer);
		break;
	}
	default:
	{
  		printf("Received Unknown Packet\n");
		break;
	}
	}
}

void CGameFramework::recvn()
{
	int nRest = recv(gSocket, m_RecvBuf, MAX_BUFFER, 0);

	char *ptr = m_RecvBuf;
	int nPacketsize = 0;

	if (m_nPrevSize > 0) nPacketsize = m_pPacketBuffer[0];

	while (nRest > 0)
	{
		if (nPacketsize == 0) nPacketsize = ptr[0];
		int nRequired = nPacketsize - m_nPrevSize;

		if (nRequired <= nRest)
		{
			memcpy(m_pPacketBuffer + m_nPrevSize, ptr, nRequired);

			ProcessPacket();

			nRest -= nRequired;
			ptr += nRequired;

			nPacketsize = 0;
			m_nPrevSize = 0;
		}
		else
		{
			memcpy(m_pPacketBuffer + m_nPrevSize, ptr, nRest);
			nRest = 0;
			m_nPrevSize += nRest;
		}
	}
}

void CGameFramework::OnProcessingSocketMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	switch (WSAGETSELECTEVENT(lParam))
	{
	case FD_READ:
	{
		recvn();
		break;
	}
	case FD_CLOSE:
	{
		printf("Do not Connect\n");
		PostQuitMessage(0);
	}
	}
}

void CGameFramework::CreateObject(PKT_CREATE_OBJECT *pCreateObjectInfo)
{
	m_pScene->InsertObject(m_pd3dDevice, m_pd3dCommandList, pCreateObjectInfo);
}

void CGameFramework::CreateEffect(PKT_CREATE_EFFECT *pCreateEffectInfo)
{
	m_pScene->CreateEffect(m_pd3dDevice, m_pd3dCommandList, pCreateEffectInfo);
}

void CGameFramework::SendToServer()
{
	int retval;
	PKT_PLAYER_INFO pktPlayerInfo;
	PKT_ID id = PKT_ID_PLAYER_INFO;
	pktPlayerInfo.PktId = (char)PKT_ID_PLAYER_INFO;
	pktPlayerInfo.PktSize = sizeof(PKT_PLAYER_INFO);

	pktPlayerInfo.ID = gClientIndex;

	pktPlayerInfo.WorldMatrix = m_pPlayer->GetWorldTransf();
	pktPlayerInfo.Player_Weapon = m_pPlayer->GetWeaponType();
	pktPlayerInfo.isChangeWeapon = m_pPlayer->GetWeaponChanged();

	if (pktPlayerInfo.isChangeWeapon) m_pPlayer->SetWeaponChanged(FALSE);

	pktPlayerInfo.Player_Up_Animation = ANIMATION_TYPE(m_pPlayer->GetAnimationState(ANIMATION_UP));
	pktPlayerInfo.isUpChangeAnimation = m_pPlayer->GetAnimationChanged(ANIMATION_UP);
	if (pktPlayerInfo.isUpChangeAnimation) m_pPlayer->SetAnimationChanged(ANIMATION_UP, FALSE);
	pktPlayerInfo.UpAnimationPosition = m_pPlayer->GetAnimationTrackPosition(ANIMATION_UP);

	pktPlayerInfo.Player_Down_Animation = ANIMATION_TYPE(m_pPlayer->GetAnimationState(ANIMATION_DOWN));
	pktPlayerInfo.isDownChangeAnimation = m_pPlayer->GetAnimationChanged(ANIMATION_DOWN);
	if (pktPlayerInfo.isDownChangeAnimation) m_pPlayer->SetAnimationChanged(ANIMATION_DOWN, FALSE);
	pktPlayerInfo.DownAnimationPosition = m_pPlayer->GetAnimationTrackPosition(ANIMATION_DOWN);

	pktPlayerInfo.State = m_pPlayer->GetState();

	//send(gSocket, (char*)&id, sizeof(PKT_ID), 0);
	if (retval = send(gSocket, (char*)&pktPlayerInfo, pktPlayerInfo.PktSize, 0) == SOCKET_ERROR)
		printf("Send Player Info Error\n");
}

void CGameFramework::SendToServer(PKT_ID pktID, void *pData)
{
	switch (pktID)
	{
	case PKT_ID_GAME_START:
	{
		PKT_GAME_START pktToServer;
		pktToServer.PktID = pktID;
		pktToServer.PktSize = sizeof(pktToServer);

		//send(gSocket, (char*)&id, sizeof(PKT_ID), 0);
		if (send(gSocket, (char*)&pktToServer, sizeof(pktToServer), 0) == SOCKET_ERROR)
			printf("Send Game Start Error\n");

		break;
	}
	case PKT_ID_LOAD_COMPLETE:
	{
		PKT_LOAD_COMPLETE pktToServer;
		pktToServer.PktID = pktID;
		pktToServer.PktSize = sizeof(pktToServer);

		//send(gSocket, (char*)&id, sizeof(PKT_ID), 0);
		if (send(gSocket, (char*)&pktToServer, sizeof(pktToServer), 0) == SOCKET_ERROR)
			printf("Send Load Complete Error\n");

		break;
	}
	case PKT_ID_ROOM_IN:
	{
		PKT_ROOM_IN pktToServer;
		pktToServer.PktId = pktID;
		pktToServer.PktSize = sizeof(pktToServer);
		pktToServer.Room_num = m_pScene->GetSelectRoom();

		//send(gSocket, (char*)&id, sizeof(PKT_ID), 0);
		if (send(gSocket, (char*)&pktToServer, sizeof(pktToServer), 0) == SOCKET_ERROR)
			printf("Send Load Complete Error\n");

		break;
	}
	case PKT_ID_CREATE_ROOM:
	{
		PKT_CREATE_ROOM packet;
		packet.PktId = pktID;
		packet.PktSize = sizeof(packet);

		wchar_t *pwstrName = (wchar_t*)pData;
		lstrcpynW(packet.name, pwstrName, lstrlenW(pwstrName) + 1);

		if (send(gSocket, (char*)&packet, sizeof(packet), 0) == SOCKET_ERROR)
			printf("Send Load Complete Error\n");

		break;
	}
	case PKT_ID_LOBBY_PLAYER_INFO:
	{
		PKT_LOBBY_PLAYER_INFO pktToServer;
		pktToServer.id = gClientIndex;
		pktToServer.PktId = pktID;
		pktToServer.PktSize = sizeof(pktToServer);
		pktToServer.selected_robot = m_pScene->GetPlayerRobotType();
		pktToServer.slot = m_nClientSlot;

		//send(gSocket, (char*)&id, sizeof(PKT_ID), 0);
		if (send(gSocket, (char*)&pktToServer, sizeof(pktToServer), 0) == SOCKET_ERROR)
			printf("Send LOBBY Player Info Error\n");
		break;
	}
	case PKT_ID_LEAVE_ROOM:
	{
		PKT_LEAVE_ROOM pktToServer;
		pktToServer.PktId = pktID;
		pktToServer.PktSize = sizeof(pktToServer);

		//send(gSocket, (char*)&id, sizeof(PKT_ID), 0);
		if (send(gSocket, (char*)&pktToServer, sizeof(pktToServer), 0) == SOCKET_ERROR)
			printf("Send Leave Room Error\n");
		break;
	}
	case PKT_ID_CHANGE_MAP:
	{
		PKT_CHANGE_MAP pktToServer;
		pktToServer.PktId = pktID;
		pktToServer.PktSize = sizeof(pktToServer);
		pktToServer.map = m_pScene->GetSelectedMap();

		//send(gSocket, (char*)&id, sizeof(PKT_ID), 0);
		if (send(gSocket, (char*)&pktToServer, sizeof(pktToServer), 0) == SOCKET_ERROR)
			printf("Send Change Map Error\n");
		break;
	}
	case PKT_ID_MOVE_TEAM:
	{
		int *p = (int*)pData;

		PKT_MOVE_TEAM pktToServer;
		pktToServer.PktId = pktID;
		pktToServer.PktSize = sizeof(pktToServer);
		pktToServer.team = *p;

		if (send(gSocket, (char*)&pktToServer, sizeof(pktToServer), 0) == SOCKET_ERROR)
			printf("Send Change Map Error\n");
		break;
	}
	case PKT_ID_CHANGE_NAME:
	{
		PKT_CHANGE_NAME pktToServer;
		pktToServer.PktId = pktID;
		pktToServer.PktSize = sizeof(pktToServer);
		lstrcpynW(pktToServer.name, CScene::GetMyName(), MAX_NAME_LENGTH);

		if (send(gSocket, (char*)&pktToServer, sizeof(pktToServer), 0) == SOCKET_ERROR)
			printf("Send Change Map Error\n");
		break;
	}
	case PKT_ID_MOVE_TO_MAIN_LOBBY:
	{
		PKT_MOVE_TO_MAIN_LOBBY pktToServer;
		pktToServer.PktId = pktID;
		pktToServer.PktSize = sizeof(pktToServer);

		if (send(gSocket, (char*)&pktToServer, sizeof(pktToServer), 0) == SOCKET_ERROR)
			printf("Send Change Map Error\n");
		break;
	}
	case PKT_ID_LOG_IN:
	{
		PKT_LOG_IN pktToServer;
		pktToServer.PktId = PKT_ID_LOG_IN;
		pktToServer.PktSize = sizeof(PKT_LOG_IN);
		lstrcpynW(pktToServer.id, L"asdf", MAX_NAME_LENGTH);
		lstrcpynW(pktToServer.pass, L"asdf", MAX_NAME_LENGTH);

		if (send(gSocket, (char*)&pktToServer, sizeof(pktToServer), 0) == SOCKET_ERROR)
			printf("Send Change Map Error\n");
		break;
	}
	default:
		break;
	}
}