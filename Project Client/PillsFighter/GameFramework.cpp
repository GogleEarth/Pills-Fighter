#include "stdafx.h"
#include "GameFramework.h"

//////////////////////////////////////////////////////////////////////

DWORD WINAPI CGameFramework::recvThread(LPVOID arg)
{
	FrameworkThread* pFT = (FrameworkThread*)arg;

	DWORD retval = pFT->pGFW->ThreadFunc((LPVOID)pFT->sock);

	delete pFT;

	return retval;
}

void CGameFramework::err_quit(char* msg)
{
	LPVOID lpMsgBuf;
	FormatMessage(FLAG, NULL, WSAGetLastError(), LANG, (LPTSTR)&lpMsgBuf, 0, NULL);
	MessageBox(NULL, (LPCTSTR)lpMsgBuf, (LPCWSTR)msg, MB_ICONERROR);
	LocalFree(lpMsgBuf);

	exit(1);
}

void CGameFramework::err_display(char* msg)
{
	LPVOID lpMsgBuf;
	FormatMessage(FLAG, NULL, WSAGetLastError(), LANG, (LPTSTR)&lpMsgBuf, 0, NULL);
	printf("[%s] %s", msg, (char*)lpMsgBuf);
	LocalFree(lpMsgBuf);
}

int CGameFramework::recvn(SOCKET s, char * buf, int len, int flags)
{
	int received;
	char* ptr = buf;
	int left = len;

	while (left > 0)
	{
		received = recv(s, ptr, left, flags);
		if (received == SOCKET_ERROR)
			return SOCKET_ERROR;
		else if (received == 0)
			break;
		left -= received;
		ptr += received;
	}

	return (len - left);
}

DWORD CGameFramework::ThreadFunc(LPVOID arg)
{
	SOCKET socket = (SOCKET)arg;

	int retval;

	PKT_ID iPktID;
	int nPktSize = 0;

	char* buf;

	while (true)
	{
		// 데이터 받기 # 패킷 식별 ID
		retval = recvn(socket, (char*)&iPktID, sizeof(PKT_ID), 0);
		if (retval == SOCKET_ERROR)	std::cout << "[ERROR] 데이터 받기 # 패킷 식별 ID" << std::endl;

		// 데이터 받기 # 패킷 구조체 - SIZE 결정
		m_Mutex.lock();
		if (iPktID == PKT_ID_PLAYER_INFO) nPktSize = sizeof(PKT_PLAYER_INFO); // 플레이어 정보 [ 행렬, 상태 ]
		else if (iPktID == PKT_ID_PLAYER_LIFE) nPktSize = sizeof(PKT_PLAYER_LIFE); // 플레이어 정보 [ 체력 ]
		else if (iPktID == PKT_ID_CREATE_OBJECT) nPktSize = sizeof(PKT_CREATE_OBJECT); // 오브젝트 정보 [ 생성 ]
		else if (iPktID == PKT_ID_DELETE_OBJECT) nPktSize = sizeof(PKT_DELETE_OBJECT); // 오브젝트 정보 [ 삭제 ]
		else if (iPktID == PKT_ID_TIME_INFO) nPktSize = sizeof(PKT_TIME_INFO); // 서버 시간 정보 
		else if (iPktID == PKT_ID_UPDATE_OBJECT) nPktSize = sizeof(PKT_UPDATE_OBJECT); // 이펙트  정보 [ 생성 ]
		else if (iPktID == PKT_ID_SEND_COMPLETE)
		{
			SetEvent(hEvent);
			m_Mutex.unlock();
			continue;
		}
		else if(iPktID == PKT_ID_CREATE_EFFECT)  nPktSize = sizeof(PKT_CREATE_EFFECT); // 오브젝트 업데이트 정보
		else if (iPktID == PKT_ID_PLAYER_IN) nPktSize = sizeof(PKT_PLAYER_IN); // 룸 플레이어정보
		else if (iPktID == PKT_ID_GAME_STATE) nPktSize = sizeof(PKT_GAME_STATE);
		else std::cout << "[ERROR] 패킷 ID 식별 불가" << std::endl;

		// 데이터 받기 # 패킷 구조체 - 결정
		buf = new char[nPktSize];
		retval = recvn(socket, buf, nPktSize, 0);
		if (retval == SOCKET_ERROR)	std::cout << "[ERROR] 데이터 받기 # 패킷 구조체 - 결정" << std::endl;

		if (iPktID == PKT_ID_PLAYER_INFO) m_vMsgPlayerInfo.emplace_back((PKT_PLAYER_INFO*)buf); // 플레이어 정보 [ 행렬, 상태 ]
		else if (iPktID == PKT_ID_PLAYER_LIFE) m_vMsgPlayerLife.emplace_back((PKT_PLAYER_LIFE*)buf); // 플레이어 정보 [ 체력 ]
		else if (iPktID == PKT_ID_CREATE_OBJECT) m_vMsgCreateObject.emplace_back((PKT_CREATE_OBJECT*)buf); // 오브젝트 정보 [ 생성 ]
		else if (iPktID == PKT_ID_DELETE_OBJECT) m_vMsgDeleteObject.emplace_back((PKT_DELETE_OBJECT*)buf); // 오브젝트 정보 [ 삭제 ]
		else if (iPktID == PKT_ID_TIME_INFO) m_vMsgTimeInfo.emplace_back((PKT_TIME_INFO*)buf); // 서버 시간 정보 
		else if (iPktID == PKT_ID_UPDATE_OBJECT)
		{
			//std::cout << ((PKT_UPDATE_OBJECT*)buf)->Object_Index << " : " << ((PKT_UPDATE_OBJECT*)buf)->Object_Position.y << std::endl;
			m_vMsgUpdateInfo.emplace_back((PKT_UPDATE_OBJECT*)buf); // 오브젝트 업데이트 정보
		}
		else if (iPktID == PKT_ID_CREATE_EFFECT)  m_vMsgCreateEffect.emplace_back((PKT_CREATE_EFFECT*)buf); // 이펙트  정보 [ 생성 ]
		else if (iPktID == PKT_ID_PLAYER_IN)
		{
			std::cout << "\n" << ((PKT_PLAYER_IN*)buf)->id << "번 플레이어 접속\n";
		}
		else if (iPktID == PKT_ID_GAME_STATE)
		{
			gamestart = true;
			if(((PKT_GAME_STATE*)buf)->game_state == GAME_STATE_START)
			for (int i = 0; i < 8 - ((PKT_GAME_STATE*)buf)->num_player; i++)
			{
				PKT_DELETE_OBJECT pktDO;
				pktDO.Object_Index = ((PKT_GAME_STATE*)buf)->num_player + i;

				m_pScene->DeleteObject(pktDO);
			}
		}
		m_Mutex.unlock();
	}
}

