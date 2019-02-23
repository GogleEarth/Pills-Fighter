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
		// ������ �ޱ� # ��Ŷ �ĺ� ID
		retval = recvn(socket, (char*)&iPktID, sizeof(PKT_ID), 0);
		if (retval == SOCKET_ERROR)	std::cout << "[ERROR] ������ �ޱ� # ��Ŷ �ĺ� ID" << std::endl;

		// ������ �ޱ� # ��Ŷ ����ü - SIZE ����
		m_Mutex.lock();
		if (iPktID == PKT_ID_PLAYER_INFO) nPktSize = sizeof(PKT_PLAYER_INFO); // �÷��̾� ���� [ ���, ���� ]
		else if (iPktID == PKT_ID_PLAYER_LIFE) nPktSize = sizeof(PKT_PLAYER_LIFE); // �÷��̾� ���� [ ü�� ]
		else if (iPktID == PKT_ID_CREATE_OBJECT) nPktSize = sizeof(PKT_CREATE_OBJECT); // ������Ʈ ���� [ ���� ]
		else if (iPktID == PKT_ID_DELETE_OBJECT) nPktSize = sizeof(PKT_DELETE_OBJECT); // ������Ʈ ���� [ ���� ]
		else if (iPktID == PKT_ID_TIME_INFO) nPktSize = sizeof(PKT_TIME_INFO); // ���� �ð� ���� 
		else if (iPktID == PKT_ID_UPDATE_OBJECT) nPktSize = sizeof(PKT_UPDATE_OBJECT); // ����Ʈ  ���� [ ���� ]
		else if (iPktID == PKT_ID_SEND_COMPLETE)
		{
			SetEvent(hEvent);
			m_Mutex.unlock();
			continue;
		}
		else if(iPktID == PKT_ID_CREATE_EFFECT)  nPktSize = sizeof(PKT_CREATE_EFFECT); // ������Ʈ ������Ʈ ����
		else std::cout << "[ERROR] ��Ŷ ID �ĺ� �Ұ�" << std::endl;

		// ������ �ޱ� # ��Ŷ ����ü - ����
		buf = new char[nPktSize];
		retval = recvn(socket, buf, nPktSize, 0);
		if (retval == SOCKET_ERROR)	std::cout << "[ERROR] ������ �ޱ� # ��Ŷ ����ü - ����" << std::endl;

		if (iPktID == PKT_ID_PLAYER_INFO) m_vMsgPlayerInfo.emplace_back((PKT_PLAYER_INFO*)buf); // �÷��̾� ���� [ ���, ���� ]
		else if (iPktID == PKT_ID_PLAYER_LIFE) m_vMsgPlayerLife.emplace_back((PKT_PLAYER_LIFE*)buf); // �÷��̾� ���� [ ü�� ]
		else if (iPktID == PKT_ID_CREATE_OBJECT) m_vMsgCreateObject.emplace_back((PKT_CREATE_OBJECT*)buf); // ������Ʈ ���� [ ���� ]
		else if (iPktID == PKT_ID_DELETE_OBJECT) m_vMsgDeleteObject.emplace_back((PKT_DELETE_OBJECT*)buf); // ������Ʈ ���� [ ���� ]
		else if (iPktID == PKT_ID_TIME_INFO) m_vMsgTimeInfo.emplace_back((PKT_TIME_INFO*)buf); // ���� �ð� ���� 
		else if (iPktID == PKT_ID_UPDATE_OBJECT)
		{
			//std::cout << ((PKT_UPDATE_OBJECT*)buf)->Object_Index << " : " << ((PKT_UPDATE_OBJECT*)buf)->Object_Position.y << std::endl;
			m_vMsgUpdateInfo.emplace_back((PKT_UPDATE_OBJECT*)buf); // ������Ʈ ������Ʈ ����
		}
		else if (iPktID == PKT_ID_CREATE_EFFECT)  m_vMsgCreateEffect.emplace_back((PKT_CREATE_EFFECT*)buf); // ����Ʈ  ���� [ ���� ]
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

	char ServerIP[20];
	::memset(&ServerIP, 0, sizeof(char[20]));

	std::cout << "���� IP �Է� : ";
	std::cin >> ServerIP;

	// socket()
	m_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (m_sock == INVALID_SOCKET)
		err_quit("socket()");

	int retval;

	// connect()
	SOCKADDR_IN serveraddr;
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = inet_addr(ServerIP);
	serveraddr.sin_port = htons(SERVERPORT);
	retval = connect(m_sock, (SOCKADDR*)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR)
		err_quit("connect()");

	// Ŭ���̾�Ʈ ���̵� �ޱ�
	retval = recvn(m_sock, (char*)&m_Client_Info, sizeof(CLIENTID), 0);
	if (retval == SOCKET_ERROR) err_display("recvn");

	// �� ���� �ޱ�
	SCENEINFO SceneInfo;

	retval = recvn(m_sock, (char*)&SceneInfo, sizeof(SCENEINFO), 0);
	if (retval == SOCKET_ERROR)	err_display("recv()");

	// �������� ������Ʈ �ʱ�����(��ġ, .. ���) �ޱ� [ �ٸ� �÷��̾��� �ʱ� ��ġ ]
	// �ڽ��� �������� ��ġ
	PKT_PLAYER_INFO pktPlayerInfo;
	retval = recvn(m_sock, (char*)&pktPlayerInfo, sizeof(PKT_PLAYER_INFO), 0);

	BuildScene(&SceneInfo);
	m_pPlayer->SetWorldTransf(pktPlayerInfo.WorldMatrix);
	
	for (int i = 0; i < 7; ++i)
	{
		// �ٸ� Ŭ���̾�Ʈ �÷��̾� ����
		PKT_CREATE_OBJECT pktCreateObject;
		retval = recvn(m_sock, (char*)&pktCreateObject, sizeof(PKT_CREATE_OBJECT), 0);

		CreateObject(pktCreateObject);
	}

	if (m_Client_Info == 0)
	{
		while (true)
		{
			std::string gamestart;
			std::cout << "���ӽ����Ϸ��� 'start'�� �Է� : ";
			std::cin >> gamestart;
			if (gamestart == "start")
			{
				PKT_ID pid = PKT_ID_GAME_STATE;
				retval = send(m_sock, (char*)&pid, sizeof(PKT_ID), 0);
				if (retval == SOCKET_ERROR)
					std::cout << "���Ͽ���\n";
				break;
			}
		}
	}

	PKT_GAME_STATE pktGameState;
	retval = recvn(m_sock, (char*)&pktGameState, sizeof(PKT_GAME_STATE), 0);

	if (pktGameState == PKT_GAME_STATE_START)
	{
		FrameworkThread *sFT = new FrameworkThread;
		sFT->pGFW = this;
		sFT->sock = m_sock;
		m_hThread = CreateThread(NULL, 0, recvThread, (LPVOID)sFT, 0, NULL);
	}
	else
		exit(0);
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

	//���콺 ĸ�ĸ� �����Ѵ�. 
	::ReleaseCapture();
}

