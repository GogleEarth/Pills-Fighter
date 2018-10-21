// PillsFighter.cpp: 응용 프로그램의 진입점을 정의합니다.
//

#include "stdafx.h"
#include "PillsFighter.h"
#include "GameFramework.h"

#define MAX_LOADSTRING 100

// 전역 변수:
HINSTANCE hInst;                                // 현재 인스턴스입니다.
WCHAR szTitle[MAX_LOADSTRING];                  // 제목 표시줄 텍스트입니다.
WCHAR szWindowClass[MAX_LOADSTRING];            // 기본 창 클래스 이름입니다.

// 이 코드 모듈에 들어 있는 함수의 정방향 선언입니다.
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
DWORD WINAPI recvThread(LPVOID sock);
void err_quit(char* msg);
void err_display(char* msg);
int recvn(SOCKET s, char * buf, int len, int flags);

CGameFramework gGameFramework;
std::mutex mymutex;
std::queue<PLAYER_INFO> msg_queue;
HANDLE hThread;
SOCKET mysock;

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR    lpCmdLine, _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: 여기에 코드를 입력합니다.

    // 전역 문자열을 초기화합니다.
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_MY03, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // 응용 프로그램 초기화를 수행합니다.
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_MY03));

    MSG msg;
	PLAYER_INFO p_info;
	ZeroMemory(&p_info, sizeof(PLAYER_INFO));
    // 기본 메시지 루프입니다.
	while (1)
	{
		if (::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT) break;
			if (!::TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
			{
				::TranslateMessage(&msg);
				::DispatchMessage(&msg);
			}
		}
		else
		{
			mymutex.lock();
			if (!msg_queue.empty())
			{
				p_info = msg_queue.front();
				msg_queue.pop();
			}
			mymutex.unlock();
			gGameFramework.FrameAdvance(p_info);
		}
	}
	gGameFramework.OnDestroy();

    return (int) msg.wParam;
}

//
//  함수: MyRegisterClass()
//
//  목적: 창 클래스를 등록합니다.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = ::LoadIcon(hInstance, MAKEINTRESOURCE(IDC_MYICON));
	wcex.hCursor = ::LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

	//주 윈도우의 메뉴가 나타나지 않도록 한다. 
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = ::LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));
	return ::RegisterClassEx(&wcex);
}

//
//   함수: InitInstance(HINSTANCE, int)
//
//   목적: 인스턴스 핸들을 저장하고 주 창을 만듭니다.
//
//   설명:
//
//        이 함수를 통해 인스턴스 핸들을 전역 변수에 저장하고
//        주 프로그램 창을 만든 다음 표시합니다.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // 인스턴스 핸들을 전역 변수에 저장합니다.

   RECT rc = { 0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT };
   DWORD dwStyle = WS_OVERLAPPED | WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU | WS_BORDER;
   AdjustWindowRect(&rc, dwStyle, FALSE);
   HWND hMainWnd = CreateWindow(szWindowClass, szTitle, dwStyle, CW_USEDEFAULT,
	   CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, NULL, NULL, hInstance, NULL);

   if (!hMainWnd) return(FALSE);

   int retval;
   PLAYER_INFO pinfo;
   std::cout << "아이디 입력:";
   std::cin >> pinfo.client_id;
   ZeroMemory(&pinfo.xmf4x4World, sizeof(XMFLOAT4X4));

   gGameFramework.OnCreate(hInstance, hMainWnd);

   // 데이터 통신에 사용될 변수
   char buf[sizeof(PLAYER_INFO)];

   // 윈속 초기화
   WSADATA wsa;
   if (WSAStartup(WS22, &wsa) != 0)
	   return 1;

   // socket()
   mysock = socket(AF_INET, SOCK_STREAM, 0);
   if (mysock == INVALID_SOCKET)
	   err_quit("socket()");

   int optval = 3000;

   retval = setsockopt(mysock, SOL_SOCKET, SO_RCVTIMEO, (char*)&optval, sizeof(optval));
   if (retval == SOCKET_ERROR)
	   err_quit("setsockopt()");

   // connect()
   SOCKADDR_IN serveraddr;
   ZeroMemory(&serveraddr, sizeof(serveraddr));
   serveraddr.sin_family = AF_INET;
   serveraddr.sin_addr.s_addr = inet_addr(SERVERIP);
   serveraddr.sin_port = htons(SERVERPORT);
   retval = connect(mysock, (SOCKADDR*)&serveraddr, sizeof(serveraddr));
   if (retval == SOCKET_ERROR)
	   err_quit("connect()");

   gGameFramework.sock = mysock;
   gGameFramework.m_pPlayer->Client_id = pinfo.client_id;

   // 클라이언트 아이디 전송
   memcpy(&buf, &pinfo.client_id, sizeof(int));
   retval = send(mysock, buf, sizeof(int), 0);
   if (retval == SOCKET_ERROR)
	   err_display("send");
   //std::cout << retval << "바이트 보냈음\n";

   retval = send(mysock, buf, sizeof(PLAYER_INFO), 0);
   if (retval == SOCKET_ERROR)
	   err_display("send");

   hThread = CreateThread(
	   NULL, 0, recvThread,
	   (LPVOID)mysock, 0, NULL);

   //std::thread t(&recvThread, sock);
   //t.join();

   ::ShowWindow(hMainWnd, nCmdShow);
   ::UpdateWindow(hMainWnd);

   return(TRUE);
}