void CGameFramework::InitNetwork()
{
	hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

	//// for Networking
	WSADATA wsa;
	if (WSAStartup(WS22, &wsa) != 0)
		return;

	int retval;

	while (true)
	{
		// socket()
		m_sock = socket(AF_INET, SOCK_STREAM, 0);
		if (m_sock == INVALID_SOCKET)
			err_display("socket()");

		// connect()
		SOCKADDR_IN serveraddr;
		ZeroMemory(&serveraddr, sizeof(serveraddr));
		serveraddr.sin_family = AF_INET;
		serveraddr.sin_addr.s_addr = inet_addr(SERVERIP);
		serveraddr.sin_port = htons(SERVERPORT);
		retval = connect(m_sock, (SOCKADDR*)&serveraddr, sizeof(serveraddr));
		if (retval == SOCKET_ERROR)
		{
			err_display("connect()");
			continue;
		}

		break;
	}

	// 클라이언트 아이디 받기
	retval = recvn(m_sock, (char*)&m_Client_Info, sizeof(CLIENTID), 0);
	if (retval == SOCKET_ERROR) err_display("recvn");

	// 씬 정보 받기
	SCENEINFO SceneInfo;

	retval = recvn(m_sock, (char*)&SceneInfo, sizeof(SCENEINFO), 0);
	if (retval == SOCKET_ERROR)	err_display("recv()");

	// 씬에서의 오브젝트 초기정보(위치, .. 등등) 받기 [ 다른 플레이어의 초기 위치 ]
	// 자신의 씬에서의 위치
	PKT_PLAYER_INFO pktPlayerInfo;
	retval = recvn(m_sock, (char*)&pktPlayerInfo, sizeof(PKT_PLAYER_INFO), 0);

	BuildScene(&SceneInfo);
	m_pPlayer->SetWorldTransf(pktPlayerInfo.WorldMatrix);
	
	for (int i = 0; i < 7; ++i)
	{
		// 다른 클라이언트 플레이어 정보
		PKT_CREATE_OBJECT pktCreateObject;
		retval = recvn(m_sock, (char*)&pktCreateObject, sizeof(PKT_CREATE_OBJECT), 0);

		CreateObject(pktCreateObject);
	}

	FrameworkThread *sFT = new FrameworkThread;
	sFT->pGFW = this;
	sFT->sock = m_sock;
	m_hThread = CreateThread(NULL, 0, recvThread, (LPVOID)sFT, 0, NULL);

	if (m_Client_Info == 0)
	{
		while (true)
		{
			std::string gamestart;
			std::cout << "게임시작하려면 'start'를 입력 : ";
			std::cin >> gamestart;
			if (gamestart == "start")
			{
				PKT_ID pid = PKT_ID_GAME_STATE;
				retval = send(m_sock, (char*)&pid, sizeof(PKT_ID), 0);
				if (retval == SOCKET_ERROR)
					std::cout << "소켓에러\n";
			}
		}
	}

	while (!gamestart);
}

void CGameFramework::CloseNetwork()
{
	closesocket(m_sock);
	CloseHandle(hEvent);

	WSACleanup();
}

void CGameFramework::CreateObject(PKT_CREATE_OBJECT CreateObjectInfo)
{
	m_pScene->InsertObject(m_pd3dDevice, m_pd3dCommandList, CreateObjectInfo);
}

