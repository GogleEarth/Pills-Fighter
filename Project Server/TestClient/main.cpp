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
#define BUFSIZE		76

void err_quit(char* msg);
void err_display(char* msg);
int recvn(SOCKET s, char* buf, int len, int flags);
DWORD WINAPI receve(LPVOID arg);


enum PKT_ID
{
	PKT_ID_PLAYER_INFO = 1, // �÷��̾� ����
	PKT_ID_PLAYER_LIFE = 2, // �÷��̾� hp
	PKT_ID_CREATE_OBJECT = 3, // ������Ʈ ����
	PKT_ID_DELETE_OBJECT = 4 // ������Ʈ ����
};

enum OBJECT_TYPE
{
	OBJECT_TYPE_BULLET, // �Ѿ�
	OBJECT_TYPE_OBSTACLE // ��ֹ�
};

// ���� ��Ŷ ����ü
struct PKT_PLAYER_INFO
{
	int Id;
	DirectX::XMFLOAT4X4 worldmatrix;
	int shooting;
};

struct PKT_PLAYER_LIFE
{
	int id;
	DWORD player_hp;
};

struct PKT_CREATE_OBJECT
{
	OBJECT_TYPE Object_Type;
	DirectX::XMFLOAT4X4 worldmatrix;
	BYTE Object_Index;
};

struct PKT_DELETE_OBJECT
{
	BYTE Object_Index;
};

int main()
{
	int retval;
	PKT_PLAYER_INFO pinfo;
	ZeroMemory(&pinfo.worldmatrix, sizeof(DirectX::XMFLOAT4X4));
	std::cout << "���̵� �Է�:";
	std::cin >> pinfo.Id;
	pinfo.shooting = 0;

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


	//HANDLE hThread = CreateThread(
	//	NULL, 0, receve,
	//	(LPVOID)sock, 0, NULL);
	
	float elapsedtime = 0.0f;
	unsigned int i = 0;
	// ������ ������ ���
	while (true)
	{
		//auto start = std::chrono::high_resolution_clock::now();
		//if (elapsedtime >= 1.0f)
		//{
		if (i >= 10000000)
		{
			PKT_ID pid = PKT_ID_PLAYER_INFO;
			char pid_buf[sizeof(PKT_ID)];
			memcpy(&pid_buf, &pid, sizeof(PKT_ID));
			retval = send(sock, pid_buf, sizeof(PKT_ID), 0);
			if (retval == SOCKET_ERROR)
				err_display("send");
			std::cout << retval << "����Ʈ ������\n";

			pinfo.worldmatrix._11 += 0.01f;

			char pinfo_buf[sizeof(PKT_PLAYER_INFO)];
			memcpy(&pinfo_buf, &pinfo, sizeof(PKT_PLAYER_INFO));
			retval = send(sock, pinfo_buf, sizeof(PKT_PLAYER_INFO), 0);
			if (retval == SOCKET_ERROR)
				err_display("send");
			std::cout << retval << "����Ʈ ������\n";

			i = 0;
		}
		i++;
			//elapsedtime = 0;
		//}
		//std::cout << "�޷ո޷�" << std::endl;

		//auto end = std::chrono::high_resolution_clock::now();
		//auto du = end - start;
		//elapsedtime += std::chrono::duration_cast<std::chrono::milliseconds>(du).count()/1000.0f;
		//std::cout << elapsedtime << "����\n";
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
	char buf[BUFSIZE];
	SOCKET sock = (SOCKET)arg;

	while (true)
	{
		retval = recv(sock, buf, BUFSIZE, 0);
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
