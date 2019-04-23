#include "pch.h"
#include "Framework.h"
#include "GameObject.h"

Framework::Framework()
{
	for (int i = 0; i < MAX_CLIENT; ++i)
		thread[i] = NULL;

	count = 0;
}

Framework::~Framework()
{
}

int Framework::Build()
{
	int retval;

	Event = CreateEvent(NULL, TRUE, FALSE, NULL);

	game_start = false;

	m_pScene = new CScene();
	m_pScene->BuildObjects();

	// 윈속 초기화
	if (WSAStartup(WS22, &wsa) != 0)
		return 1;

	// socket()
	listen_sock = socket(AF_INET, SOCK_STREAM, 0);
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
}

void Framework::Release()
{
	WaitForMultipleObjects(playernum, thread, true, INFINITE);

	// CloseHandle()
	for (int i = 0; i < MAX_CLIENT; ++i)
		if(thread[i])
			CloseHandle(thread[i]);

	// closesocket()
	for (auto client_sock : clients)
		closesocket(client_sock.socket);
	closesocket(listen_sock);

	// 윈속 종료
	WSACleanup();
}

void Framework::main_loop()
{
	int addrlen;
	char buf[sizeof(int)];
	int retval;

	Update_Arg* arg = new Update_Arg;
	arg->pthis = this;
	arg->pScene = m_pScene;

	update_thread = CreateThread(
		NULL, 0, Update,
		(LPVOID)arg, 0, NULL);

	while (true)
	{
		if (count < MAX_CLIENT && count != -1)
		{
			SOCKET client_sock;
			SOCKADDR_IN clientaddr;
			// accept()
			addrlen = sizeof(clientaddr);
			std::cout << count << std::endl;
			client_sock = accept(listen_sock, (SOCKADDR*)&clientaddr, &addrlen);
			if (client_sock == INVALID_SOCKET)
			{
				std::cout << "main_loop ERROR : ";
				err_display("accept()");
				break;
			}
			else
			{
				if (!game_start)
				{
					PKT_CLIENTID pkt_cid;
					pkt_cid.PktId = (char)PKT_ID_PLAYER_ID;
					pkt_cid.PktSize = (char)sizeof(PKT_CLIENTID);
					pkt_cid.id = count;
					if (send(client_sock, (char*)&pkt_cid, pkt_cid.PktSize, 0) == SOCKET_ERROR)
					{
						std::cout << "main_loop ERROR : ";
						err_display("send()");
						break;
					}

					//SCENEINFO sinfo = SCENE_NAME_COLONY;
					//retval = send(client_sock, (char*)&sinfo, sizeof(SCENEINFO), 0);

					//PKT_PLAYER_INFO pktdata;
					//PKT_CREATE_OBJECT anotherpktdata;
					//std::cout << count << std::endl;
					//pktdata.ID = count;
					//pktdata.WorldMatrix = m_pScene->m_pObjects[count]->m_xmf4x4World;
					//m_pScene->m_pObjects[count]->m_bPlay = true;
					//std::cout << pktdata.ID << " : " << pktdata.WorldMatrix._41 << ", " << pktdata.WorldMatrix._42 << ", " << pktdata.WorldMatrix._43 << std::endl;
					//pktdata.IsShooting = false;
					//retval = send(client_sock, (char*)&pktdata, sizeof(PKT_PLAYER_INFO), 0);

					//for (int i = 0; i < MAX_CLIENT; ++i)
					//{
					//	if (i != count)
					//	{
					//		anotherpktdata.Object_Type = m_pScene->m_pObjects[i]->m_Object_Type;
					//		anotherpktdata.Object_Index = i;
					//		anotherpktdata.WorldMatrix = m_pScene->m_pObjects[i]->m_xmf4x4World;
					//		retval = send(client_sock, (char*)&anotherpktdata, sizeof(PKT_CREATE_OBJECT), 0);
					//	}
					//}

					if (clients.size() > 0)
					{
						PKT_PLAYER_IN pkt_pin;
						pkt_pin.PktId = (char)PKT_ID_PLAYER_IN;
						pkt_pin.PktSize = (char)sizeof(PKT_PLAYER_IN);
						for (int i = 0; i < count; ++i)
						{
							pkt_pin.id = i;
							std::cout << i << "번쨰 플레이어 정보 정보 보냄\n";
							retval = send(client_sock, (char*)&pkt_pin, pkt_pin.PktSize, 0);
						}
						for (auto client : clients)
						{
							pkt_pin.id = count;
							std::cout << count << "번쨰 플레이어 정보 정보 입장\n";
							retval = send(client.socket, (char*)&pkt_pin, pkt_pin.PktSize, 0);
						}
					}

					m.lock();
					clients.emplace_back(Client_INFO{ count, client_sock, false });
					m.unlock();

					Arg* arg = new Arg;
					arg->pthis = this;
					arg->client_socket = client_sock;
					arg->id = count;

					client_Event[count] = CreateEvent(NULL, FALSE, FALSE, NULL);
					thread[count] = CreateThread(
						NULL, 0, client_thread,
						(LPVOID)arg, 0, NULL);
					count++;
				}
				else
				{
					closesocket(client_sock);
				}
			}
		}
	}
}

