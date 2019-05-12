#include "pch.h"
#include "Framework.h"
#include "GameObject.h"
#include "Protocol.h"

Framework::Framework()
{
	for (int i = 0; i < MAX_CLIENT; ++i)
	{
		thread[i] = NULL;
		clients[i].id = i;
		clients[i].socket = NULL;
		clients[i].load_complete = false;
		clients[i].enable = false;
	}

	count = 0;

	m_pRepository = new CRepository();
}

Framework::~Framework()
{
}

void Framework::Build()
{
	int retval;

	game_start = false;

	m_pScene = new CScene();
	m_pScene->BuildObjects(m_pRepository);

	// 윈속 초기화
	if (WSAStartup(WS22, &wsa) != 0)
		return;

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
		if (thread[i])
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
					int id = findindex();
					pkt_cid.Team = id % 2;
					if (id != -1)
						pkt_cid.id = id;
					else
						std::cout << "풀방임.\n";
					if (send(client_sock, (char*)&pkt_cid, pkt_cid.PktSize, 0) == SOCKET_ERROR)
					{
						std::cout << "main_loop ERROR : ";
						err_display("send()");
						break;
					}


					m.lock();
					int numclients = searchenables();
					m.unlock();

					if (numclients > 0)
					{
						PKT_PLAYER_IN pkt_pin;
						pkt_pin.PktId = (char)PKT_ID_PLAYER_IN;
						pkt_pin.PktSize = (char)sizeof(PKT_PLAYER_IN);
						for (int i = 0; i < MAX_CLIENT; ++i)
						{
							m.lock();
							bool enable = clients[i].enable;
							char team = clients[i].team;
							m.unlock();
							if (enable)
							{
								pkt_pin.id = i;
								pkt_pin.Team = team;
								std::cout << i << "번쨰 플레이어 정보를 새로 들어온 클라이언트에게 보냄\n";
								retval = send(client_sock, (char*)&pkt_pin, pkt_pin.PktSize, 0);
							}
						}
						m.lock();
						for (auto client : clients)
						{
							if (client.enable)
							{
								pkt_pin.id = id;
								pkt_pin.Team = id % 2;
								std::cout << id << "번쨰 플레이어의 입장 다른 플레이어에게 알려줌\n";
								retval = send(client.socket, (char*)&pkt_pin, pkt_pin.PktSize, 0);
							}
						}
						m.unlock();
					}

					m.lock();
					clients[id].id = id;
					clients[id].enable = true;
					clients[id].socket = client_sock;
					clients[id].selected_robot = ROBOT_TYPE_GM;
					clients[id].team = id % 2;
					m.unlock();

					Arg* arg = new Arg;
					arg->pthis = this;
					arg->client_socket = client_sock;
					arg->id = id;

					thread[id] = CreateThread(
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
		if(d.enable)
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
		//robby 처리
		while (true)
		{
			lobbyplayermutex.lock();
			if (lobby_player_msg_queue.size() > 0)
			{
				PKT_LOBBY_PLAYER_INFO RPIpkt;
				RPIpkt = lobby_player_msg_queue.front();
				lobby_player_msg_queue.pop();
				Send_msg((char*)&RPIpkt, RPIpkt.PktSize, 0);
			}
			lobbyplayermutex.unlock();

			if (game_start)
			{
				playernum = get_players();
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

		//모든 플레이어의 로딩끝날때까지 대기
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
			if (d.enable)
			{
				PKT_PLAYER_INFO pktdata;
				pktdata.PktId = (char)PKT_ID_PLAYER_INFO;
				pktdata.PktSize = (char)sizeof(PKT_PLAYER_INFO);
				pktdata.ID = d.id;
				pktdata.WorldMatrix = m_pScene->m_pObjects[d.id]->m_xmf4x4World;
				m_pScene->m_pObjects[d.id]->m_bPlay = true;
				m_pScene->m_pObjects[d.id]->m_iId = d.id;
				pktdata.IsShooting = false;
				PKT_CREATE_OBJECT anotherpktdata;
				for (int i = 0; i < playernum; ++i)
				{
					if (i != d.id)
					{
						anotherpktdata.PktId = (char)PKT_ID_CREATE_OBJECT;
						anotherpktdata.PktSize = (char)sizeof(PKT_CREATE_OBJECT);
						anotherpktdata.Object_Type = m_pScene->m_pObjects[i]->m_Object_Type;
						anotherpktdata.Object_Index = i;
						anotherpktdata.WorldMatrix = m_pScene->m_pObjects[i]->m_xmf4x4World;
						anotherpktdata.Robot_Type = clients[i].selected_robot;
						retval = send(d.socket, (char*)&pktdata, pktdata.PktSize, 0);
						retval = send(d.socket, (char*)&anotherpktdata, anotherpktdata.PktSize, 0);
					}
				}
			}
		}
		m.unlock();

		BlueScore = 100;
		RedScore = 100;
		PKT_SCORE scorepkt;
		scorepkt.PktSize = sizeof(PKT_SCORE);
		scorepkt.PktId = PKT_ID_SCORE;
		scorepkt.BlueScore = BlueScore;
		scorepkt.RedScore = RedScore;
		Send_msg((char*)&scorepkt, scorepkt.PktSize, 0);

		PlayGame(pScene);
	}

	return 0;
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
				std::cout << "소켓 에러 : " << arg->id << "번 플레이어 나감\n";
				Player_out = true;
				m.lock();
				clients[arg->id].enable = false;
				playernum--;
				m.unlock();
				closesocket(client_socket);
				PKT_PLAYER_OUT playerout;
				playerout.id = arg->id;
				playerout.PktId = PKT_ID_PLAYER_OUT;
				playerout.PktSize = sizeof(PKT_PLAYER_OUT);
				Send_msg((char*)&playerout, playerout.PktSize, 0);
				std::cout << arg->id << "번 플레이어 스레드 종료\n";
				m_pScene->m_pObjects[arg->id]->m_bPlay = false;
				break;
			}
			else
			{
				// 데이터 받기 # 패킷 구조체 - SIZE 결정
				if (iPktID == PKT_ID_PLAYER_INFO) nPktSize = sizeof(PKT_PLAYER_INFO); // 플레이어 정보 [ 행렬, 상태 ]
				else if (iPktID == PKT_ID_GAME_START) nPktSize = sizeof(PKT_GAME_START);
				else if (iPktID == PKT_ID_LOAD_COMPLETE) nPktSize = sizeof(PKT_LOAD_COMPLETE);
				else if (iPktID == PKT_ID_SHOOT) nPktSize = sizeof(PKT_SHOOT);
				else if (iPktID == PKT_ID_LOBBY_PLAYER_INFO) nPktSize = sizeof(PKT_LOBBY_PLAYER_INFO);
				else std::cout << "[ERROR] 패킷 ID 식별 불가 ID : " << iPktID << std::endl;

				// 데이터 받기 # 패킷 구조체 - 결정
				buf = new char[nPktSize];
				retval = recvn(client_socket, buf, nPktSize, 0);
				if (retval == SOCKET_ERROR)	std::cout << "[ERROR] 데이터 받기 # 패킷 구조체 - 결정" << std::endl;

				if (iPktID == PKT_ID_PLAYER_INFO)
				{
					playerinfomutex.lock();
					msg_queue.push(PKT_PLAYER_INFO{ ((PKT_PLAYER_INFO*)buf)->PktSize, ((PKT_PLAYER_INFO*)buf)->PktId,
						((PKT_PLAYER_INFO*)buf)->ID, ((PKT_PLAYER_INFO*)buf)->WorldMatrix,
						((PKT_PLAYER_INFO*)buf)->IsShooting, ((PKT_PLAYER_INFO*)buf)->Player_Weapon,
						((PKT_PLAYER_INFO*)buf)->isChangeWeapon, ((PKT_PLAYER_INFO*)buf)->Player_Up_Animation,
						((PKT_PLAYER_INFO*)buf)->isUpChangeAnimation, ((PKT_PLAYER_INFO*)buf)->UpAnimationPosition,
						((PKT_PLAYER_INFO*)buf)->Player_Down_Animation, ((PKT_PLAYER_INFO*)buf)->isDownChangeAnimation,
						((PKT_PLAYER_INFO*)buf)->DownAnimationPosition, ((PKT_PLAYER_INFO*)buf)->State });
					playerinfomutex.unlock();
					if (((PKT_PLAYER_INFO*)buf)->Player_Up_Animation == ANIMATION_TYPE_BEAM_SABER_1_ONE ||
						((PKT_PLAYER_INFO*)buf)->Player_Up_Animation == ANIMATION_TYPE_BEAM_SABER_2_ONE ||
						((PKT_PLAYER_INFO*)buf)->Player_Up_Animation == ANIMATION_TYPE_BEAM_SABER_3_ONE)

					if (((PKT_PLAYER_INFO*)buf)->Player_Up_Animation == ANIMATION_TYPE_BEAM_SABER_1_ONE)
						if (((PKT_PLAYER_INFO*)buf)->UpAnimationPosition >= 0.3f)
						{
							if (!m_pScene->m_BeamsaberCollisionmesh[3 * ((PKT_PLAYER_INFO*)buf)->ID].in_used)
							{
								m_pScene->m_BeamsaberCollisionmesh[3 * ((PKT_PLAYER_INFO*)buf)->ID + 2].in_used = false;
								m_pScene->m_BeamsaberCollisionmesh[3 * ((PKT_PLAYER_INFO*)buf)->ID].in_used = true;
								m_pScene->m_BeamsaberCollisionmesh[3 * ((PKT_PLAYER_INFO*)buf)->ID].m_iId = ((PKT_PLAYER_INFO*)buf)->ID;
								m_pScene->m_BeamsaberCollisionmesh[3 * ((PKT_PLAYER_INFO*)buf)->ID].SetWorldTransf(((PKT_PLAYER_INFO*)buf)->WorldMatrix);
								XMFLOAT3 lookvector = { ((PKT_PLAYER_INFO*)buf)->WorldMatrix._31,((PKT_PLAYER_INFO*)buf)->WorldMatrix._32,((PKT_PLAYER_INFO*)buf)->WorldMatrix._33 };
								lookvector = Vector3::ScalarProduct(lookvector, 10.0f, false);
								lookvector = Vector3::Add(m_pScene->m_BeamsaberCollisionmesh[3 * ((PKT_PLAYER_INFO*)buf)->ID].GetPosition(), lookvector);
								m_pScene->m_BeamsaberCollisionmesh[3 * ((PKT_PLAYER_INFO*)buf)->ID].SetPosition(lookvector.x, lookvector.y, lookvector.z);
							}
						}
					if (((PKT_PLAYER_INFO*)buf)->Player_Up_Animation == ANIMATION_TYPE_BEAM_SABER_2_ONE)
						if (((PKT_PLAYER_INFO*)buf)->UpAnimationPosition >= 0.35f)
						{
							if (!m_pScene->m_BeamsaberCollisionmesh[3 * ((PKT_PLAYER_INFO*)buf)->ID + 1].in_used)
							{
								m_pScene->m_BeamsaberCollisionmesh[3 * ((PKT_PLAYER_INFO*)buf)->ID].in_used = false;
								m_pScene->m_BeamsaberCollisionmesh[3 * ((PKT_PLAYER_INFO*)buf)->ID + 1].in_used = true;
								m_pScene->m_BeamsaberCollisionmesh[3 * ((PKT_PLAYER_INFO*)buf)->ID + 1].m_iId = ((PKT_PLAYER_INFO*)buf)->ID;
								m_pScene->m_BeamsaberCollisionmesh[3 * ((PKT_PLAYER_INFO*)buf)->ID + 1].SetWorldTransf(((PKT_PLAYER_INFO*)buf)->WorldMatrix);
								XMFLOAT3 lookvector = { ((PKT_PLAYER_INFO*)buf)->WorldMatrix._31,((PKT_PLAYER_INFO*)buf)->WorldMatrix._32,((PKT_PLAYER_INFO*)buf)->WorldMatrix._33 };
								lookvector = Vector3::ScalarProduct(lookvector, 15.0f, false);
								lookvector = Vector3::Add(m_pScene->m_BeamsaberCollisionmesh[3 * ((PKT_PLAYER_INFO*)buf)->ID + 1].GetPosition(), lookvector);
								m_pScene->m_BeamsaberCollisionmesh[3 * ((PKT_PLAYER_INFO*)buf)->ID + 1].SetPosition(lookvector.x, lookvector.y, lookvector.z);
							}
						}
					if (((PKT_PLAYER_INFO*)buf)->Player_Up_Animation == ANIMATION_TYPE_BEAM_SABER_3_ONE)
						if (((PKT_PLAYER_INFO*)buf)->UpAnimationPosition >= 0.5f)
						{
							if (!m_pScene->m_BeamsaberCollisionmesh[3 * ((PKT_PLAYER_INFO*)buf)->ID + 2].in_used)
							{
								m_pScene->m_BeamsaberCollisionmesh[3 * ((PKT_PLAYER_INFO*)buf)->ID + 1].in_used = false;
								m_pScene->m_BeamsaberCollisionmesh[3 * ((PKT_PLAYER_INFO*)buf)->ID + 2].in_used = true;
								m_pScene->m_BeamsaberCollisionmesh[3 * ((PKT_PLAYER_INFO*)buf)->ID + 2].m_iId = ((PKT_PLAYER_INFO*)buf)->ID;
								m_pScene->m_BeamsaberCollisionmesh[3 * ((PKT_PLAYER_INFO*)buf)->ID + 2].SetWorldTransf(((PKT_PLAYER_INFO*)buf)->WorldMatrix);
								XMFLOAT3 lookvector = { ((PKT_PLAYER_INFO*)buf)->WorldMatrix._31,((PKT_PLAYER_INFO*)buf)->WorldMatrix._32,((PKT_PLAYER_INFO*)buf)->WorldMatrix._33 };
								lookvector = Vector3::ScalarProduct(lookvector, 15.0f, false);
								lookvector = Vector3::Add(m_pScene->m_BeamsaberCollisionmesh[2].GetPosition(), lookvector);
								m_pScene->m_BeamsaberCollisionmesh[3 * ((PKT_PLAYER_INFO*)buf)->ID + 2].SetPosition(lookvector.x, lookvector.y, lookvector.z);
							}
						}
				}
				else if (iPktID == PKT_ID_SHOOT)
				{
					shootmutex.lock();
					shoot_msg_queue.push(PKT_SHOOT{ ((PKT_SHOOT*)buf)->PktSize, ((PKT_SHOOT*)buf)->PktId,
						((PKT_SHOOT*)buf)->ID,  ((PKT_SHOOT*)buf)->Player_Weapon, ((PKT_SHOOT*)buf)->BulletWorldMatrix });
					shootmutex.unlock();
				}
				else if (iPktID == PKT_ID_GAME_START)
				{
					game_start = true;
				}
				else if (iPktID == PKT_ID_LOAD_COMPLETE)
				{
					std::cout << arg->id << "번 플레이어 로드완료\n";
					m.lock();
					clients[arg->id].load_complete = true;
					m.unlock();
				}
				else if (iPktID == PKT_ID_LOBBY_PLAYER_INFO)
				{
					lobbyplayermutex.lock();
					lobby_player_msg_queue.push(PKT_LOBBY_PLAYER_INFO{ ((PKT_LOBBY_PLAYER_INFO*)buf)->PktSize, ((PKT_LOBBY_PLAYER_INFO*)buf)->PktId,
						((PKT_LOBBY_PLAYER_INFO*)buf)->id,  ((PKT_LOBBY_PLAYER_INFO*)buf)->selected_robot, ((PKT_LOBBY_PLAYER_INFO*)buf)->Team });
					lobbyplayermutex.unlock();
					ROBOT_TYPE rt;
					if (((PKT_LOBBY_PLAYER_INFO*)buf)->selected_robot == 0)
						rt = ROBOT_TYPE_GM;
					else if (((PKT_LOBBY_PLAYER_INFO*)buf)->selected_robot == 1)
						rt = ROBOT_TYPE_GUNDAM;
					m.lock();
					clients[((PKT_LOBBY_PLAYER_INFO*)buf)->id].selected_robot = rt;
					clients[((PKT_LOBBY_PLAYER_INFO*)buf)->id].team = ((PKT_LOBBY_PLAYER_INFO*)buf)->Team;
					m.unlock();
				}
			}
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
			if (pScene->m_pObjects[i]->m_Object_Type == OBJECT_TYPE_ITEM_AMMO)
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
						if ((Bullet->GetAABB())[0].Intersects((pScene->m_pObjects[k]->GetAABB())[0]))
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
							if(Bullet->m_Object_Type == OBJECT_TYPE_BZK_BULLET)
								pktCE.efType = EFFECT_TYPE_EXPLOSION;
							else
								pktCE.efType = EFFECT_TYPE_HIT;
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

							pScene->m_pObjects[k]->hp -= Bullet->hp;
							if (pScene->m_pObjects[k]->hp <= 0)
							{
								if (pScene->m_pObjects[k]->m_iId % 2 == 0)
								{
									RedScore -= 5;
								}
								else
								{
									BlueScore -= 5;
								}

								PKT_SCORE scorepkt;
								scorepkt.PktSize = sizeof(PKT_SCORE);
								scorepkt.PktId = PKT_ID_SCORE;
								scorepkt.BlueScore = BlueScore;
								scorepkt.RedScore = RedScore;
								Send_msg((char*)&scorepkt, scorepkt.PktSize, 0);
								pScene->m_pObjects[k]->hp = PLAYER_HP;
							}
						}
					}
				}
			}
		}
	}

	// 플레이어의 총알과 장애물의 충돌처리
	for (const auto& Bullet : vPlayerBulletObjects)
	{
		for (auto obstacle : pScene->Obstacles)
		{
			if (!Bullet->IsDelete())
			{
				if ((Bullet->GetAABB())[0].Intersects((obstacle->GetAABB())[0]))
				{
					XMFLOAT3 position = Bullet->GetPosition();
					pktCE.PktId = PKT_ID_CREATE_EFFECT;
					pktCE.PktSize = (char)sizeof(PKT_CREATE_EFFECT);
					if (Bullet->m_Object_Type == OBJECT_TYPE_BZK_BULLET)
						pktCE.efType = EFFECT_TYPE_EXPLOSION;
					else
						pktCE.efType = EFFECT_TYPE_HIT;
					pktCE.EftAnitType = EFFECT_ANIMATION_TYPE_ONE;
					pktCE.xmf3Position = position;
					effect_msg_queue.push(pktCE);
					pktDO.PktId = (char)PKT_ID_DELETE_OBJECT;
					pktDO.PktSize = (char)sizeof(PKT_DELETE_OBJECT);
					pktDO.Object_Index = Bullet->index;
					delete_msg_queue.push(pktDO);
					Bullet->Delete();
				}
			}
		}
	}

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
						if ((Item->GetAABB())[0].Intersects((pScene->m_pObjects[k]->GetAABB())[0]))
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
						if ((Item->GetAABB())[0].Intersects((pScene->m_pObjects[k]->GetAABB())[0]))
						{
							spawn_ammo[Item->m_iId - ITEM_AMMO1] = false;
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

	// 플레이어와 빔사벨 충돌
	for (int i = 0; i < 24; ++i)
	{
		for (int k = 0; k < MAX_CLIENT; ++k)
		{
			if (pScene->m_pObjects[k]->m_bPlay)
			{
				if (pScene->m_BeamsaberCollisionmesh[i].in_used)
				{
					if (pScene->m_BeamsaberCollisionmesh[i].m_iId != pScene->m_pObjects[k]->m_iId)
					{
						if ((pScene->m_BeamsaberCollisionmesh[i].GetAABB())[0].Intersects((pScene->m_pObjects[k]->GetAABB())[0]))
						{
							pktCE.PktId = PKT_ID_CREATE_EFFECT;
							pktCE.PktSize = (char)sizeof(PKT_CREATE_EFFECT);
							pktCE.efType = EFFECT_TYPE_HIT_FONT;
							pktCE.EftAnitType = EFFECT_ANIMATION_TYPE_ONE;
							pktCE.xmf3Position = pScene->m_pObjects[k]->GetPosition();
							pktCE.xmf3Position.y += 20.0f;
							effect_msg_queue.push(pktCE);
							pktCDE.PktId = PKT_ID_CREATE_EFFECT;
							pktCDE.PktSize = (char)sizeof(PKT_CREATE_EFFECT);
							pktCDE.efType = EFFECT_TYPE_HIT;
							pktCDE.EftAnitType = EFFECT_ANIMATION_TYPE_ONE;
							pktCDE.xmf3Position = pScene->m_pObjects[k]->GetPosition();
							pktCDE.xmf3Position.y += 10.0f;
							effect_msg_queue.push(pktCDE);
							pktLF.PktId = (char)PKT_ID_PLAYER_LIFE;
							pktLF.PktSize = (char)sizeof(PKT_PLAYER_LIFE);
							pktLF.ID = pScene->m_pObjects[k]->m_iId;
							pktLF.HP = m_pScene->m_BeamsaberCollisionmesh[i].hp;
							pktLF.AMMO = 0;
							life_msg_queue.push(pktLF);
							m_pScene->m_BeamsaberCollisionmesh[i].in_used = false;

							pScene->m_pObjects[k]->hp -= pScene->m_BeamsaberCollisionmesh[i].hp;
							if (pScene->m_pObjects[k]->hp <= 0)
							{
								if (pScene->m_pObjects[k]->m_iId % 2 == 0)
								{
									RedScore -= 5;
								}
								else
								{
									BlueScore -= 5;
								}

								PKT_SCORE scorepkt;
								scorepkt.PktSize = sizeof(PKT_SCORE);
								scorepkt.PktId = PKT_ID_SCORE;
								scorepkt.BlueScore = BlueScore;
								scorepkt.RedScore = RedScore;
								Send_msg((char*)&scorepkt, scorepkt.PktSize, 0);
								pScene->m_pObjects[k]->hp = PLAYER_HP;
							}
						}
					}

					//m_pScene->m_BeamsaberCollisionmesh[i].in_used = false;
				}
			}
		}
	}
}

