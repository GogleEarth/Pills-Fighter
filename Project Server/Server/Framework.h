#pragma once
#include "pch.h"
#include "Mesh.h"
#include "Scene.h"
#include "Timer.h"
#include "Model.h"

struct Client_INFO
{
	int id;
	SOCKET socket;
	bool load_complete;
};

struct Client_arg
{
	SOCKET socket;
	int id;
};

class Framework
{
	WSADATA wsa;
	SOCKET listen_sock;

	HANDLE thread[MAX_CLIENT];
	HANDLE update_thread;
	HANDLE Event;
	HANDLE client_Event[MAX_CLIENT];
	std::mutex m;
	std::queue<PKT_PLAYER_INFO> msg_queue;
	std::mutex playerinfomutex;
	std::queue<PKT_PLAYER_LIFE> life_msg_queue;
	std::queue<PKT_DELETE_OBJECT> delete_msg_queue;
	std::queue<PKT_UPDATE_OBJECT> update_msg_queue;
	std::queue<PKT_CREATE_EFFECT> effect_msg_queue;
	std::queue<PKT_SHOOT> shoot_msg_queue;
	std::mutex shootmutex;
	std::vector<Client_INFO> clients;
	int count = 0;
	CModel PlayerMesh;
	CModel BulletMesh;
	CModel HangarMesh;
	CGameTimer m_GameTimer;
	int playernum;
	volatile bool game_start = false;
	float item_cooltime = 0.0f;
	float ammo_item_cooltime[2] = { 0.0f,0.0f };
	bool spawn_ammo[2] = { false,false };
	bool spawn_item = false;
	float elapsed_time;
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
	DWORD Update_Process(CScene* pScene);
	static DWORD WINAPI client_thread(LPVOID arg);
	DWORD client_process(Client_arg* arg);
	// 충돌 체크를 검사한다.
	void CheckCollision(CScene* pScene);
};

struct Arg
{
	Framework* pthis;
	SOCKET client_socket;
	int id;
};

struct Update_Arg
{
	Framework* pthis;
	CScene* pScene;
};