void CGameFramework::CreateEffect(PKT_CREATE_EFFECT CreateEffectInfo)
{
	m_pScene->CreateEffect(m_pd3dDevice, m_pd3dCommandList, CreateEffectInfo);
}

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

	m_nRtvDescriptorIncrementSize = 0;
	m_nDsvDescriptorIncrementSize = 0;

	m_hFenceEvent = NULL;
	m_pd3dFence = NULL;
	for (int i = 0; i < m_nSwapChainBuffers; i++) m_nFenceValues[i] = 0;

	m_nWndClientWidth = FRAME_BUFFER_WIDTH;
	m_nWndClientHeight = FRAME_BUFFER_HEIGHT;

	m_pScene = NULL;
	m_pPlayer = NULL;

	_tcscpy_s(m_pszCaption, _T(GAME_TITLE));
}

CGameFramework::~CGameFramework()
{
}

bool CGameFramework::OnCreate(HINSTANCE hInstance, HWND hMainWnd)
{
	m_hInstance = hInstance;
	m_hWnd = hMainWnd;

	CreateDirect3DDevice();
	CreateCommandQueueAndList();
	CreateRtvAndDsvDescriptorHeaps();
	CreateSwapChain();
	CreateEnvironmentMap();
	CreateCubeMapCamera();
	CreateMinimapMap();
	CreateMiniMapCamera();

	BuildObjects();

	m_pMiniMapCamera->SetPlayer(m_pScene->GetPlayer());


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

	CloseNetwork();

	::CloseHandle(m_hFenceEvent);

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

	if (m_pd3dEnvirCube) m_pd3dEnvirCube->Release();
	if (m_pd3dEnvirCubeDSBuffer) m_pd3dEnvirCubeDSBuffer->Release();

	if (m_pd3dMinimapRsc) m_pd3dMinimapRsc->Release();

	for (int i = 0; i < 6; i++)
	{
		m_pCubeMapCamera[i]->ReleaseShaderVariables();
		delete m_pCubeMapCamera[i];
	}

	if (m_pMiniMapCamera) {
		m_pMiniMapCamera->ReleaseShaderVariables();
		delete m_pMiniMapCamera;
	}
	::ReleaseCapture();
}

void CGameFramework::CreateSwapChain()
{
	RECT rcClient;
	::GetClientRect(m_hWnd, &rcClient);
	m_nWndClientWidth = rcClient.right - rcClient.left;
	m_nWndClientHeight = rcClient.bottom - rcClient.top;

	DXGI_SWAP_CHAIN_DESC dxgiSwapChainDesc;
	::ZeroMemory(&dxgiSwapChainDesc, sizeof(dxgiSwapChainDesc));
	dxgiSwapChainDesc.BufferCount = m_nSwapChainBuffers;
	dxgiSwapChainDesc.BufferDesc.Width = m_nWndClientWidth;
	dxgiSwapChainDesc.BufferDesc.Height = m_nWndClientHeight;
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
	d3dDescriptorHeapDesc.NumDescriptors = m_nSwapChainBuffers + 6 + 1;
	d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	d3dDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	d3dDescriptorHeapDesc.NodeMask = 0;
	HRESULT hResult = m_pd3dDevice->CreateDescriptorHeap(&d3dDescriptorHeapDesc, __uuidof(ID3D12DescriptorHeap), (void **)&m_pd3dRtvDescriptorHeap);

	m_nRtvDescriptorIncrementSize = m_pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	d3dDescriptorHeapDesc.NumDescriptors = 2;
	d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	hResult = m_pd3dDevice->CreateDescriptorHeap(&d3dDescriptorHeapDesc, __uuidof(ID3D12DescriptorHeap), (void **)&m_pd3dDsvDescriptorHeap);

	m_nDsvDescriptorIncrementSize = m_pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
}

void CGameFramework::CreateEnvironmentMap()
{
	m_nCubeMapWidth = CUBE_MAP_WIDTH;
	m_nCubeMapHeight = CUBE_MAP_HEIGHT;

	D3D12_HEAP_PROPERTIES d3dHeapPropertiesDesc;
	::ZeroMemory(&d3dHeapPropertiesDesc, sizeof(D3D12_HEAP_PROPERTIES));
	d3dHeapPropertiesDesc.Type = D3D12_HEAP_TYPE_DEFAULT;
	d3dHeapPropertiesDesc.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	d3dHeapPropertiesDesc.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	d3dHeapPropertiesDesc.CreationNodeMask = 1;
	d3dHeapPropertiesDesc.VisibleNodeMask = 1;

	D3D12_RESOURCE_DESC d3dResourceDesc;
	ZeroMemory(&d3dResourceDesc, sizeof(D3D12_RESOURCE_DESC));

	d3dResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	d3dResourceDesc.Alignment = 0;
	d3dResourceDesc.Width = m_nCubeMapWidth;
	d3dResourceDesc.Height = m_nCubeMapHeight;
	d3dResourceDesc.DepthOrArraySize = 6;
	d3dResourceDesc.MipLevels = 1;
	d3dResourceDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	d3dResourceDesc.SampleDesc.Count = (m_bMsaa4xEnable) ? 4 : 1;
	d3dResourceDesc.SampleDesc.Quality = (m_bMsaa4xEnable) ? (m_nMsaa4xQualityLevels - 1) : 0;
	d3dResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	d3dResourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
	
	D3D12_CLEAR_VALUE d3dClear = { DXGI_FORMAT_R8G8B8A8_UNORM, { 0.0f, 0.0f, 0.0f, 1.0f } };

	m_pd3dDevice->CreateCommittedResource(&d3dHeapPropertiesDesc, D3D12_HEAP_FLAG_NONE, &d3dResourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, &d3dClear, __uuidof(ID3D12Resource), (void **)&m_pd3dEnvirCube);

	D3D12_CPU_DESCRIPTOR_HANDLE d3dRtvCPUDescriptorHandle = m_pd3dRtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	d3dRtvCPUDescriptorHandle.ptr += (m_nSwapChainBuffers * m_nRtvDescriptorIncrementSize);

	for (int i = 0; i < 6; i++)
	{
		D3D12_RENDER_TARGET_VIEW_DESC d3dDesc;
		d3dDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
		d3dDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		d3dDesc.Texture2DArray.MipSlice = 0;
		d3dDesc.Texture2DArray.PlaneSlice = 0;
		d3dDesc.Texture2DArray.FirstArraySlice = i;
		d3dDesc.Texture2DArray.ArraySize = 1;

		m_pd3dDevice->CreateRenderTargetView(m_pd3dEnvirCube, &d3dDesc, d3dRtvCPUDescriptorHandle);
		m_d3dRrvEnvirCubeMapCPUHandle[i] = d3dRtvCPUDescriptorHandle;

		d3dRtvCPUDescriptorHandle.ptr += m_nRtvDescriptorIncrementSize;
	}
}

