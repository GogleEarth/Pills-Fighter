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
#define SERVERPORT	9001
#define FLAG		FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM
#define LANG		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT)
#define WS22		MAKEWORD(2,2)
#define MAX_BUFFER	13

void err_quit(char* msg);
void err_display(char* msg);
int recvn(SOCKET s, char* buf, int len, int flags);
DWORD WINAPI receve(LPVOID arg);

typedef struct RoomInfo
{
	char room_num;
	char Player_num;
}RoomInfo,
RoobyPacketCreateRoom,
RoobyPacketDestroyRoom,
RoobyPacketRoomInfo;

enum RoobyPacketType
{
	RoobyPacketTypeLogIn,
	RoobyPacketTypeLogOut,
	RoobyPacketTypeCreateRoom,
	RoobyPacketTypeDestroyRoom,
	RoobyPacketTypeInvitePlayer,
	RoobyPacketTypeRoomIn,
	RoobyPacketTypeRoomOut,
	RoobyPacketTypeRoomInfo
};

typedef struct RoobyPacketLogIn
{
	int Player_num;
	char* Player_id;
}RoobyPacketLogIn,
RoobyPacketLogOut;

struct RoobyPacketInvitePlayer
{
	int fromPlayer_num;
	int toPlayer_num;
};

typedef struct RoobyPacketRoomIn
{
	int Player_num;
	char Room_num;
}RoobyPacketRoomIn,
RoobyPacketRoomOut;

int main()
{
	int retval;
	char Id[MAX_BUFFER];
	std::cout << "���̵� �Է�(����12�� �ѱ�6���̳�):";
	std::cin.getline(Id, MAX_BUFFER);
	std::cin.clear();
	std::cout << Id << "\n";
	// ���� �ʱ�ȭ
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

	HANDLE hThread = CreateThread(
		NULL, 0, receve,
		(LPVOID)sock, 0, NULL);
	
	// ������ ������ ���
	while (true)
	{

	}

	closesocket(sock);
	// CloseHandle(hThread);

	// ���� ����
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
	char buf[MAX_BUFFER];
	SOCKET sock = (SOCKET)arg;

	while (true)
	{
		retval = recvn(sock, buf, MAX_BUFFER, 0);
		if (retval == SOCKET_ERROR)
		{
			err_display("recv()");
			break;
		}
		std::cout << retval << "����Ʈ ������\n";

		//// ���� ������ ���
		//PLAYER_INFO p_info;
		//memcpy(&p_info, &buf, sizeof(PLAYER_INFO));
		//std::cout << "���̵�: " << p_info.client_id << std::endl;
	}

	return 0;
}
