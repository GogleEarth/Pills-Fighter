#pragma comment(lib, "ws2_32")

#include <WinSock2.h>
#include <iostream>
#include <stdlib.h>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <DirectXCollision.h>
#include <vector>
#include <queue>
#include <mutex>
#include <chrono>

#define SERVERPORT	9000
#define FLAG		FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM
#define LANG		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT)
#define WS22		MAKEWORD(2,2)

void err_quit(char* msg);
void err_display(char* msg);
DWORD WINAPI client_thread(LPVOID arg);
DWORD WINAPI send_msg(LPVOID arg);
int recvn(SOCKET s, char* buf, int len, int flags);

struct PLAYER_INFO
{
	int client_id;
	DirectX::XMFLOAT4X4 xmf4x4World;
};

struct CLIENT_INFO
{
	int client_id;
	SOCKET client_sock;
};

std::mutex m;
std::queue<PLAYER_INFO> msg_queue;
std::vector<SOCKET> clients;
HANDLE thread[4];
int count = 0;

int main()
{
	int retval; 

	// 윈속 초기화
	WSADATA wsa;
	if (WSAStartup(WS22, &wsa) != 0)
		return 1;

	// socket()
	SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_sock == INVALID_SOCKET)
		err_quit("socket()");

	// bind()
	SOCKADDR_IN serveraddr;
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons(SERVERPORT);
	retval = bind(listen_sock, (SOCKADDR*)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR)
		err_quit("bind()");

	// listen()
	retval = listen(listen_sock, SOMAXCONN);
	if (retval == SOCKET_ERROR)
		err_quit("listen()");

	// 데이터 통신에 사용될 변수
	SOCKET client_sock;
	SOCKADDR_IN clientaddr;
	int addrlen;
	char buf[sizeof(int)];
	HANDLE sendthread = CreateThread(
		NULL, 0, send_msg,
		NULL, 0, NULL);
	while (true)
	{
		if (count < 4)
		{
			// accept()
			addrlen = sizeof(clientaddr);
			client_sock = accept(listen_sock, (SOCKADDR*)&clientaddr, &addrlen);
			if (client_sock == INVALID_SOCKET)
			{
				err_display("accept()");
				break;
			}
			else
			{
				retval = recvn(client_sock, buf, sizeof(int), 0);
				if (retval == SOCKET_ERROR)
				{
					err_display("recv()");
					break;
				}

				m.lock();
				//CLIENT_INFO* c_info = new CLIENT_INFO;
				//memcpy(&c_info->client_id, &buf, sizeof(int));
				//std::cout << "id : " << c_info->client_id << "\n";
				//c_info->client_sock = client_sock;
				clients.emplace_back(client_sock);
				m.unlock();

				thread[count] = CreateThread(
					NULL, 0, client_thread,
					(LPVOID)client_sock, 0, NULL);
				count++;
			}
		}
	}

	// CloseHandle()
	for (int i = 0; i < count; ++i)
		CloseHandle(thread[i]);
	CloseHandle(sendthread);

	// closesocket()
	closesocket(listen_sock);

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

DWORD WINAPI client_thread(LPVOID arg)
{
	SOCKET client_sock = (SOCKET)arg;
	int retval;
	SOCKADDR_IN clientaddr;
	int addrlen;
	char buf[sizeof(PLAYER_INFO)];

	// 클라이언트 정보 얻기
	addrlen = sizeof(clientaddr);
	getpeername(client_sock, (SOCKADDR*)&clientaddr, &addrlen);

	while (1)
	{
		retval = recvn(client_sock, buf, sizeof(PLAYER_INFO), 0);
		if (retval == SOCKET_ERROR)
		{
			err_display("recv()");
			break;
		}
		
		// 받은 데이터 출력
		PLAYER_INFO p_info;
		memcpy(&p_info, &buf, sizeof(PLAYER_INFO));
		m.lock();
		std::cout << "아이디: " << p_info.client_id << std::endl;
		msg_queue.push(PLAYER_INFO{p_info.client_id, p_info.xmf4x4World});
		m.unlock();

	}

	closesocket(client_sock);
	return 0;
}

DWORD WINAPI send_msg(LPVOID arg)
{
	int retval;
	char buf(sizeof(PLAYER_INFO));
	//float elapsedtime = 0.0f;
	unsigned int i = 0;

	while (true)
	{
		//auto start = std::chrono::high_resolution_clock::now();
		if (i >= 1000)
		{
			m.lock();
			if (msg_queue.size() != 0)
			{
				PLAYER_INFO p_info = msg_queue.front();
				int id = p_info.client_id;
				memcpy(&buf, &p_info, sizeof(PLAYER_INFO));
				for (SOCKET d : clients)
				{
					retval = send(d, &buf, sizeof(PLAYER_INFO), 0);
					if (retval == SOCKET_ERROR)
					{
						err_display("send");
						break;
					}
					std::cout << id << "의 정보 " << retval << "바이트 보냈음\n";
				}
				msg_queue.pop();
			}
			m.unlock();
			i = 0;
		}
		//auto end = std::chrono::high_resolution_clock::now();
		//auto du = end - start;
		//std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(du).count() / 1000.0f << "지남\n";
		//elapsedtime += std::chrono::duration_cast<std::chrono::milliseconds>(du).count() / 1000.0f;
		i++;
	}

	return 0;
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