void CGameFramework::CreateMinimapMap()
{
	screenCaptureTexture = new CTexture(1, RESOURCE_TEXTURE2D, 0);

	D3D12_CLEAR_VALUE d3dClear = { DXGI_FORMAT_R8G8B8A8_UNORM, { 0.0f, 0.0f, 0.0f, 1.0f } };

	m_pd3dMinimapRsc = screenCaptureTexture->CreateTexture(m_pd3dDevice, m_pd3dCommandList, m_nWndClientWidth, m_nWndClientHeight, DXGI_FORMAT_R8G8B8A8_UNORM, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ, &d3dClear, 0);

	D3D12_CPU_DESCRIPTOR_HANDLE d3dRtvCPUDescriptorHandle = m_pd3dRtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	d3dRtvCPUDescriptorHandle.ptr += (8 * m_nRtvDescriptorIncrementSize);

	//
	D3D12_RENDER_TARGET_VIEW_DESC d3dDesc;
	d3dDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	d3dDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	d3dDesc.Texture2D.MipSlice = 0;
	d3dDesc.Texture2D.PlaneSlice = 0;

	m_pd3dDevice->CreateRenderTargetView(m_pd3dMinimapRsc, &d3dDesc, d3dRtvCPUDescriptorHandle);
	m_d3dRtvMinimapCPUHandle = d3dRtvCPUDescriptorHandle;

	d3dRtvCPUDescriptorHandle.ptr += m_nRtvDescriptorIncrementSize;
}



void CGameFramework::CreateCubeMapCamera()
{
	XMFLOAT3 xmf3Looks[6] = {
		XMFLOAT3(1.0f, 0.0f, 0.0f),
		XMFLOAT3(-1.0f, 0.0f, 0.0f),
		XMFLOAT3(0.0f, 1.0f, 0.0f),
		XMFLOAT3(0.0f, -1.0f, 0.0f),
		XMFLOAT3(0.0f, 0.0f, 1.0f),
		XMFLOAT3(0.0f, 0.0f, -1.0f)
	};

	XMFLOAT3 xmf3Ups[6] = {
		XMFLOAT3(0.0f, 1.0f, 0.0f),
		XMFLOAT3(0.0f, 1.0f, 0.0f),
		XMFLOAT3(0.0f, 0.0f, -1.0f),
		XMFLOAT3(0.0f, 0.0f, 1.0f),
		XMFLOAT3(0.0f, 1.0f, 0.0f),
		XMFLOAT3(0.0f, 1.0f, 0.0f)
	};

	for (int i = 0; i < 6; i++)
	{
		XMFLOAT3 xmf3Right = Vector3::CrossProduct(xmf3Looks[i], xmf3Ups[i], true);

		m_pCubeMapCamera[i] = new CCamera();

		m_pCubeMapCamera[i]->SetOffset(XMFLOAT3(0.0f, 0.0f, 0.0f));
		m_pCubeMapCamera[i]->GenerateProjectionMatrix(1.01f, 5000.0f, ASPECT_RATIO, 60.0f);
		m_pCubeMapCamera[i]->SetViewport(0, 0, m_nCubeMapWidth, m_nCubeMapHeight, 0.0f, 1.0f);
		m_pCubeMapCamera[i]->SetScissorRect(0, 0, m_nCubeMapWidth, m_nCubeMapHeight);
		m_pCubeMapCamera[i]->SetRight(xmf3Right);
		m_pCubeMapCamera[i]->SetUp(xmf3Ups[i]);
		m_pCubeMapCamera[i]->SetLook(xmf3Looks[i]);

		m_pCubeMapCamera[i]->CreateShaderVariables(m_pd3dDevice, m_pd3dCommandList);
	}

}

