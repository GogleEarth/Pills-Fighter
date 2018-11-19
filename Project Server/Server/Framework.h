#pragma once
#include "pch.h"
#include "Mesh.h"
#include "Scene.h"

struct Client_INFO
{
	int id;
	SOCKET socket;
};

class Framework
{
	WSADATA wsa;
	SOCKET listen_sock;

	HANDLE thread[MAX_CLIENT];
	HANDLE update_thread;
	HANDLE Event;
	std::mutex m;
	std::queue<PKT_PLAYER_INFO> msg_queue;
	std::vector<Client_INFO> clients;
	int count = 0;


public:
	CScene* m_pScene;
	Framework();
	~Framework();
	int Build();
	void Release();
	void main_loop();
	//int CreateThread();
	void err_quit(char* msg);
	void err_display(char* msg);
	int recvn(SOCKET s, char* buf, int len, int flags);
	int Send_msg(char* buf, int len, int flags);
	static DWORD WINAPI Update(LPVOID arg);
	DWORD Update_Process(LPVOID arg);
	static DWORD WINAPI client_thread(LPVOID arg);
	DWORD client_process(LPVOID arg);
};

struct Arg
{
	Framework* pthis;
	SOCKET client_socket;
};

struct Update_Arg
{
	Framework* pthis;
	CScene* pScene;
};
