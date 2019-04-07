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
#define MAX_ROOMS	20

void err_quit(char* msg);
void err_display(char* msg);
int recvn(SOCKET s, char* buf, int len, int flags);
DWORD WINAPI receve(LPVOID arg);

int rooms[MAX_ROOMS];
int my_num;
char roominfail;

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
	char Player_id[MAX_BUFFER];
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
	for (int i = 0; i < MAX_ROOMS; ++i)
	{
		rooms[i] = -1;
	}
	
	std::cout << "���̵� �Է�(����12�� �ѱ�6���̳�):";
	std::cin >> Id;
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

	int len = strlen(Id);
	char* buf = new char[(int)len];
	strncpy(buf, Id, len);
	std::cout << (int)len << "����Ʈ ��������\n";
	retval = send(sock, (char*)&len, sizeof(int), 0);
	std::cout << retval << "����Ʈ ����\n";
	retval = send(sock, buf, (int)len, 0);
	std::cout << retval << "����Ʈ ����\n";

	retval = recv(sock, (char*)&my_num, sizeof(int), 0);

	HANDLE hThread = CreateThread(
		NULL, 0, receve,
		(LPVOID)sock, 0, NULL);
	
	// ������ ������ ���
	while (true)
	{
		int n;
		int i;
		RoobyPacketType pkttype;
		std::cout << "Ŀ��Ʈ �Է� : ";
		std::cin >> n;
		if (roominfail == 1)
			continue;
		switch (n)
		{
		case 1:
			std::cout << "���� �����մϴ�.\n";
			pkttype = RoobyPacketTypeCreateRoom;
			send(sock, (char*)&pkttype, sizeof(RoobyPacketType), 0);
			while (true);
			break;
		case 2:
			std::cout << "�� ��ȣ �Է� : ";
			std::cin >> i;
			if (rooms[i] != -1)
			{
				pkttype = RoobyPacketTypeRoomIn;
				send(sock, (char*)&pkttype, sizeof(RoobyPacketType), 0);
				RoobyPacketRoomIn pkt;
				pkt.Player_num = my_num;
				pkt.Room_num = i;
				send(sock, (char*)&pkt, sizeof(RoobyPacketRoomIn), 0);
			}
			else
			{
				std::cout << "�������� ���� ��\n";
			}
		}
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
	SOCKET sock = (SOCKET)arg;

	while (true)
	{
		int pktlen;
		RoobyPacketType PktID;
		// ������ �ޱ� # ��Ŷ �ĺ� ID
		retval = recvn(sock, (char*)&PktID, sizeof(RoobyPacketType), 0);
		if (retval == SOCKET_ERROR)
		{
			std::cout << "[ERROR] ������ �ޱ� # ��Ŷ �ĺ� ID\n";
			break;
		}
		else
		{
			std::cout << "��Ŷ Ÿ�� : " << PktID << std::endl;
			switch (PktID)
			{
			case RoobyPacketTypeLogIn:
				RoobyPacketLogIn login;
				retval = recvn(sock, (char*)&pktlen, sizeof(int), 0);
				std::cout << pktlen << "����Ʈ �޾ƾ���\n";
				retval = recvn(sock, (char*)&login, pktlen, 0);
				login.Player_id[pktlen - sizeof(int)] = '\0';
				std::cout << login.Player_id << "����!\n";
				break;
			case RoobyPacketTypeLogOut:
				break;
			case RoobyPacketTypeCreateRoom:
				RoobyPacketCreateRoom createroom;
				retval = recvn(sock, (char*)&createroom, sizeof(RoobyPacketCreateRoom), 0);
				std::cout << "\n" << (int)createroom.room_num << "�� �� ������!\n";
				std::cout << (int)createroom.Player_num << "�� ����\n";
				rooms[createroom.room_num] = createroom.Player_num;
				break;
			case RoobyPacketTypeDestroyRoom:
				RoobyPacketDestroyRoom destroyroom;
				retval = recvn(sock, (char*)&destroyroom, sizeof(RoobyPacketDestroyRoom), 0);
				std::cout << "\n" << (int)destroyroom.room_num << "�� �� �ı���!\n";
				rooms[destroyroom.room_num] = -1;
				break;
			case RoobyPacketTypeInvitePlayer:
				break;
			case RoobyPacketTypeRoomIn:
				recvn(sock, &roominfail, sizeof(char), 0);
				if (roominfail == 1)
					std::cout << "�� ���� ����\n";
				else
					std::cout << "�� ���� ����\n";
				break;
			case RoobyPacketTypeRoomOut:
				break;
			case RoobyPacketTypeRoomInfo:
				RoobyPacketRoomInfo roominfo;
				retval = recvn(sock, (char*)&roominfo, sizeof(RoobyPacketRoomInfo), 0);
				rooms[roominfo.room_num] = roominfo.Player_num;
				std::cout << (int)roominfo.room_num << "���濡 " << (int)roominfo.Player_num << "�� ����\n";
				break;
			default:
				break;
			}
		}
	}

	return 0;
}