void CGameFramework::CreateMiniMapCamera()
{
	m_pMiniMapCamera = new CCamera();

	XMFLOAT3 xmf3Looks = XMFLOAT3(0.0f, -1.0f, 0.0f);
	XMFLOAT3 xmf3Ups = XMFLOAT3(0.0f, 0.0f, 1.0f);
	XMFLOAT3 xmf3Right = Vector3::CrossProduct(xmf3Looks, xmf3Ups, true);

	m_pMiniMapCamera->SetOffset(XMFLOAT3(0.0f, 0.0f, 0.0f));
	m_pMiniMapCamera->GenerateProjectionMatrix(1.01f, 5000.0f, ASPECT_RATIO, 60.0f);
	m_pMiniMapCamera->SetViewport(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 0.0f, 1.0f);
	m_pMiniMapCamera->SetScissorRect(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT);
	m_pMiniMapCamera->SetRight(xmf3Right);
	m_pMiniMapCamera->SetUp(xmf3Ups);
	m_pMiniMapCamera->SetLook(xmf3Looks);

	m_pMiniMapCamera->CreateShaderVariables(m_pd3dDevice, m_pd3dCommandList);
}


void CGameFramework::CreateRenderTargetViews()
{
	//스왑체인의 각 후면 버퍼에 대한 렌더 타겟 뷰를 생성한다.
	D3D12_CPU_DESCRIPTOR_HANDLE d3dRtvCPUDescriptorHandle = m_pd3dRtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();

	for (UINT i = 0; i < m_nSwapChainBuffers; i++)
	{
		m_pdxgiSwapChain->GetBuffer(i, __uuidof(ID3D12Resource), (void**)&m_ppd3dRenderTargetBuffers[i]);
		m_pd3dDevice->CreateRenderTargetView(m_ppd3dRenderTargetBuffers[i], NULL, d3dRtvCPUDescriptorHandle);
		d3dRtvCPUDescriptorHandle.ptr += m_nRtvDescriptorIncrementSize;
	}
}

void CGameFramework::CreateDepthStencilView()
{
	D3D12_RESOURCE_DESC d3dResourceDesc;
	d3dResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	d3dResourceDesc.Alignment = 0;
	d3dResourceDesc.Width = m_nWndClientWidth;
	d3dResourceDesc.Height = m_nWndClientHeight;
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

	D3D12_CPU_DESCRIPTOR_HANDLE d3dDsvCPUDescriptorHandle = m_pd3dDsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	m_pd3dDevice->CreateCommittedResource(&d3dHeapProperties, D3D12_HEAP_FLAG_NONE, &d3dResourceDesc, D3D12_RESOURCE_STATE_DEPTH_WRITE, &d3dClearValue, __uuidof(ID3D12Resource), (void **)&m_pd3dDepthStencilBuffer);

	D3D12_DEPTH_STENCIL_VIEW_DESC d3dDepthStencilViewDesc;
	::ZeroMemory(&d3dDepthStencilViewDesc, sizeof(D3D12_DEPTH_STENCIL_VIEW_DESC));
	d3dDepthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	d3dDepthStencilViewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	d3dDepthStencilViewDesc.Flags = D3D12_DSV_FLAG_NONE;

	m_pd3dDevice->CreateDepthStencilView(m_pd3dDepthStencilBuffer, &d3dDepthStencilViewDesc, d3dDsvCPUDescriptorHandle);

	d3dDsvCPUDescriptorHandle.ptr += m_nDsvDescriptorIncrementSize;

	// For Environment Map
	d3dResourceDesc.Width = m_nCubeMapWidth;
	d3dResourceDesc.Height = m_nCubeMapHeight;

	m_pd3dDevice->CreateCommittedResource(&d3dHeapProperties, D3D12_HEAP_FLAG_NONE, &d3dResourceDesc, D3D12_RESOURCE_STATE_DEPTH_WRITE, &d3dClearValue, __uuidof(ID3D12Resource), (void **)&m_pd3dEnvirCubeDSBuffer);

	m_pd3dDevice->CreateDepthStencilView(m_pd3dEnvirCubeDSBuffer, &d3dDepthStencilViewDesc, d3dDsvCPUDescriptorHandle);
	m_d3dDsvEnvirCubeMapCPUHandle = d3dDsvCPUDescriptorHandle;

}

void CGameFramework::BuildScene(SCENEINFO *pSI)
{
	if (pSI)
	{
		switch (*pSI)
		{
		case SCENE_NAME_COLONY:
			m_pScene = new CColonyScene();
			break;
		default:
			m_pScene = new CScene();
			break;
		}
	}
	else
	{
		m_pScene = new CColonyScene();
	}

	m_pScene->BuildObjects(m_pd3dDevice, m_pd3dCommandList, m_pRepository);
	m_pScene->SetEnvirMapAndSRV(m_pd3dDevice, m_pd3dEnvirCube);

	// 렌더타겟의 내용을 텍스쳐로 만들기
	m_pScene->SetMinimapSRV(m_pd3dDevice, screenCaptureTexture);

	CPlayer *pPlayer = new CPlayer(m_pd3dDevice, m_pd3dCommandList, m_pScene->GetGraphicsRootSignature(), m_pRepository, screenCaptureTexture, m_pScene->GetTerrain());
	pPlayer->SetMovingSpeed(100.0f);
	pPlayer->SetHitPoint(100);

	m_pPlayer = pPlayer;
	m_pScene->SetPlayer(pPlayer);
	m_pCamera = m_pPlayer->GetCamera();

	m_pScene->SetAfterBuildObject(m_pd3dDevice, m_pd3dCommandList);
	m_pScene->StartScene();
}