int Framework::findindex()
{
	for (int i = 0; i < MAX_CLIENT; ++i)
	{
		bool enable = clients[i].enable;
		if(enable == false)
			return i;
	}
	return -1;
}

int Framework::searchenables()
{
	int returnvalue = 0;
	for (int i = 0; i < MAX_CLIENT; ++i)
	{
		if (clients[i].enable)
			returnvalue++;
	}
	return returnvalue;
}

void Framework::SendCreateEffect(CScene* pScene)
{
	while (!effect_msg_queue.empty())
	{
		PKT_CREATE_EFFECT pkt_ce;

		pkt_ce = effect_msg_queue.front();
		effect_msg_queue.pop();

		Send_msg((char*)&pkt_ce, pkt_ce.PktSize, 0);
	}
}

void Framework::SendPlayerInfo(CScene* pScene)
{
	while (!msg_queue.empty())
	{
		PKT_PLAYER_INFO pkt;

		pkt = msg_queue.front();
		msg_queue.pop();

		pScene->m_pObjects[pkt.ID]->SetWorldTransf(pkt.WorldMatrix);

		// 플레이어의 정보 전송
		Send_msg((char*)&pkt, pkt.PktSize, 0);
	}
}

void Framework::SendUpfateObject(CScene* pScene)
{
	while (!update_msg_queue.empty())
	{
		PKT_UPDATE_OBJECT pkt_u;

		pkt_u = update_msg_queue.front();
		update_msg_queue.pop();

		//std::cout << pkt_u.Object_Index << "번 오브젝트 업데이트 \n";
		Send_msg((char*)&pkt_u, pkt_u.PktSize, 0);
	}
}