void CGameFramework::CreateSwapChain()
{
	RECT rcClient;
	::GetClientRect(m_hWnd, &rcClient);
	m_nWndClientWidth = rcClient.right - rcClient.left;
	m_nWndClientHeight = rcClient.bottom - rcClient.top;

	// ����ü���� �����Ѵ�.
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
	//��üȭ�� ��忡�� ����ȭ���� �ػ󵵸� �ٲ��� �ʰ� �ĸ������ ũ�⸦ ����ȭ�� ũ��� �����Ѵ�. 
	dxgiSwapChainDesc.Flags = 0;
#else
	//��üȭ�� ��忡�� ����ȭ���� �ػ󵵸� ����ü��(�ĸ����)�� ũ�⿡ �°� �����Ѵ�. 
	dxgiSwapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
#endif
	HRESULT hResult = m_pdxgiFactory->CreateSwapChain(m_pd3dCommandQueue,
		&dxgiSwapChainDesc, (IDXGISwapChain **)&m_pdxgiSwapChain);

	//��Alt+Enter�� Ű�� ������ ��Ȱ��ȭ�Ѵ�. 
	m_pdxgiFactory->MakeWindowAssociation(m_hWnd, DXGI_MWA_NO_ALT_ENTER);

	//����ü���� ���� �ĸ���� �ε����� �����Ѵ�.
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
	//����(Direct) ��� ť�� �����Ѵ�. 
	D3D12_COMMAND_QUEUE_DESC d3dCommandQueueDesc;
	::ZeroMemory(&d3dCommandQueueDesc, sizeof(D3D12_COMMAND_QUEUE_DESC));
	d3dCommandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	d3dCommandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	HRESULT hResult = m_pd3dDevice->CreateCommandQueue(&d3dCommandQueueDesc,
		_uuidof(ID3D12CommandQueue), (void **)&m_pd3dCommandQueue);

	//����(Direct) ��� �Ҵ��ڸ� �����Ѵ�. 
	hResult = m_pd3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
		__uuidof(ID3D12CommandAllocator), (void **)&m_pd3dCommandAllocator);

	//����(Direct) ��� ����Ʈ�� �����Ѵ�. 
	hResult = m_pd3dDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
		m_pd3dCommandAllocator, NULL, __uuidof(ID3D12GraphicsCommandList), (void
			**)&m_pd3dCommandList);

	//��� ����Ʈ�� �����Ǹ� ����(Open) �����̹Ƿ� ����(Closed) ���·� �����. 
	hResult = m_pd3dCommandList->Close();
}

