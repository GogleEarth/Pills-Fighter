#include "stdafx.h"
#include "GameFramework.h"

////////////////////////////////////////////////////////////////

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
}

CGameFramework::~CGameFramework()
{
}

bool CGameFramework::OnCreate(HINSTANCE hInstance, HWND hMainWnd, SOCKET sock)
{
	m_hInstance = hInstance;
	m_hWnd = hMainWnd;
	m_Socket = sock;

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

	::CloseHandle(m_hFenceEvent);
	::closesocket(m_Socket);
	WSACleanup();

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
	case SCENE_TYPE_LOBBY:
		BuildLobbyScene();
		break;
	case SCENE_TYPE_COLONY:
		BuildColonyScene();
		break;
	}
	
	m_pScene->AddFont(m_pd3dDevice, &m_Arial);
	m_pScene->AddFont(m_pd3dDevice, &m_HumanMagic);

	m_pScene->SetAfterBuildObject(m_pd3dDevice, m_pd3dCommandList, NULL);
	m_pScene->StartScene();

	m_pd3dCommandList->Close();
	ID3D12CommandList *ppd3dCommandLists[] = { m_pd3dCommandList };
	m_pd3dCommandQueue->ExecuteCommandLists(1, ppd3dCommandLists);

	WaitForGpuComplete();

	if (m_pScene) m_pScene->ReleaseUploadBuffers();
	if (m_pPlayer) m_pPlayer->ReleaseUploadBuffers();
	if (m_pRepository) m_pRepository->ReleaseUploadBuffers();
}

void CGameFramework::BuildColonyScene()
{
	m_pScene = new CColonyScene();

	m_pScene->BuildObjects(m_pd3dDevice, m_pd3dCommandList, m_pRepository);

	CPlayer *pPlayer = new CPlayer(m_pd3dDevice, m_pd3dCommandList, m_pScene->GetGraphicsRootSignature(), m_pRepository, m_pScene->GetTerrain());
	pPlayer->SetMovingSpeed(100.0f);
	pPlayer->SetHitPoint(100);

	m_pPlayer = pPlayer;
	m_pScene->SetPlayer(pPlayer);

	m_pCamera = m_pPlayer->GetCamera();
}

void CGameFramework::BuildLobbyScene()
{
	m_pScene = new CLobbyScene();

	m_pScene->BuildObjects(m_pd3dDevice, m_pd3dCommandList, m_pRepository);
}

void CGameFramework::BuildObjects()
{
	m_pd3dCommandList->Reset(m_pd3dCommandAllocator, NULL);

	CScene::CreateGraphicsRootSignature(m_pd3dDevice);
	CScene::CreateDescriptorHeaps(m_pd3dDevice, SCENE_DESCIPTOR_HEAP_COUNT);
	CScene::CreateRtvAndDsvDescriptorHeaps(m_pd3dDevice);

	m_pRepository = new CRepository();

	m_Arial.Initialize(m_pd3dDevice, m_pd3dCommandList, "./Resource/Font/Arial.fnt");
	m_HumanMagic.Initialize(m_pd3dDevice, m_pd3dCommandList, "./Resource/Font/휴먼매직체.fnt");

	m_d3dViewport = { 0.0f, 0.0f, float(FRAME_BUFFER_WIDTH), float(FRAME_BUFFER_HEIGHT), 0.0f, 1.0f };
	m_d3dScissorRect = { 0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT };

	m_pd3dCommandList->Close();
	ID3D12CommandList *ppd3dCommandLists[] = { m_pd3dCommandList };
	m_pd3dCommandQueue->ExecuteCommandLists(1, ppd3dCommandLists);

	WaitForGpuComplete();

	m_Arial.ReleaseUploadBuffers();
	m_HumanMagic.ReleaseUploadBuffers();

	::pDevice = m_pd3dDevice;
	::pCommandList = m_pd3dCommandList;

	BuildScene(SCENE_TYPE_LOBBY);
}

void CGameFramework::ReleaseObjects()
{
	if (m_pPlayer) delete m_pPlayer;

	CScene::ReleaseDescHeapsAndGraphicsRootSign();

	if (m_pScene)
	{
		m_pScene->ReleaseObjects();
		delete m_pScene;
	}

	if (m_pRepository) delete m_pRepository;

	m_Arial.Destroy();
	m_HumanMagic.Destroy();
}