void Framework::SendDeleteObject(CScene* pScene)
{
	while (!delete_msg_queue.empty())
	{
		PKT_DELETE_OBJECT pkt_d;

		pkt_d = delete_msg_queue.front();
		delete_msg_queue.pop();

		Send_msg((char*)&pkt_d, pkt_d.PktSize, 0);
	}
}

void Framework::SendPlayerLife(CScene* pScene)
{
	while (!life_msg_queue.empty())
	{
		PKT_PLAYER_LIFE pkt_l;

		pkt_l = life_msg_queue.front();
		life_msg_queue.pop();

		Send_msg((char*)&pkt_l, pkt_l.PktSize, 0);
	}
}

void Framework::SendCreateBullet(CScene * pScene)
{

	while (!shoot_msg_queue.empty())
	{
		PKT_SHOOT shootpkt;

		shootpkt = shoot_msg_queue.front();
		shoot_msg_queue.pop();

		//std::cout << "총알생성\n";
		PKT_CREATE_OBJECT bulletpkt;
		bulletpkt.PktId = (char)PKT_ID_CREATE_OBJECT;
		bulletpkt.PktSize = (char)sizeof(PKT_CREATE_OBJECT);
		if (shootpkt.Player_Weapon == WEAPON_TYPE_MACHINE_GUN)
			bulletpkt.Object_Type = OBJECT_TYPE_MACHINE_BULLET;
		else if (shootpkt.Player_Weapon == WEAPON_TYPE_BAZOOKA)
			bulletpkt.Object_Type = OBJECT_TYPE_BZK_BULLET;
		else if (shootpkt.Player_Weapon == WEAPON_TYPE_BEAM_RIFLE)
			bulletpkt.Object_Type = OBJECT_TYPE_BEAM_BULLET;
		bulletpkt.WorldMatrix = shootpkt.BulletWorldMatrix;
		bulletpkt.Object_Index = pScene->GetIndex();
		Send_msg((char*)&bulletpkt, bulletpkt.PktSize, 0);

		CGameObject* bullet;
		if (shootpkt.Player_Weapon == WEAPON_TYPE_MACHINE_GUN)
		{
			bullet = new CGameObject(BULLET_TYPE_MACHINE_GUN);
			bullet->m_Object_Type = OBJECT_TYPE_MACHINE_BULLET;
		}
		else if (shootpkt.Player_Weapon == WEAPON_TYPE_BAZOOKA)
		{
			bullet = new CGameObject(BULLET_TYPE_BAZOOKA);
			bullet->m_Object_Type = OBJECT_TYPE_BZK_BULLET;
		}
		else if (shootpkt.Player_Weapon == WEAPON_TYPE_BEAM_RIFLE)
		{
			bullet = new CGameObject(BULLET_TYPE_BEAM_RIFLE);
			bullet->m_Object_Type = OBJECT_TYPE_BEAM_BULLET;
		}
		bullet->m_iId = shootpkt.ID;
		bullet->SetWorldTransf(shootpkt.BulletWorldMatrix);
		pScene->AddObject(bullet);

	}
}