void CGameFramework::CreateRtvAndDsvDescriptorHeaps()
{
	//���� Ÿ�� ������ ��(�������� ������ ����ü�� ������ ����)�� �����Ѵ�.
	D3D12_DESCRIPTOR_HEAP_DESC d3dDescriptorHeapDesc;
	::ZeroMemory(&d3dDescriptorHeapDesc, sizeof(D3D12_DESCRIPTOR_HEAP_DESC));
	d3dDescriptorHeapDesc.NumDescriptors = m_nSwapChainBuffers;
	d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	d3dDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	d3dDescriptorHeapDesc.NodeMask = 0;
	HRESULT hResult = m_pd3dDevice->CreateDescriptorHeap(&d3dDescriptorHeapDesc, __uuidof(ID3D12DescriptorHeap), (void **)&m_pd3dRtvDescriptorHeap);

	//���� Ÿ�� ������ ���� ������ ũ�⸦ �����Ѵ�. 
	m_nRtvDescriptorIncrementSize = m_pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	//����-���ٽ� ������ ��(�������� ������ 1)�� �����Ѵ�. 
	d3dDescriptorHeapDesc.NumDescriptors = 1;
	d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	hResult = m_pd3dDevice->CreateDescriptorHeap(&d3dDescriptorHeapDesc, __uuidof(ID3D12DescriptorHeap), (void **)&m_pd3dDsvDescriptorHeap);

	//����-���ٽ� ������ ���� ������ ũ�⸦ �����Ѵ�. 
	m_nDsvDescriptorIncrementSize = m_pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
}

void CGameFramework::CreateRenderTargetViews()
{
	//����ü���� �� �ĸ� ���ۿ� ���� ���� Ÿ�� �並 �����Ѵ�.
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

	m_pd3dDevice->CreateDepthStencilView(m_pd3dDepthStencilBuffer, &d3dDepthStencilViewDesc/*NULL*/, d3dDsvCPUDescriptorHandle);
}