void CGameFramework::BuildObjects()
{
	m_pd3dCommandList->Reset(m_pd3dCommandAllocator, NULL);

	m_pRepository = new CRepository();
	//씬 객체를 생성하고 씬에 포함될 게임 객체들을 생성한다. 
	
#ifdef ON_NETWORKING
	InitNetwork();
#else
	BuildScene();
#endif

	//씬 객체를 생성하기 위하여 필요한 그래픽 명령 리스트들을 명령 큐에 추가한다. 
	m_pd3dCommandList->Close();
	ID3D12CommandList *ppd3dCommandLists[] = { m_pd3dCommandList };
	m_pd3dCommandQueue->ExecuteCommandLists(1, ppd3dCommandLists);

	//그래픽 명령 리스트들이 모두 실행될 때까지 기다린다. 
	WaitForGpuComplete();

	//그래픽 리소스들을 생성하는 과정에 생성된 업로드 버퍼들을 소멸시킨다. 
	if (m_pScene) m_pScene->ReleaseUploadBuffers();
	if (m_pPlayer) m_pPlayer->ReleaseUploadBuffers();

	if(m_pRepository) m_pRepository->ReleaseUploadBuffers();

	::pDevice = m_pd3dDevice;
	::pCommandList = m_pd3dCommandList;
}

void CGameFramework::ReleaseObjects()
{
	if (m_pPlayer) delete m_pPlayer;

	if (m_pScene)
	{
		m_pScene->ReleaseObjects();
		delete m_pScene;
	}

	if (m_pRepository) delete m_pRepository;
}

void CGameFramework::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	switch (nMessageID)
	{
	case WM_LBUTTONDOWN:
		if (::GetCapture() != m_hWnd)
		{
			::SetCapture(m_hWnd);
			::SetCursorPos(m_ptOldCursorPos.x, m_ptOldCursorPos.y);
		}
		else
		{
			if (!m_LButtonDown) m_pPlayer->PrepareAttack(m_pPlayer->GetRHWeapon());

			m_LButtonDown = TRUE;
		}
		break;
	case WM_RBUTTONDOWN:
		break;
	case WM_LBUTTONUP:
	{
		m_LButtonDown = FALSE;
		break;
	}
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
			dxgiTargetParameters.Width = m_nWndClientWidth;
			dxgiTargetParameters.Height = m_nWndClientHeight;
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
		case VK_SPACE:
			m_pPlayer->DeactivationBooster();
			break;
		default:
			break;
		}
		break;
	case WM_KEYDOWN:
		switch (wParam)
		{
		case '1':
			m_pPlayer->ChangeWeapon(0);
			break;
		case '2':
			m_pPlayer->ChangeWeapon(1);
			break;
		case '3':
			m_pPlayer->ChangeWeapon(2);
			break;
		case 'B':
			m_bRenderWire = !m_bRenderWire;
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
		m_nWndClientWidth = LOWORD(lParam);
		m_nWndClientHeight = HIWORD(lParam);

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
	ULONG dwDirection = 0;

	if (::GetKeyboardState(pKeyBuffer))
	{
		if (pKeyBuffer['W'] & 0xF0) dwDirection |= DIR_FORWARD;
		if (pKeyBuffer['S'] & 0xF0) dwDirection |= DIR_BACKWARD;
		if (pKeyBuffer['A'] & 0xF0) dwDirection |= DIR_LEFT;
		if (pKeyBuffer['D'] & 0xF0) dwDirection |= DIR_RIGHT;
		if (pKeyBuffer[VK_SPACE] & 0xF0) dwDirection |= DIR_UP;
		if (pKeyBuffer['R'] & 0xF0) m_pPlayer->Reload(m_pPlayer->GetRHWeapon());
	}

	if (dwDirection) m_pPlayer->Move(dwDirection, m_pPlayer->GetMovingSpeed() * m_fElapsedTime);
	else m_pPlayer->ChangeAnimation(ANIMATION_STATE_IDLE);

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
			m_pPlayer->Rotate(cyDelta, cxDelta, 0.0f);
		}
	}

	if (m_LButtonDown)
	{
		m_pPlayer->Attack(m_pPlayer->GetRHWeapon());
	}

	m_pPlayer->Update(m_fElapsedTime);
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
	m_fElapsedTime = 0.0f;