void Framework::err_quit(char * msg)
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

void Framework::err_display(char * msg)
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FLAG, NULL,
		WSAGetLastError(), LANG,
		(LPTSTR)&lpMsgBuf, 0, NULL);
	printf("[%s] %s", msg, (char*)lpMsgBuf);
	LocalFree(lpMsgBuf);
}

int Framework::recvn(SOCKET s, char * buf, int len, int flags)
{
	int received;
	char* ptr = buf;
	int left = len;

	while (left > 0)
	{
		received = recv(s, ptr, left, flags);
		if (received == SOCKET_ERROR)
		{
			std::cout << "recvn ERROR : ";
			return SOCKET_ERROR;
		}
		else if (received == 0)
			break;
		left -= received;
		ptr += received;
	}

	return (len - left);
}

int Framework::Send_msg(char * buf, int len, int flags)
{
	m.lock();
	for (auto d : clients)
	{
		send(d.socket, buf, len, flags);
	}
	m.unlock();
	return 0;
}

DWORD __stdcall Framework::Update(LPVOID arg)
{
	Update_Arg* t_arg = (Update_Arg*)arg;
	return t_arg->pthis->Update_Process(t_arg->pScene);
}

DWORD Framework::Update_Process(CScene* pScene)
{
	while (true)
	{
		//std::cout << "게임시작 대기중\n";
		if (game_start)
		{
			playernum = count;
			count = -1;
			std::cout << "게임 시작!\n";
			break;
		}
	}

	int retval;
	CGameObject** Objects = pScene->GetObjects(OBJECT_TYPE_OBSTACLE);

	PKT_GAME_START pktgamestart;
	pktgamestart.PktID = PKT_ID_GAME_START;
	pktgamestart.PktSize = sizeof(PKT_GAME_START);
	Send_msg((char*)&pktgamestart, pktgamestart.PktSize, 0);

	//로딩끝날때까지 대기
	int load_count;
	while (true)
	{
		load_count = 0;
		m.lock();
		for (auto d : clients)
		{
			if (d.load_complete)
				load_count++;
		}
		m.unlock();
		if (load_count == playernum)
			break;
	}

	PKT_LOAD_COMPLETE pktgamestate;
	pktgamestate.PktID = (char)PKT_ID_LOAD_COMPLETE_ALL;
	pktgamestate.PktSize = (char)sizeof(PKT_GAME_STATE);
	retval = Send_msg((char*)&pktgamestate, pktgamestate.PktSize, 0);
	std::cout << "전원 로드 완료\n";

	m.lock();
	for (auto d : clients)
	{
		PKT_PLAYER_INFO pktdata;
		pktdata.PktId = (char)PKT_ID_PLAYER_INFO;
		pktdata.PktSize = (char)sizeof(PKT_PLAYER_INFO);
		PKT_CREATE_OBJECT anotherpktdata;
		//std::cout << count << std::endl;
		pktdata.ID = d.id;
		pktdata.WorldMatrix = m_pScene->m_pObjects[d.id]->m_xmf4x4World;
		m_pScene->m_pObjects[d.id]->m_bPlay = true;
		//std::cout << pktdata.ID << " : " << pktdata.WorldMatrix._41 << ", " << pktdata.WorldMatrix._42 << ", " << pktdata.WorldMatrix._43 << std::endl;
		pktdata.IsShooting = false;
		for (int i = 0; i < playernum; ++i)
		{
			if (i != d.id)
			{
				anotherpktdata.PktId = (char)PKT_ID_CREATE_OBJECT;
				anotherpktdata.PktSize = (char)sizeof(PKT_CREATE_OBJECT);
				anotherpktdata.Object_Type = m_pScene->m_pObjects[i]->m_Object_Type;
				anotherpktdata.Object_Index = i;
				anotherpktdata.WorldMatrix = m_pScene->m_pObjects[i]->m_xmf4x4World;
				retval = send(d.socket, (char*)&pktdata, pktdata.PktSize, 0);
				retval = send(d.socket, (char*)&anotherpktdata, anotherpktdata.PktSize, 0);
			}
		}
	}
	m.unlock();

	game_start = false;

	while (true)
	{
		WaitForMultipleObjects(playernum, client_Event, TRUE, INFINITE);
		ResetEvent(Event);
		
		m_GameTimer.Tick(60.0f);
		elapsed_time = m_GameTimer.GetTimeElapsed();

		//서버의 시간을 모든 플레이어에게 보내줌
		PKT_ID id_time = PKT_ID_TIME_INFO;
		PKT_TIME_INFO server_time;
		server_time.PktId = (char)PKT_ID_TIME_INFO;
		server_time.PktSize = sizeof(PKT_TIME_INFO);
		server_time.elapsedtime = elapsed_time;
		retval = Send_msg((char*)&server_time, server_time.PktSize, 0);
		if (!spawn_item)
			item_cooltime += server_time.elapsedtime;
		for (int i = 0; i < 2; ++i)
		{
			if (!spawn_ammo[i])
				ammo_item_cooltime[i] += server_time.elapsedtime;
		}
		// 씬의 오브젝트 애니메이트
		pScene->AnimateObjects(server_time.elapsedtime);
		for (int i = MAX_CLIENT; i < MAX_NUM_OBJECT; ++i)
		{
			if (pScene->m_pObjects[i] != NULL)
			{
				PKT_UPDATE_OBJECT updateobj;
				updateobj.PktId = (char)PKT_ID_UPDATE_OBJECT;
				updateobj.PktSize = (char)sizeof(PKT_UPDATE_OBJECT);
				updateobj.Object_Index = i;
				updateobj.Object_Position = pScene->m_pObjects[i]->GetPosition();
				update_msg_queue.push(updateobj);
				if (pScene->m_pObjects[i]->IsDelete())
				{
					PKT_DELETE_OBJECT pkt_d;
					pkt_d.PktId = (char)PKT_ID_DELETE_OBJECT;
					pkt_d.PktSize = (char)sizeof(PKT_DELETE_OBJECT);
					pkt_d.Object_Index = i;
					delete_msg_queue.push(pkt_d);
				}
			}
		}
		// 충돌 처리
		CheckCollision(pScene);

		// 플레이어 정보를 기반으로 패킷 보내기
		while (true)
		{
			//std::cout << "플레이어 큐 크기 : " << msg_queue.size() << std::endl;
			PKT_PLAYER_INFO pkt;
			if (msg_queue.empty())
				break;

			pkt = msg_queue.front();
			msg_queue.pop();

			//std::cout << "Player_id : " << pkt.ID << std::endl;
			//std::cout << "Player_position : " << pkt.WorldMatrix._41 << " " << pkt.WorldMatrix._42 << " " << pkt.WorldMatrix._43 << std::endl;
			//std::cout << "Player_Weapon : " << pkt.Player_Weapon << std::endl;
			pScene->m_pObjects[pkt.ID]->SetWorldTransf(pkt.WorldMatrix);
			XMFLOAT3 p_position = pScene->m_pObjects[pkt.ID]->GetPosition();

			// 플레이어의 정보 전송
			retval = Send_msg((char*)&pkt, pkt.PktSize, 0);

			// 어떤 플레이어가 총알을 발사중인 상태이면 그 플레이어의 총알을 만드는 패킷을 전송
			if (pkt.IsShooting == TRUE)
			{
				//std::cout << "총알생성\n";
				PKT_CREATE_OBJECT bulletpkt;
				bulletpkt.PktId = (char)PKT_ID_CREATE_OBJECT;
				bulletpkt.PktSize = (char)sizeof(PKT_CREATE_OBJECT);
				if (pkt.Player_Weapon == WEAPON_TYPE_MACHINE_GUN)
					bulletpkt.Object_Type = OBJECT_TYPE_MACHINE_BULLET;
				else if (pkt.Player_Weapon == WEAPON_TYPE_BAZOOKA)
					bulletpkt.Object_Type = OBJECT_TYPE_BZK_BULLET;
				else if (pkt.Player_Weapon == WEAPON_TYPE_BEAM_RIFLE)
					bulletpkt.Object_Type = OBJECT_TYPE_BEAM_BULLET;
				bulletpkt.WorldMatrix = pkt.BulletWorldMatrix;
				bulletpkt.Object_Index = pScene->GetIndex();
				retval = Send_msg((char*)&bulletpkt, bulletpkt.PktSize, 0);

				CGameObject* bullet;
				if (pkt.Player_Weapon == WEAPON_TYPE_MACHINE_GUN)
				{
					bullet = new CGameObject(BULLET_TYPE_MACHINE_GUN);
					bullet->m_Object_Type = OBJECT_TYPE_MACHINE_BULLET;
				}
				else if (pkt.Player_Weapon == WEAPON_TYPE_BAZOOKA)
				{
					bullet = new CGameObject(BULLET_TYPE_BAZOOKA);
					bullet->m_Object_Type = OBJECT_TYPE_BZK_BULLET;
				}
				else if (pkt.Player_Weapon == WEAPON_TYPE_BEAM_RIFLE)
				{
					bullet = new CGameObject(BULLET_TYPE_BEAM_RIFLE);
					bullet->m_Object_Type = OBJECT_TYPE_BEAM_BULLET;
				}
				bullet->m_iId = pkt.ID;
				bullet->SetWorldTransf(pkt.BulletWorldMatrix);
				pScene->AddObject(bullet);
			}
		}

		// 회복 아이템 생성 패킷 보내기(60초마다 생성)
		if(item_cooltime >= ITEM_HEALING_COOLTIME && !spawn_item)
		{
			//std::cout << "아이템 생성\n";
			PKT_CREATE_OBJECT bulletpkt;
			bulletpkt.PktId = (char)PKT_ID_CREATE_OBJECT;
			bulletpkt.PktSize = (char)sizeof(PKT_CREATE_OBJECT);
			bulletpkt.Object_Type = OBJECT_TYPE_ITEM_HEALING;
			bulletpkt.WorldMatrix = XMFLOAT4X4{ 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f , 0.0f, 0.0f, 1.0f, 0.0f , 0.0f, 5.0f, 0.0f, 1.0f };
			bulletpkt.Object_Index = pScene->GetIndex();
			retval = Send_msg((char*)&bulletpkt, bulletpkt.PktSize, 0);

			CGameObject* item = new CGameObject;
			item->m_Object_Type = OBJECT_TYPE_ITEM_HEALING;
			item->m_iId = ITEM_ID;
			item->SetWorldTransf(XMFLOAT4X4{ 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f , 0.0f, 0.0f, 1.0f, 0.0f , 0.0f, 5.0f, 0.0f, 1.0f });
			pScene->AddObject(item);
			item_cooltime = 0.0f;
			spawn_item = true;
		}

		// 잔탄 아이템 생성 패킷 보내기(10초마다 생성)
		for (int i = 0; i < 2; ++i)
		{
			if (ammo_item_cooltime[i] >= ITEM_AMMO_COOLTIME && !spawn_ammo[i])
			{
				//std::cout << "아이템 생성\n";
				PKT_CREATE_OBJECT bulletpkt;
				bulletpkt.PktId = (char)PKT_ID_CREATE_OBJECT;
				bulletpkt.PktSize = (char)sizeof(PKT_CREATE_OBJECT);
				bulletpkt.Object_Type = OBJECT_TYPE_ITEM_AMMO;
				if(i == 0)
					bulletpkt.WorldMatrix = XMFLOAT4X4{ 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f , 0.0f, 0.0f, 1.0f, 0.0f , 100.0f, 5.0f, 0.0f, 1.0f };
				else
					bulletpkt.WorldMatrix = XMFLOAT4X4{ 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f , 0.0f, 0.0f, 1.0f, 0.0f , -100.0f, 5.0f, 0.0f, 1.0f };
				bulletpkt.Object_Index = pScene->GetIndex();
				retval = Send_msg((char*)&bulletpkt, bulletpkt.PktSize, 0);

				CGameObject* item = new CGameObject;
				item->m_Object_Type = OBJECT_TYPE_ITEM_AMMO;
				item->m_iId = ITEM_AMMO1+i;
				if (i == 0)
					item->SetWorldTransf(XMFLOAT4X4{ 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f , 0.0f, 0.0f, 1.0f, 0.0f , 100.0f, 5.0f, 0.0f, 1.0f });
				else
					item->SetWorldTransf(XMFLOAT4X4{ 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f , 0.0f, 0.0f, 1.0f, 0.0f , -100.0f, 5.0f, 0.0f, 1.0f });
				pScene->AddObject(item);
				ammo_item_cooltime[i] = 0.0f;
				spawn_ammo[i] = true;
			}
		}


		// 오브젝트 업데이트 패킷 보내기
		while (true)
		{
			PKT_UPDATE_OBJECT pkt_u;

			if (update_msg_queue.empty())
				break;

			pkt_u = update_msg_queue.front();
			update_msg_queue.pop();

			//std::cout << pkt_u.Object_Index << "번 오브젝트 업데이트 \n";
			retval = Send_msg((char*)&pkt_u, pkt_u.PktSize, 0);

		}

		// 오브젝트 삭제 패킷 보내기
		while (true)
		{
			PKT_DELETE_OBJECT pkt_d;

			if (delete_msg_queue.empty())
				break;

			pkt_d = delete_msg_queue.front();
			delete_msg_queue.pop();

			retval = Send_msg((char*)&pkt_d, pkt_d.PktSize, 0);
			//std::cout << "오브젝트 삭제 패킷 전송\n";
		}

		// 플레이어 체력/잔탄 변경 패킷 보내기
		while (true)
		{
			PKT_PLAYER_LIFE pkt_l;

			if (life_msg_queue.empty())
				break;

			pkt_l = life_msg_queue.front();
			life_msg_queue.pop();

			retval = Send_msg((char*)&pkt_l, pkt_l.PktSize, 0);
		}

		// 이펙트 생성 패킷 보내기
		while (true)
		{
			PKT_CREATE_EFFECT pkt_ce;

			if (effect_msg_queue.empty())
				break;

			pkt_ce = effect_msg_queue.front();
			effect_msg_queue.pop();

			retval = Send_msg((char*)&pkt_ce, pkt_ce.PktSize, 0);
		}

		PKT_SEND_COMPLETE pkt_cpl;
		pkt_cpl.PktID = (char)PKT_ID_SEND_COMPLETE;
		pkt_cpl.PktSize = (char)sizeof(PKT_SEND_COMPLETE);
		retval = Send_msg((char*)&pkt_cpl, pkt_cpl.PktSize, 0);
		//std::cout << "패킷 전송 완료\n";

		SetEvent(Event);
	}
}