void CGameFramework::BuildScene(SCENEINFO *pSI)
{
	if (pSI)
	{
		switch (*pSI)
		{
		case SCENE_NAME_COLONY:
			m_pScene = new CScene();
			m_pScene->BuildObjects(m_pd3dDevice, m_pd3dCommandList, m_pRepository);
			break;
		}
	}
	else
	{
		m_pScene = new CScene();
		m_pScene->BuildObjects(m_pd3dDevice, m_pd3dCommandList, m_pRepository);
	}

	CPlayer *pPlayer = new CPlayer(m_pd3dDevice, m_pd3dCommandList, m_pScene->GetGraphicsRootSignature(), m_pRepository, m_pScene->GetTerrain());
	pPlayer->SetMovingSpeed(100.0f);
	pPlayer->SetHitPoint(100);

	m_pPlayer = pPlayer;
	m_pScene->SetPlayer(pPlayer);
	m_pPlayer->SetGravity(m_pScene->m_fGravAcc);
	m_pPlayer->SetScene(m_pScene);
	m_pCamera = m_pPlayer->GetCamera();

	m_pPlayer->AddWeapon(m_pd3dDevice, m_pd3dCommandList, m_pScene->GetGimGun(), WEAPON_TYPE_OF_GIM_GUN, m_pScene->GetGimGunBullet());
	m_pPlayer->AddWeapon(m_pd3dDevice, m_pd3dCommandList, m_pScene->GetBazooka(), WEAPON_TYPE_OF_BAZOOKA, m_pScene->GetBazookaBullet());
	m_pPlayer->AddWeapon(m_pd3dDevice, m_pd3dCommandList, m_pScene->GetMachineGun(), WEAPON_TYPE_OF_MACHINEGUN, m_pScene->GetMachineGunBullet());
	m_pPlayer->PickUpAmmo(1000);
}