void Framework::SendCreateHeal(CScene * pScene)
{
	if (item_cooltime >= ITEM_HEALING_COOLTIME && !spawn_item)
	{
		//std::cout << "아이템 생성\n";
		PKT_CREATE_OBJECT bulletpkt;
		bulletpkt.PktId = (char)PKT_ID_CREATE_OBJECT;
		bulletpkt.PktSize = (char)sizeof(PKT_CREATE_OBJECT);
		bulletpkt.Object_Type = OBJECT_TYPE_ITEM_HEALING;
		bulletpkt.WorldMatrix = XMFLOAT4X4{ 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f , 0.0f, 0.0f, 1.0f, 0.0f , 0.0f, 5.0f, 0.0f, 1.0f };
		bulletpkt.Object_Index = pScene->GetIndex();
		Send_msg((char*)&bulletpkt, bulletpkt.PktSize, 0);

		CGameObject* item = new CGameObject;
		item->m_Object_Type = OBJECT_TYPE_ITEM_HEALING;
		item->m_iId = ITEM_ID;
		item->SetWorldTransf(XMFLOAT4X4{ 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f , 0.0f, 0.0f, 1.0f, 0.0f , 0.0f, 5.0f, 0.0f, 1.0f });
		pScene->AddObject(item);
		item_cooltime = 0.0f;
		spawn_item = true;
	}
}

