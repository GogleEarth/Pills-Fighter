#pragma comment(lib, "ws2_32")

#include <WinSock2.h>
#include <iostream>
#include <stdlib.h>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <DirectXCollision.h>
#include <vector>
#include <queue>
#include <chrono>

#define SERVERIP	"127.0.0.1"
#define SERVERPORT	9000
#define FLAG		FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM
#define LANG		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT)
#define WS22		MAKEWORD(2,2)

void err_quit(char* msg);
void err_display(char* msg);
int recvn(SOCKET s, char* buf, int len, int flags);
DWORD WINAPI receve(LPVOID arg);

struct PLAYER_INFO
{
	int client_id;
	DirectX::XMFLOAT4X4 xmf4x4World;
};

int main()
{
	int retval;
	PLAYER_INFO pinfo;
	ZeroMemory(&pinfo.xmf4x4World, sizeof(DirectX::XMFLOAT4X4));
	std::cout << "아이디 입력:";
	std::cin >> pinfo.client_id;

	// 윈속 초기화
	WSADATA wsa;
	if (WSAStartup(WS22, &wsa) != 0)
		return 1;

	// socket()
	SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET)
		err_quit("socket()");

	// connect()
	SOCKADDR_IN serveraddr;
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = inet_addr(SERVERIP);
	serveraddr.sin_port = htons(SERVERPORT);
	retval = connect(sock, (SOCKADDR*)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR)
		err_quit("connect()");

	// 데이터 통신에 사용될 변수
	char buf[sizeof(PLAYER_INFO)];

	// 클라이언트 아이디 전송
	memcpy(&buf, &pinfo.client_id, sizeof(int));
	retval = send(sock, buf, sizeof(int), 0);
	if (retval == SOCKET_ERROR)
		err_display("send");
	std::cout << retval << "바이트 보냈음\n";

	pinfo.xmf4x4World._11 += 0.01f;
	memcpy(&buf, &pinfo, sizeof(PLAYER_INFO));
	retval = send(sock, buf, sizeof(PLAYER_INFO), 0);
	if (retval == SOCKET_ERROR)
		err_display("send");
	std::cout << retval << "바이트 보냈음\n";

	HANDLE hThread = CreateThread(
		NULL, 0, receve,
		(LPVOID)sock, 0, NULL);
	
	float elapsedtime = 0.0f;
	unsigned int i = 0;
	// 서버와 데이터 통신
	while (true)
	{
		//auto start = std::chrono::high_resolution_clock::now();
		//if (elapsedtime >= 1.0f)
		//{
		if (i >= 100000000)
		{
			pinfo.xmf4x4World._11 += 0.01f;
			memcpy(&buf, &pinfo, sizeof(PLAYER_INFO));
			retval = send(sock, buf, sizeof(PLAYER_INFO), 0);
			if (retval == SOCKET_ERROR)
			{
				err_display("send");
				break;
			}
			std::cout << retval << "바이트 보냈음\n";
			i = 0;
		}
		i++;
			//elapsedtime = 0;
		//}
		//std::cout << "메롱메롱" << std::endl;

		//auto end = std::chrono::high_resolution_clock::now();
		//auto du = end - start;
		//elapsedtime += std::chrono::duration_cast<std::chrono::milliseconds>(du).count()/1000.0f;
		//std::cout << elapsedtime << "지남\n";
	}

	closesocket(sock);
	CloseHandle(hThread);

	// 윈속 종료
	WSACleanup();

	return 0;
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
		msg, MB_ICONERROR);
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

DWORD WINAPI receve(LPVOID arg)
{
	int retval;
	char buf[sizeof(PLAYER_INFO)];
	SOCKET sock = (SOCKET)arg;

	while (true)
	{
		retval = recv(sock, buf, sizeof(PLAYER_INFO), 0);
		if (retval == SOCKET_ERROR)
		{
			err_display("recv()");
			break;
		}
		std::cout << retval << "바이트 보냈음\n";

		// 받은 데이터 출력
		PLAYER_INFO p_info;
		memcpy(&p_info, &buf, sizeof(PLAYER_INFO));
		std::cout << "아이디: " << p_info.client_id << std::endl;
	}

	return 0;
}