DWORD __stdcall Framework::client_thread(LPVOID arg)
{
	Arg* t_arg = (Arg*)arg;
	Client_arg* c_arg = new Client_arg;
	c_arg->id = t_arg->id;
	std::cout << "id : " << c_arg->id << "\n";
	c_arg->socket = t_arg->client_socket;
	std::cout << ((Arg*)arg)->pthis << std::endl;
	return t_arg->pthis->client_process(c_arg);
}

DWORD Framework::client_process(Client_arg* arg)
{
	SOCKET client_socket = arg->socket;
	int retval;

	PKT_ID iPktID;
	int nPktSize = 0;

	char* buf;
	bool Player_out = false;
	std::cout << "id : " << arg->id << "\n";

	while (true)
	{
		if (!Player_out)
		{
			// 데이터 받기 # 패킷 식별 ID
			retval = recvn(client_socket, (char*)&iPktID, sizeof(PKT_ID), 0);
			if (retval == SOCKET_ERROR)
			{
				std::cout << "소켓 에러 : " << arg->id << "번 플레이어\n";
				Player_out = true;
				
				closesocket(client_socket);

				SetEvent(client_Event[arg->id]);
				WaitForSingleObject(Event, INFINITE);
			}
			else
			{
				// 데이터 받기 # 패킷 구조체 - SIZE 결정
				if (iPktID == PKT_ID_PLAYER_INFO) nPktSize = sizeof(PKT_PLAYER_INFO); // 플레이어 정보 [ 행렬, 상태 ]
				else if (iPktID == PKT_ID_GAME_START) nPktSize = sizeof(PKT_GAME_START); 
				else if (iPktID == PKT_ID_LOAD_COMPLETE) nPktSize = sizeof(PKT_LOAD_COMPLETE);
				else std::cout << "[ERROR] 패킷 ID 식별 불가 ID : " << iPktID << std::endl;

				// 데이터 받기 # 패킷 구조체 - 결정

				if (iPktID == PKT_ID_PLAYER_INFO)
				{
					buf = new char[nPktSize];
					retval = recvn(client_socket, buf, nPktSize, 0);
					if (retval == SOCKET_ERROR)	std::cout << "[ERROR] 데이터 받기 # 패킷 구조체 - 결정" << std::endl;
					std::cout << retval << "바이트 받음(플레이어 정보)\n";
					msg_queue.push(PKT_PLAYER_INFO{ ((PKT_PLAYER_INFO*)buf)->PktSize, ((PKT_PLAYER_INFO*)buf)->PktId, ((PKT_PLAYER_INFO*)buf)->ID,
						((PKT_PLAYER_INFO*)buf)->WorldMatrix, ((PKT_PLAYER_INFO*)buf)->IsShooting,
						((PKT_PLAYER_INFO*)buf)->BulletWorldMatrix, ((PKT_PLAYER_INFO*)buf)->Player_Weapon,
						((PKT_PLAYER_INFO*)buf)->isChangeWeapon, ((PKT_PLAYER_INFO*)buf)->Player_Animation,
						((PKT_PLAYER_INFO*)buf)->isChangeAnimation, ((PKT_PLAYER_INFO*)buf)->State });

					SetEvent(client_Event[arg->id]);
					WaitForSingleObject(Event, INFINITE);
				}
				else if (iPktID == PKT_ID_GAME_START)
				{
					buf = new char[nPktSize];
					retval = recvn(client_socket, buf, nPktSize, 0);
					if (retval == SOCKET_ERROR)	std::cout << "[ERROR] 데이터 받기 # 패킷 구조체 - 결정" << std::endl;
					game_start = true;
				}
				else if (iPktID == PKT_ID_LOAD_COMPLETE)
				{
					buf = new char[nPktSize];
					retval = recvn(client_socket, buf, nPktSize, 0);
					if (retval == SOCKET_ERROR)	std::cout << "[ERROR] 데이터 받기 # 패킷 구조체 - 결정" << std::endl;
					std::cout << arg->id << "번 플레이어 로드완료\n";
					//m.lock();
					clients[arg->id].load_complete = true;
					//m.unlock();
					continue;
				}
			}
		}
		if (!game_start)
		{
			SetEvent(client_Event[arg->id]);
			WaitForSingleObject(Event, INFINITE);
		}
	}

	return 0;
}