//
//  함수: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  목적:  주 창의 메시지를 처리합니다.
//
//  WM_COMMAND  - 응용 프로그램 메뉴를 처리합니다.
//  WM_PAINT    - 주 창을 그립니다.
//  WM_DESTROY  - 종료 메시지를 게시하고 반환합니다.
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_SIZE:
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
	case WM_MOUSEMOVE:
	case WM_KEYDOWN:
	case WM_KEYUP:
		gGameFramework.OnProcessingWindowMessage(hWnd, message, wParam, lParam);
		break;
	case WM_DESTROY:
		::PostQuitMessage(0);
		break;
	default:
		return(::DefWindowProc(hWnd, message, wParam, lParam));
	}
	return 0;
}

// 정보 대화 상자의 메시지 처리기입니다.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

DWORD WINAPI recvThread(LPVOID sock)
{
	int retval;
	char buf[sizeof(PLAYER_INFO)];
	//SOCKET socket = (SOCKET)sock;

	while (true)
	{
		retval = recvn(mysock, buf, sizeof(PLAYER_INFO), 0);
		if (retval == SOCKET_ERROR)
		{
			err_display("recv()");
		}
		std::cout << retval << " 바이트 받음\n";
		// 받은 데이터 출력
		if (retval > 0)
		{
			PLAYER_INFO p_info;
			memcpy(&p_info, &buf, sizeof(PLAYER_INFO));
			mymutex.lock();
			msg_queue.push(PLAYER_INFO{ p_info.client_id, p_info.xmf4x4World });
			mymutex.unlock();
			std::cout << "아이디: " << p_info.client_id << std::endl;
		}
	}
}

void err_quit(char* msg)
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FLAG, NULL,
		WSAGetLastError(), LANG,
		(LPTSTR)&lpMsgBuf, 0, NULL);
	MessageBox(
		NULL, (LPCTSTR)lpMsgBuf,
		(LPCWSTR)msg, MB_ICONERROR);
	LocalFree(lpMsgBuf);
	exit(1);
}

void err_display(char* msg)
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FLAG, NULL,
		WSAGetLastError(), LANG,
		(LPTSTR)&lpMsgBuf, 0, NULL);
	printf("[%s] %s", msg, (char*)lpMsgBuf);
	LocalFree(lpMsgBuf);
}

int recvn(SOCKET s, char * buf, int len, int flags)
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