void Framework::SendCreateAmmo(CScene * pScene)
{
	for (int i = 0; i < 2; ++i)
	{
		if (ammo_item_cooltime[i] >= ITEM_AMMO_COOLTIME && !spawn_ammo[i])
		{
			//std::cout << "아이템 생성\n";
			PKT_CREATE_OBJECT bulletpkt;
			bulletpkt.PktId = (char)PKT_ID_CREATE_OBJECT;
			bulletpkt.PktSize = (char)sizeof(PKT_CREATE_OBJECT);
			bulletpkt.Object_Type = OBJECT_TYPE_ITEM_AMMO;
			if (i == 0)
				bulletpkt.WorldMatrix = XMFLOAT4X4{ 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f , 0.0f, 0.0f, 1.0f, 0.0f , 100.0f, 5.0f, 0.0f, 1.0f };
			else
				bulletpkt.WorldMatrix = XMFLOAT4X4{ 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f , 0.0f, 0.0f, 1.0f, 0.0f , -100.0f, 5.0f, 0.0f, 1.0f };
			bulletpkt.Object_Index = pScene->GetIndex();
			Send_msg((char*)&bulletpkt, bulletpkt.PktSize, 0);

			CGameObject* item = new CGameObject;
			item->m_Object_Type = OBJECT_TYPE_ITEM_AMMO;
			item->m_iId = ITEM_AMMO1 + i;
			if (i == 0)
				item->SetWorldTransf(XMFLOAT4X4{ 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f , 0.0f, 0.0f, 1.0f, 0.0f , 100.0f, 5.0f, 0.0f, 1.0f });
			else
				item->SetWorldTransf(XMFLOAT4X4{ 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f , 0.0f, 0.0f, 1.0f, 0.0f , -100.0f, 5.0f, 0.0f, 1.0f });
			pScene->AddObject(item);
			ammo_item_cooltime[i] = 0.0f;
			spawn_ammo[i] = true;
		}
	}
}