#ifdef ON_NETWORKING
	auto start = std::chrono::high_resolution_clock::now();

	if (TRUE)
	{
		PKT_PLAYER_INFO pktPlayerInfo;
		int retval;

		pktPlayerInfo.ID = m_Client_Info;
		pktPlayerInfo.WorldMatrix = m_pPlayer->GetWorldTransf();
		if (m_pPlayer->IsShotable())
		{
			pktPlayerInfo.BulletWorldMatrix = m_pPlayer->GetToTarget();
			pktPlayerInfo.IsShooting = TRUE;
			m_pPlayer->IsShotable(false);
		}
		else pktPlayerInfo.IsShooting = 0;

		pktPlayerInfo.Player_Weapon = m_pPlayer->GetWeaponType();
		pktPlayerInfo.isChangeWeapon = m_pPlayer->GetWeaponChanged();
		if (pktPlayerInfo.isChangeWeapon) m_pPlayer->SetWeaponChanged(FALSE);
		pktPlayerInfo.Player_Animation = ANIMATION_TYPE(m_pPlayer->GetAnimationState());
		pktPlayerInfo.isChangeAnimation = m_pPlayer->GetAnimationChanged();
		if (pktPlayerInfo.isChangeAnimation) m_pPlayer->SetAnimationChanged(FALSE);
		pktPlayerInfo.State = m_pPlayer->GetState();


		PKT_ID pid = PKT_ID_PLAYER_INFO;
		retval = send(m_sock, (char*)&pid, sizeof(PKT_ID), 0);

		retval = send(m_sock, (char*)&pktPlayerInfo, sizeof(PKT_PLAYER_INFO), 0);
		if (retval == SOCKET_ERROR)	err_display("send");
	}

	WaitForSingleObject(hEvent, INFINITE);

	m_Mutex.lock();
	for (const auto& TimeInfo : m_vMsgTimeInfo)
	{
		m_fElapsedTime += TimeInfo->ElapsedTime;

		delete TimeInfo;
	}
	m_vMsgTimeInfo.clear();
	m_Mutex.unlock();

	m_Mutex.lock();

	for (auto& PlayerInfo : m_vMsgPlayerInfo)
	{
		if (PlayerInfo->ID != m_Client_Info)

			m_pScene->ApplyRecvInfo(PKT_ID_PLAYER_INFO, (LPVOID)PlayerInfo);

		delete PlayerInfo;
	}
	m_vMsgPlayerInfo.clear();

	for (auto& PlayerLife : m_vMsgPlayerLife)
	{
		if (PlayerLife->ID == m_Client_Info)
		{
			m_pPlayer->SetHitPoint(m_pPlayer->GetHitPoint() - PlayerLife->HP);
			m_pPlayer->PickUpAmmo(WEAPON_TYPE_OF_GIM_GUN, PlayerLife->AMMO);
			m_pPlayer->PickUpAmmo(WEAPON_TYPE_OF_BAZOOKA, PlayerLife->AMMO);
			m_pPlayer->PickUpAmmo(WEAPON_TYPE_OF_MACHINEGUN, PlayerLife->AMMO);
		}
		else
			m_pScene->ApplyRecvInfo(PKT_ID_PLAYER_LIFE, (LPVOID)PlayerLife);

		delete PlayerLife;
	}
	m_vMsgPlayerLife.clear();

	for (auto& CreateInfo : m_vMsgCreateObject)
	{
		CreateObject(*CreateInfo);

		delete CreateInfo;
	}
	m_vMsgCreateObject.clear();

	for (auto& UpdateInfo : m_vMsgUpdateInfo)
	{
		m_pScene->ApplyRecvInfo(PKT_ID_UPDATE_OBJECT, (LPVOID)UpdateInfo);

		delete UpdateInfo;
	}
	m_vMsgUpdateInfo.clear();

	for (auto& DelteInfo : m_vMsgDeleteObject)
	{
		m_pScene->ApplyRecvInfo(PKT_ID_DELETE_OBJECT, (LPVOID)DelteInfo);

		delete DelteInfo;
	}
	m_vMsgDeleteObject.clear();

	for (auto& CreateInfo : m_vMsgCreateEffect)
	{
		CreateEffect(*CreateInfo);

		delete CreateInfo;
	}
	m_vMsgCreateEffect.clear();

	m_Mutex.unlock();
	SendComplete = false;

	ResetEvent(hEvent);
#else
	m_GameTimer.Tick(60.0f);
	m_fElapsedTime = m_GameTimer.GetTimeElapsed();
#endif

	ProcessInput();

	AnimateObjects(m_fElapsedTime);

	HRESULT hResult = m_pd3dCommandAllocator->Reset();
	hResult = m_pd3dCommandList->Reset(m_pd3dCommandAllocator, NULL);

	if (m_pScene) m_pScene->PrepareRenderEffects(m_pd3dCommandList);
	
	int nFPS;
#ifdef ON_NETWORKING
	nFPS = m_nFramePerSecond;
#else
	nFPS = m_GameTimer.GetFPS();