void CGameFramework::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	if (m_pScene) m_pScene->OnProcessingMouseMessage(hWnd, nMessageID, wParam, lParam);

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
	case WM_LBUTTONUP:
		if (m_pScene)
		{
			switch (m_pScene->MouseClick())
			{
			case MOUSE_CLICK_TYPE_START:
#ifdef ON_NETWORKING
				SendToServer(PKT_ID_GAME_START);
#else
				if (m_pScene)
				{
					m_pScene->ReleaseObjects();
					delete m_pScene;

					m_pScene = NULL;
				}

				BuildScene(SCENE_TYPE_COLONY);
#endif
				break;
			}
		}
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
	if (m_pScene) m_pScene->OnProcessingKeyboardMessage(hWnd, nMessageID, wParam, lParam);

	switch (nMessageID)
	{
	case WM_KEYUP:
		switch (wParam)
		{
		case VK_ESCAPE:
			::PostQuitMessage(0);
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
		}
		break;
		case VK_RETURN:
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
	static UCHAR pKeyBuffer[256];

	if (::GetKeyboardState(pKeyBuffer)) 
		if (m_pScene) m_pScene->ProcessInput(pKeyBuffer, m_fElapsedTime);

	float cxDelta = 0.0f, cyDelta = 0.0f;
	POINT ptCursorPos;

	if (::GetCapture() == m_hWnd)
	{
		::SetCursor(NULL);

		::GetCursorPos(&ptCursorPos);

		cxDelta = (float)(ptCursorPos.x - m_ptOldCursorPos.x) / MOUSE_SENSITIVITY;
		cyDelta = (float)(ptCursorPos.y - m_ptOldCursorPos.y) / MOUSE_SENSITIVITY;

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

	if (m_pPlayer) m_pPlayer->Update(m_fElapsedTime);
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
	ProcessInput();

#ifdef ON_NETWORKING
	if (m_pPlayer)
	{
		if (m_pPlayer->IsShotable() && m_bDrawScene && m_bSend_Complete)
		{
			//총알 생성 패킷 보내기
			PKT_SHOOT pktShoot;
			PKT_ID id = PKT_ID_SHOOT;
			pktShoot.PktId = (char)PKT_ID_SHOOT;
			pktShoot.PktSize = sizeof(PKT_SHOOT);
			pktShoot.Player_Weapon = m_pPlayer->GetWeaponType();
			pktShoot.BulletWorldMatrix = m_pPlayer->GetToTarget();
			send(m_Socket, (char*)&id, sizeof(PKT_ID), 0);
			if (send(m_Socket, (char*)&pktShoot, pktShoot.PktSize, 0) == SOCKET_ERROR)
			{
				printf("Send Player Info Error\n");
			}
			else
				printf("Send Player Info Complete\n");
		}
	}

	SendToServer();
#else
	m_GameTimer.Tick(60.0f);
	m_fElapsedTime = m_GameTimer.GetTimeElapsed();
#endif

	m_Arial.CheckUsingTexts();
	m_HumanMagic.CheckUsingTexts();

	AnimateObjects(m_fElapsedTime);

	HRESULT hResult = m_pd3dCommandAllocator->Reset();
	hResult = m_pd3dCommandList->Reset(m_pd3dCommandAllocator, NULL);
		
	int nFPS = 0;

#ifdef ON_NETWORKING
	nFPS = m_nFramePerSecond;
#else
	nFPS = m_GameTimer.GetFPS();
#endif

	if (m_bDrawScene)
	{
		CScene::SetDescHeapsAndGraphicsRootSignature(m_pd3dCommandList);

		if (m_pScene) m_pScene->PrepareRender(m_pd3dCommandList);

		::TransitionResourceState(m_pd3dCommandList, m_ppd3dRenderTargetBuffers[m_nSwapChainBufferIndex], D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

		D3D12_CPU_DESCRIPTOR_HANDLE d3dRtvHandle = m_pd3dRtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
		d3dRtvHandle.ptr += m_nSwapChainBufferIndex * ::gnRtvDescriptorIncrementSize;

		m_pd3dCommandList->ClearRenderTargetView(d3dRtvHandle, Colors::Black, 0, NULL);

		D3D12_CPU_DESCRIPTOR_HANDLE d3dDsvHandle = m_pd3dDsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
		m_pd3dCommandList->ClearDepthStencilView(d3dDsvHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, NULL);

		m_pd3dCommandList->OMSetRenderTargets(1, &d3dRtvHandle, TRUE, &d3dDsvHandle);

		m_pd3dCommandList->RSSetViewports(1, &m_d3dViewport);
		m_pd3dCommandList->RSSetScissorRects(1, &m_d3dScissorRect);

		if (m_pScene) m_pScene->Render(m_pd3dCommandList, m_pCamera);

		if (m_pPlayer) m_pPlayer->Render(m_pd3dCommandList, m_pCamera);

		if (m_pScene)
		{
			m_pScene->RenderWire(m_pd3dCommandList, m_pCamera);

			m_pScene->AfterRender(m_pd3dCommandList);
		}

		::TransitionResourceState(m_pd3dCommandList, m_ppd3dRenderTargetBuffers[m_nSwapChainBufferIndex], D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	}

	hResult = m_pd3dCommandList->Close();

	ID3D12CommandList *ppd3dCommandLists[] = { m_pd3dCommandList };
	m_pd3dCommandQueue->ExecuteCommandLists(_countof(ppd3dCommandLists), ppd3dCommandLists);

	WaitForGpuComplete();

	m_pdxgiSwapChain->Present(0, 0);

	MoveToNextFrame();

	float Screenx = ((2.0f  * m_ptCursorPos.x) / gnWndClientWidth) - 1;
	float Screeny = -((2.0f * m_ptCursorPos.y) / gnWndClientHeight) + 1;

#ifdef ON_NETWORKING
	m_nFramePerSecond++;
	m_fFPSTimeElapsed += m_fElapsedTime;
	if (m_fFPSTimeElapsed > 1.0f)
	{
		m_fFrameRate = m_nFramePerSecond;
		m_nFramePerSecond = 0;
		m_fFPSTimeElapsed = 0.0f;
	}

	m_fElapsedTime = 0.0f;
	_itow_s(m_fFrameRate, m_pszCaption + strlen(GAME_TITLE), 37, 10);
	wcscat_s(m_pszCaption + strlen(GAME_TITLE), 37, _T(" FPS)"));
#else
	m_GameTimer.GetFrameRate(m_pszCaption + strlen(GAME_TITLE), 37);
#endif
	size_t nLength = _tcslen(m_pszCaption);
	_stprintf(m_pszCaption + nLength, _T("(%f, %f)"), Screenx, Screeny);

	::SetWindowText(m_hWnd, m_pszCaption);

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
	char nType = m_pPacketBuffer[1];

	switch (nType)
	{
	case PKT_ID_PLAYER_INFO:
	{
		PKT_PLAYER_INFO *pPacket = (PKT_PLAYER_INFO*)m_pPacketBuffer;
		if (pPacket->ID != m_nClinetIndex)
			m_pScene->ApplyRecvInfo(PKT_ID_PLAYER_INFO, (LPVOID)pPacket);

		break;
	}
	case PKT_ID_TIME_INFO:
	{
		PKT_TIME_INFO *pPacket = (PKT_TIME_INFO*)m_pPacketBuffer;

		m_fElapsedTime += pPacket->elapsedtime;
		break;
	}
	case PKT_ID_PLAYER_LIFE:
	{
		PKT_PLAYER_LIFE *pPacket = (PKT_PLAYER_LIFE*)m_pPacketBuffer;

		if (pPacket->ID == m_nClinetIndex)
		{
			m_pPlayer->SetHitPoint(m_pPlayer->GetHitPoint() - pPacket->HP);
			m_pPlayer->PickUpAmmo(WEAPON_TYPE_OF_GIM_GUN, pPacket->AMMO);
			m_pPlayer->PickUpAmmo(WEAPON_TYPE_OF_BAZOOKA, pPacket->AMMO);
			m_pPlayer->PickUpAmmo(WEAPON_TYPE_OF_MACHINEGUN, pPacket->AMMO);
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
		PKT_UPDATE_OBJECT *pPacket = (PKT_UPDATE_OBJECT*)m_pPacketBuffer;

		m_pScene->ApplyRecvInfo(PKT_ID_UPDATE_OBJECT, (LPVOID)pPacket);
		break;
	}
	case PKT_ID_DELETE_OBJECT:
	{
		PKT_DELETE_OBJECT *pPacket = (PKT_DELETE_OBJECT*)m_pPacketBuffer;

		m_pScene->ApplyRecvInfo(PKT_ID_DELETE_OBJECT, (LPVOID)pPacket);
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

		char pstrid[32];
		sprintf(pstrid, "%d", pPacket->id);

		m_pScene->JoinPlayer(pPacket->id, pstrid);
		break;
	}
	case PKT_ID_PLAYER_ID:
	{
		PKT_CLIENTID *pPacket = (PKT_CLIENTID*)m_pPacketBuffer;

		m_nClinetIndex = pPacket->id;
		m_pScene->SetPlayerIndex(m_nClinetIndex);

		break;
	}
	case PKT_ID_GAME_START:
	{
		if (m_pScene)
		{
			m_pScene->ReleaseObjects();
			delete m_pScene;

			m_pScene = NULL;
		}

		BuildScene(SCENE_TYPE_COLONY);

		m_bDrawScene = false;

		SendToServer(PKT_ID_LOAD_COMPLETE);
		break;
	}
	case PKT_ID_LOAD_COMPLETE_ALL:
	{
		m_bDrawScene = true;

		break;
	}
	case PKT_ID_SEND_COMPLETE:
	{
		m_bSend_Complete = true;
		break;
	}
	break;
	}
}

void CGameFramework::recvn()
{
	int nRest = recv(m_Socket, m_RecvBuf, MAX_BUFFER, 0);

	char *pBuf = m_RecvBuf;
	int nPacketsize = 0;

	if (m_nPacketSize > 0) nPacketsize = m_pPacketBuffer[0];

	while (nRest > 0)
	{
		if (nPacketsize == 0) nPacketsize = pBuf[0];

		int nRequired = nPacketsize - m_nPacketSize;

		if (nRequired <= nRest)
		{
			memcpy(m_pPacketBuffer + m_nPacketSize, pBuf, nRequired);

			ProcessPacket();

			nRest -= nRequired;
			pBuf += nRequired;

			nPacketsize = 0;
			m_nPacketSize = 0;
		}
		else
		{
			memcpy(m_pPacketBuffer + m_nPacketSize, pBuf, nRest);
			nRest = 0;
			m_nPacketSize += nRest;
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
	if (m_pPlayer && m_bDrawScene && m_bSend_Complete)
	{
		int retval;
		PKT_PLAYER_INFO pktPlayerInfo;
		PKT_ID id = PKT_ID_PLAYER_INFO;
		pktPlayerInfo.PktId = (char)PKT_ID_PLAYER_INFO;
		pktPlayerInfo.PktSize = sizeof(PKT_PLAYER_INFO);

		pktPlayerInfo.ID = m_nClinetIndex;

		pktPlayerInfo.WorldMatrix = m_pPlayer->GetWorldTransf();

		if (m_pPlayer->IsShotable())
		{
			//pktPlayerInfo.BulletWorldMatrix = m_pPlayer->GetToTarget();
			pktPlayerInfo.IsShooting = TRUE;
			m_pPlayer->IsShotable(false);
		}
		else
		{
			pktPlayerInfo.IsShooting = 0;
		}

		pktPlayerInfo.Player_Weapon = m_pPlayer->GetWeaponType();

		pktPlayerInfo.isChangeWeapon = m_pPlayer->GetWeaponChanged();

		if (pktPlayerInfo.isChangeWeapon) m_pPlayer->SetWeaponChanged(FALSE);

		pktPlayerInfo.Player_Animation = ANIMATION_TYPE(m_pPlayer->GetAnimationState());

		pktPlayerInfo.isChangeAnimation = m_pPlayer->GetAnimationChanged();

		if (pktPlayerInfo.isChangeAnimation) m_pPlayer->SetAnimationChanged(FALSE);

		pktPlayerInfo.State = m_pPlayer->GetState();

		send(m_Socket, (char*)&id, sizeof(PKT_ID), 0);
		if (retval = send(m_Socket, (char*)&pktPlayerInfo, pktPlayerInfo.PktSize, 0) == SOCKET_ERROR)
			printf("Send Player Info Error\n");

		m_bSend_Complete = false;
	}
}

void CGameFramework::SendToServer(PKT_ID pktID)
{
	switch (pktID)
	{
	case PKT_ID_GAME_START:
	{
		PKT_GAME_START pktToServer;
		PKT_ID id = PKT_ID_GAME_START;
		pktToServer.PktID = (char)PKT_ID_GAME_START;
		pktToServer.PktSize = sizeof(pktToServer);

		send(m_Socket, (char*)&id, sizeof(PKT_ID), 0);
		if (send(m_Socket, (char*)&pktToServer, sizeof(pktToServer), 0) == SOCKET_ERROR)
			printf("Send Game Start Error\n");

		break;
	}
	case PKT_ID_LOAD_COMPLETE:
	{
		PKT_LOAD_COMPLETE pktToServer;
		PKT_ID id = PKT_ID_LOAD_COMPLETE;
		pktToServer.PktID = (char)PKT_ID_LOAD_COMPLETE;
		pktToServer.PktSize = sizeof(pktToServer);

		send(m_Socket, (char*)&id, sizeof(PKT_ID), 0);
		if (send(m_Socket, (char*)&pktToServer, sizeof(pktToServer), 0) == SOCKET_ERROR)
			printf("Send Load Complete Error\n");

		break;
	}
	default:
		break;
	}
	
}