void Framework::SendTime(CScene * pScene)
{
	PKT_ID id_time = PKT_ID_TIME_INFO;
	PKT_TIME_INFO server_time;
	server_time.PktId = (char)PKT_ID_TIME_INFO;
	server_time.PktSize = sizeof(PKT_TIME_INFO);
	server_time.elapsedtime = elapsed_time;
	Send_msg((char*)&server_time, server_time.PktSize, 0);
	if (!spawn_item)
		item_cooltime += server_time.elapsedtime;
	for (int i = 0; i < 2; ++i)
	{
		if (!spawn_ammo[i])
			ammo_item_cooltime[i] += server_time.elapsedtime;
	}
}

void Framework::UpdateScene(CScene * pScene)
{
	pScene->AnimateObjects(elapsed_time);
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

				if (pScene->m_pObjects[i]->m_Object_Type == OBJECT_TYPE_MACHINE_BULLET
					|| pScene->m_pObjects[i]->m_Object_Type == OBJECT_TYPE_BZK_BULLET
					|| pScene->m_pObjects[i]->m_Object_Type == OBJECT_TYPE_BEAM_BULLET)
				{
					PKT_CREATE_EFFECT pktCE;
					pktCE.PktId = PKT_ID_CREATE_EFFECT;
					pktCE.PktSize = (char)sizeof(PKT_CREATE_EFFECT);
					if (pScene->m_pObjects[i]->m_Object_Type == OBJECT_TYPE_BZK_BULLET)
						pktCE.efType = EFFECT_TYPE_EXPLOSION;
					else
						pktCE.efType = EFFECT_TYPE_HIT;
					pktCE.EftAnitType = EFFECT_ANIMATION_TYPE_ONE;
					pktCE.xmf3Position = pScene->m_pObjects[i]->GetPosition();
					effect_msg_queue.push(pktCE);
				}
			}
		}
	}
}