#endif

	/*
	최적화 우선순위. [ 쉬운 것 부터 천천히 ]
	1. Set 횟수 줄이기
	   - 현재 같은 쉐이더를 사용함에도 불구하고 쉐이더 클래스마다 Set하고 있음.
	   - 쉐이더 변수 혹은 텍스처 등 한 번만 Set해도 되는 걸 찾기.

	2. 기하 쉐이더를 사용하여 6면을 한 번에 그리기.
	   - 이렇게 할 경우 쉐이더 코드를 전부 수정/추가해야 함.
	   - 시간과 집중력이 필요한 작업.
	*/

	//Draw Environment Map
	if (nFPS % 5 == 0)
	{
		::TransitionResourceState(m_pd3dCommandList, m_pd3dEnvirCube, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_RENDER_TARGET);

		for (int i = 0; i < 6; i++)
		{
			m_pCubeMapCamera[i]->SetPosition(m_pPlayer->GetPosition());
			m_pCubeMapCamera[i]->GenerateViewMatrix();

			m_pd3dCommandList->ClearRenderTargetView(m_d3dRrvEnvirCubeMapCPUHandle[i], Colors::Black, 0, NULL);
			m_pd3dCommandList->ClearDepthStencilView(m_d3dDsvEnvirCubeMapCPUHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, NULL);

			m_pd3dCommandList->OMSetRenderTargets(1, &m_d3dRrvEnvirCubeMapCPUHandle[i], TRUE, &m_d3dDsvEnvirCubeMapCPUHandle);

			if (m_pScene)
			{
				m_pScene->Render(m_pd3dCommandList, m_pCubeMapCamera[i]);
			}
		}
		::TransitionResourceState(m_pd3dCommandList, m_pd3dEnvirCube, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ);
	}

	//// Draw Minimap
	//{
	//	::TransitionResourceState(m_pd3dCommandList, m_pd3dMinimapRsc, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_RENDER_TARGET);

	//	if (m_pMiniMapCamera) {
	//		m_pMiniMapCamera->UpdateForMinimap(m_pMiniMapCamera->GetPlayer()->GetCamera()->GetLookVector());
	//		m_pMiniMapCamera->GenerateViewMatrix();
	//	}
	//	
	//	m_pd3dCommandList->ClearRenderTargetView(m_d3dRtvMinimapCPUHandle, Colors::Black, 0, NULL);
	//	m_pd3dCommandList->ClearDepthStencilView(m_pd3dDsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, NULL);
	//	m_pd3dCommandList->OMSetRenderTargets(1, &m_d3dRtvMinimapCPUHandle, TRUE, &m_pd3dDsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
	//	
	//	if (m_pScene)
	//	{
	//		m_pScene->MinimapRender(m_pd3dCommandList, m_pMiniMapCamera); // 그릴 때 평면 위에 점들로 그리도록 하기
	//	}

	//	::TransitionResourceState(m_pd3dCommandList, m_pd3dMinimapRsc, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ);
	//}

	::TransitionResourceState(m_pd3dCommandList, m_ppd3dRenderTargetBuffers[m_nSwapChainBufferIndex], D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

	D3D12_CPU_DESCRIPTOR_HANDLE d3dRtvCPUDescriptorHandle = m_pd3dRtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	d3dRtvCPUDescriptorHandle.ptr += (m_nSwapChainBufferIndex * m_nRtvDescriptorIncrementSize);

	m_pd3dCommandList->ClearRenderTargetView(d3dRtvCPUDescriptorHandle, Colors::Black, 0, NULL);

	D3D12_CPU_DESCRIPTOR_HANDLE d3dDsvCPUDescriptorHandle = m_pd3dDsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	m_pd3dCommandList->ClearDepthStencilView(d3dDsvCPUDescriptorHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, NULL);

	m_pd3dCommandList->OMSetRenderTargets(1, &d3dRtvCPUDescriptorHandle, TRUE, &d3dDsvCPUDescriptorHandle);

	if (m_pCamera) m_pCamera->GenerateViewMatrix();

	if (m_pScene)
	{
		m_pScene->Render(m_pd3dCommandList, m_pCamera);
	}

	if (m_pPlayer)
	{
		m_pPlayer->Render(m_pd3dCommandList, m_pCamera);
	}

	if (m_bRenderWire)
	{
		if (m_pScene) m_pScene->RenderWire(m_pd3dCommandList, m_pCamera);
		if (m_pPlayer) m_pPlayer->RenderWire(m_pd3dCommandList, m_pCamera);
	}

	if (m_pScene) m_pScene->AfterRenderEffects(m_pd3dCommandList);

	//////////////////////////////////////////////////////////////////


	::TransitionResourceState(m_pd3dCommandList, m_ppd3dRenderTargetBuffers[m_nSwapChainBufferIndex], D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);

	hResult = m_pd3dCommandList->Close();

	ID3D12CommandList *ppd3dCommandLists[] = { m_pd3dCommandList };
	m_pd3dCommandQueue->ExecuteCommandLists(_countof(ppd3dCommandLists), ppd3dCommandLists);

	WaitForGpuComplete();

	m_pdxgiSwapChain->Present(0, 0);

	MoveToNextFrame();

	::GetWindowRect(m_hWnd, &m_wndRect);

	float Screenx = ((2.0f  * m_ptCursorPos.x) / m_nWndClientWidth) - 1;
	float Screeny = -((2.0f * m_ptCursorPos.y) / m_nWndClientHeight) + 1;

#ifdef ON_NETWORKING
	m_nFramePerSecond++;
	m_fFPSTimeElapsed += m_fElapsedTime;
	if (m_fFPSTimeElapsed > 1.0f)
	{
		m_fFrameRate = m_nFramePerSecond;
		m_nFramePerSecond = 0;
		m_fFPSTimeElapsed = 0.0f;
	}

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
	m_pdxgiSwapChain->ResizeBuffers(m_nSwapChainBuffers, m_nWndClientWidth, m_nWndClientHeight, dxgiSwapChainDesc.BufferDesc.Format, dxgiSwapChainDesc.Flags);
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