void CGameFramework::BuildObjects()
{
	m_pd3dCommandList->Reset(m_pd3dCommandAllocator, NULL);

	m_pRepository = new CRepository();
	//�� ��ü�� �����ϰ� ���� ���Ե� ���� ��ü���� �����Ѵ�. 
	
#ifdef ON_NETWORKING
	InitNetwork();
#else
	BuildScene();
#endif

	//�� ��ü�� �����ϱ� ���Ͽ� �ʿ��� �׷��� ��� ����Ʈ���� ��� ť�� �߰��Ѵ�. 
	m_pd3dCommandList->Close();
	ID3D12CommandList *ppd3dCommandLists[] = { m_pd3dCommandList };
	m_pd3dCommandQueue->ExecuteCommandLists(1, ppd3dCommandLists);

	//�׷��� ��� ����Ʈ���� ��� ����� ������ ��ٸ���. 
	WaitForGpuComplete();

	//�׷��� ���ҽ����� �����ϴ� ������ ������ ���ε� ���۵��� �Ҹ��Ų��. 
	if (m_pScene) m_pScene->ReleaseUploadBuffers();
	if (m_pPlayer) m_pPlayer->ReleaseUploadBuffers();

	if(m_pRepository) m_pRepository->ReleaseUploadBuffers();
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
		ptCursorPos.x = LOWORD(lParam);
		ptCursorPos.y = HIWORD(lParam);
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
		case VK_F9: //��F9�� Ű�� �������� ������ ���� ��üȭ�� ����� ��ȯ�� ó���Ѵ�
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
		case '1':
			m_pPlayer->EquipOnRightHand(m_pPlayer->GetWeapon(0));
			break;
		case '2':
			m_pPlayer->EquipOnRightHand(m_pPlayer->GetWeapon(1));
			break;
		case '3':
			m_pPlayer->EquipOnRightHand(m_pPlayer->GetWeapon(2));
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
		//break;
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
		switch (wParam)
		{
		case 'B':
			m_bRenderWire = !m_bRenderWire;
			break;
		}
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
		if (pKeyBuffer['V'] & 0xF0) dwDirection |= DIR_DOWN;
		if (pKeyBuffer['R'] & 0xF0) m_pPlayer->Reload(m_pPlayer->GetRHWeapon());
	}

	if (dwDirection) m_pPlayer->Move(dwDirection, m_pPlayer->GetMovingSpeed() * m_fElapsedTime);

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
	//CPU �潺�� ���� �����Ѵ�. 
	UINT64 nFenceValue = ++m_nFenceValues[m_nSwapChainBufferIndex];

	//GPU�� �潺�� ���� �����ϴ� ����� ��� ť�� �߰��Ѵ�. 
	HRESULT hResult = m_pd3dCommandQueue->Signal(m_pd3dFence, nFenceValue);

	//�潺�� ���� ���� ������ ������ ������ �潺�� ���� ���� ������ ���� �� ������ ��ٸ���.
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
		pktPlayerInfo.Player_Animation = ANIMATION_TYPE_IDLE;
		pktPlayerInfo.Current_Frame = 0.45646;

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
	}
	m_vMsgTimeInfo.clear();
	m_Mutex.unlock();

	m_Mutex.lock();

	for (const auto& PlayerInfo : m_vMsgPlayerInfo)
	{
		if (PlayerInfo->ID != m_Client_Info)
			m_pScene->ApplyRecvInfo(PKT_ID_PLAYER_INFO, (LPVOID)PlayerInfo);
	}
	m_vMsgPlayerInfo.clear();

	for (const auto& PlayerLife : m_vMsgPlayerLife)
	{
		if (PlayerLife->ID == m_Client_Info)
			m_pPlayer->SetHitPoint(m_pPlayer->GetHitPoint() - PlayerLife->HP);
		else
			m_pScene->ApplyRecvInfo(PKT_ID_PLAYER_LIFE, (LPVOID)PlayerLife);
	}
	m_vMsgPlayerLife.clear();

	for (const auto& CreateInfo : m_vMsgCreateObject)
	{
		CreateObject(*CreateInfo);
	}
	m_vMsgCreateObject.clear();

	for (const auto& UpdateInfo : m_vMsgUpdateInfo)
	{
		m_pScene->ApplyRecvInfo(PKT_ID_UPDATE_OBJECT, (LPVOID)UpdateInfo);
	}
	m_vMsgUpdateInfo.clear();

	for (const auto& DelteInfo : m_vMsgDeleteObject)
	{
		m_pScene->ApplyRecvInfo(PKT_ID_DELETE_OBJECT, (LPVOID)DelteInfo);
	}
	m_vMsgDeleteObject.clear();

	for (const auto& CreateInfo : m_vMsgCreateEffect)
	{
		CreateEffect(*CreateInfo);
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

	D3D12_RESOURCE_BARRIER d3dResourceBarrier;
	::ZeroMemory(&d3dResourceBarrier, sizeof(D3D12_RESOURCE_BARRIER));
	d3dResourceBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	d3dResourceBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	d3dResourceBarrier.Transition.pResource =
		m_ppd3dRenderTargetBuffers[m_nSwapChainBufferIndex];
	d3dResourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	d3dResourceBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	d3dResourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	m_pd3dCommandList->ResourceBarrier(1, &d3dResourceBarrier);

	D3D12_CPU_DESCRIPTOR_HANDLE d3dRtvCPUDescriptorHandle =
		m_pd3dRtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	d3dRtvCPUDescriptorHandle.ptr += (m_nSwapChainBufferIndex *
		m_nRtvDescriptorIncrementSize);

	float pfClearColor[4] = { 0.0f, 0.125f, 0.3f, 1.0f };
	m_pd3dCommandList->ClearRenderTargetView(d3dRtvCPUDescriptorHandle,
		pfClearColor/*Colors::Azure*/, 0, NULL);

	D3D12_CPU_DESCRIPTOR_HANDLE d3dDsvCPUDescriptorHandle =
		m_pd3dDsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();

	m_pd3dCommandList->ClearDepthStencilView(d3dDsvCPUDescriptorHandle,
		D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, NULL);

	m_pd3dCommandList->OMSetRenderTargets(1, &d3dRtvCPUDescriptorHandle, TRUE,
		&d3dDsvCPUDescriptorHandle);

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
	//////////////////////////////////////////////////////////////////

	d3dResourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	d3dResourceBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	d3dResourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	m_pd3dCommandList->ResourceBarrier(1, &d3dResourceBarrier);

	hResult = m_pd3dCommandList->Close();

	ID3D12CommandList *ppd3dCommandLists[] = { m_pd3dCommandList };
	m_pd3dCommandQueue->ExecuteCommandLists(_countof(ppd3dCommandLists), ppd3dCommandLists);

	WaitForGpuComplete();

	m_pdxgiSwapChain->Present(0, 0);

	MoveToNextFrame();

	::GetWindowRect(m_hWnd, &m_wndRect);

	float Screenx = ((2.0f  * ptCursorPos.x) / m_nWndClientWidth) - 1;
	float Screeny = -((2.0f * ptCursorPos.y) / m_nWndClientHeight) + 1;

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