void Framework::CheckCollision(CScene* pScene)
{
	std::vector<CGameObject*> vPlayerBulletObjects;
	std::vector<CGameObject*> vHealingItem;
	std::vector<CGameObject*> vAmmoItem;
	PKT_DELETE_OBJECT pktDO;
	PKT_PLAYER_LIFE pktLF;
	PKT_CREATE_EFFECT pktCDE;
	PKT_CREATE_EFFECT pktCE;

	for (int i = MAX_CLIENT; i < MAX_NUM_OBJECT; ++i)
	{
		if (pScene->m_pObjects[i] != NULL)
		{
			if (pScene->m_pObjects[i]->m_Object_Type == OBJECT_TYPE_MACHINE_BULLET 
				|| pScene->m_pObjects[i]->m_Object_Type == OBJECT_TYPE_BZK_BULLET 
				|| pScene->m_pObjects[i]->m_Object_Type == OBJECT_TYPE_BEAM_BULLET)
				vPlayerBulletObjects.emplace_back(pScene->m_pObjects[i]);
			if (pScene->m_pObjects[i]->m_Object_Type == OBJECT_TYPE_ITEM_HEALING)
				vHealingItem.emplace_back(pScene->m_pObjects[i]);
			if(pScene->m_pObjects[i]->m_Object_Type == OBJECT_TYPE_ITEM_AMMO)
				vAmmoItem.emplace_back(pScene->m_pObjects[i]);
		}
	}

	// 플레이어와 플레이어의 총알 충돌체크
	for (const auto& Bullet : vPlayerBulletObjects)
	{
		for (int k = 0; k < MAX_CLIENT; ++k)
		{
			if (pScene->m_pObjects[k]->m_bPlay)
			{
				if (!Bullet->IsDelete())
				{
					if (Bullet->m_iId != pScene->m_pObjects[k]->m_iId)
					{
						if (Bullet->GetAABB().Intersects(pScene->m_pObjects[k]->GetAABB()))
						{
							XMFLOAT3 position = Bullet->GetPosition();
							pktCDE.PktId = PKT_ID_CREATE_EFFECT;
							pktCDE.PktSize = (char)sizeof(PKT_CREATE_EFFECT);
							pktCDE.efType = EFFECT_TYPE_HIT_FONT;
							pktCDE.EftAnitType = EFFECT_ANIMATION_TYPE_ONE;
							pktCDE.xmf3Position = pScene->m_pObjects[k]->GetPosition();
							pktCDE.xmf3Position.y += 20.0f;
							effect_msg_queue.push(pktCDE);
							pktCE.PktId = PKT_ID_CREATE_EFFECT;
							pktCE.PktSize = (char)sizeof(PKT_CREATE_EFFECT);
							pktCE.efType = EFFECT_TYPE_EXPLOSION;
							pktCE.EftAnitType = EFFECT_ANIMATION_TYPE_ONE;
							pktCE.xmf3Position = position;
							effect_msg_queue.push(pktCE);
							pktDO.PktId = (char)PKT_ID_DELETE_OBJECT;
							pktDO.PktSize = (char)sizeof(PKT_DELETE_OBJECT);
							pktDO.Object_Index = Bullet->index;
							delete_msg_queue.push(pktDO);
							pktLF.PktId = (char)PKT_ID_PLAYER_LIFE;
							pktLF.PktSize = (char)sizeof(PKT_PLAYER_LIFE);
							pktLF.ID = pScene->m_pObjects[k]->m_iId;
							pktLF.HP = Bullet->hp;
							life_msg_queue.push(pktLF);
							Bullet->Delete();
						}
					}
				}
			}
		}
	}

	//// 플레이어의 총알과 장애물의 충돌처리
	//for (const auto& Bullet : vPlayerBulletObjects)
	//{
	//	for (int k = 0; k < MAX_NUM_OBJECT; ++k)
	//	{
	//		if (pScene->m_pObstacles[k] != NULL)
	//		{
	//			if (!Bullet->IsDelete())
	//			{
	//				if (Bullet->GetAABB().Intersects(pScene->m_pObstacles[k]->GetAABB()))
	//				{
	//					XMFLOAT3 position = Bullet->GetPosition();
	//					pktCE.efType = EFFECT_TYPE_ONE;
	//					pktCE.xmf3Position = position;
	//					effect_msg_queue.push(pktCE);
	//					pktDO.Object_Index = Bullet->index;
	//					delete_msg_queue.push(pktDO);
	//					Bullet->Delete();
	//				}
	//			}
	//		}
	//	}
	//}

	// 플레이어와 회복아이템의 충돌체크
	for (const auto& Item : vHealingItem)
	{
		for (int k = 0; k < MAX_CLIENT; ++k)
		{
			if (pScene->m_pObjects[k]->m_bPlay)
			{
				if (!Item->IsDelete())
				{
					if (Item->m_iId != pScene->m_pObjects[k]->m_iId)
					{
						if (Item->GetAABB().Intersects(pScene->m_pObjects[k]->GetAABB()))
						{
							spawn_item = false;
							XMFLOAT3 position = Item->GetPosition();
							pktCE.PktId = PKT_ID_CREATE_EFFECT;
							pktCE.PktSize = (char)sizeof(PKT_CREATE_EFFECT);
							pktCE.efType = EFFECT_TYPE_HIT;
							pktCE.EftAnitType = EFFECT_ANIMATION_TYPE_ONE;
							pktCE.xmf3Position = position;
							effect_msg_queue.push(pktCE);
							pktDO.PktId = (char)PKT_ID_DELETE_OBJECT;
							pktDO.PktSize = (char)sizeof(PKT_DELETE_OBJECT);
							pktDO.Object_Index = Item->index;
							delete_msg_queue.push(pktDO);
							pktLF.PktId = (char)PKT_ID_PLAYER_LIFE;
							pktLF.PktSize = (char)sizeof(PKT_PLAYER_LIFE);
							pktLF.ID = pScene->m_pObjects[k]->m_iId;
							pktLF.HP = -50;
							pktLF.AMMO = 0;
							life_msg_queue.push(pktLF);
							Item->Delete();
						}
					}
				}
			}
		}
	}

	// 플레이어와 잔탄아이템의 충돌체크
	for (const auto& Item : vAmmoItem)
	{
		for (int k = 0; k < MAX_CLIENT; ++k)
		{
			if (pScene->m_pObjects[k]->m_bPlay)
			{
				if (!Item->IsDelete())
				{
					if (Item->m_iId != pScene->m_pObjects[k]->m_iId)
					{
						if (Item->GetAABB().Intersects(pScene->m_pObjects[k]->GetAABB()))
						{
							spawn_ammo[Item->m_iId-ITEM_AMMO1] = false;
							XMFLOAT3 position = Item->GetPosition();
							pktCE.PktId = PKT_ID_CREATE_EFFECT;
							pktCE.PktSize = (char)sizeof(PKT_CREATE_EFFECT);
							pktCE.efType = EFFECT_TYPE_HIT;
							pktCE.EftAnitType = EFFECT_ANIMATION_TYPE_ONE;
							pktCE.xmf3Position = position;
							effect_msg_queue.push(pktCE);
							pktDO.PktId = (char)PKT_ID_DELETE_OBJECT;
							pktDO.PktSize = (char)sizeof(PKT_DELETE_OBJECT);
							pktDO.Object_Index = Item->index;
							delete_msg_queue.push(pktDO);
							pktLF.PktId = (char)PKT_ID_PLAYER_LIFE;
							pktLF.PktSize = (char)sizeof(PKT_PLAYER_LIFE);
							pktLF.ID = pScene->m_pObjects[k]->m_iId;
							pktLF.HP = 0;
							pktLF.AMMO = 100;
							life_msg_queue.push(pktLF);
							Item->Delete();
						}
					}
				}
			}
		}
	}
}