void Framework::PlayGame(CScene * pScene)
{
	while (true)
	{
		if (BlueScore > 0 && RedScore > 0 && playernum > 0)
		{
			m_GameTimer.Tick(60.0f);
			elapsed_time = m_GameTimer.GetTimeElapsed();

			//서버의 시간을 모든 플레이어에게 보내줌
			SendTime(pScene);

			// 씬의 오브젝트 애니메이트
			UpdateScene(pScene);

			// 충돌 처리
			CheckCollision(pScene);

			// 플레이어 정보 보내기
			SendPlayerInfo(pScene);

			// 총알 생성 보내기
			SendCreateBullet(pScene);

			// 회복 아이템 생성 패킷 보내기(60초마다 생성)
			SendCreateHeal(pScene);

			// 잔탄 아이템 생성 패킷 보내기(10초마다 생성)
			SendCreateAmmo(pScene);


			// 오브젝트 업데이트 패킷 보내기
			SendUpfateObject(pScene);

			// 오브젝트 삭제 패킷 보내기
			SendDeleteObject(pScene);

			// 플레이어 체력/잔탄 변경 패킷 보내기
			SendPlayerLife(pScene);

			// 이펙트 생성 패킷 보내기
			SendCreateEffect(pScene);

			PKT_SEND_COMPLETE pkt_cpl;
			pkt_cpl.PktID = (char)PKT_ID_SEND_COMPLETE;
			pkt_cpl.PktSize = (char)sizeof(PKT_SEND_COMPLETE);
			Send_msg((char*)&pkt_cpl, pkt_cpl.PktSize, 0);
			//std::cout << "패킷 전송 완료\n";
		}
		else if (playernum <= 0)
		{
			playernum = 0;
			count = 0;
			game_start = false;
			std::cout << "플레이어가 0명이여서 게임을 종료\n";

			while(!msg_queue.empty())
				msg_queue.pop();
			while (!life_msg_queue.empty())
				life_msg_queue.pop();
			while (!delete_msg_queue.empty())
				delete_msg_queue.pop();
			while (!update_msg_queue.empty())
				update_msg_queue.pop();
			while (!effect_msg_queue.empty())
				effect_msg_queue.pop();
			while (!shoot_msg_queue.empty())
				shoot_msg_queue.pop();
			while (!lobby_player_msg_queue.empty())
				lobby_player_msg_queue.pop();
			break;
		}
		else
		{
			PKT_GAME_END pkt_gend;
			pkt_gend.PktId = (char)PKT_ID_GAME_END;
			pkt_gend.PktSize = (char)sizeof(PKT_GAME_END);
			if (RedScore == 0)
				pkt_gend.WinTeam = 0;
			else
				pkt_gend.WinTeam = 1;
			Send_msg((char*)&pkt_gend, pkt_gend.PktSize, 0);

			game_start = false;
			playernum = 0;
			count = get_players();
			std::cout << "점수가 0이된 팀이 있어서 게임을 종료\n";
			break;
		}
	}
}

int Framework::get_players()
{
	int num = 0;
	m.lock();
	for (auto c : clients)
	{
		if (c.enable)
			num++;
	}
	m.unlock();

	return num;
}
