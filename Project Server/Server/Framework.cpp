#include "pch.h"
#include "Framework.h"
#include "GameObject.h"

Framework::Framework()
{
	for (int i = 0; i < MAX_CLIENT; ++i)
		thread[i] = NULL;

	m_pScene = new CScene();
	m_pScene->BuildObjects();

	count = 0;
}

Framework::~Framework()
{
}

int Framework::Build()
{
	int retval;

	Event = CreateEvent(NULL, TRUE, FALSE, NULL);

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
	WaitForMultipleObjects(2, thread, true, INFINITE);

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
				memcpy(buf, &count, sizeof(int));
				retval = send(client_sock, buf, sizeof(int), 0);
				if (retval == SOCKET_ERROR)
				{
					std::cout << "main_loop ERROR : ";
					err_display("send()");
					break;
				}

				Client_INFO info;
				info.id = count;
				info.socket = client_sock;
				m.lock();
				clients.emplace_back(Client_INFO{ count, client_sock });
				m.unlock();

				SCENEINFO sinfo = SCENE_NAME_COLONY;
				retval = send(client_sock, (char*)&sinfo, sizeof(SCENEINFO), 0);

				CGameObject** Objects = m_pScene->GetObjects(OBJECT_TYPE_OBSTACLE);
				PKT_PLAYER_INFO pktdata;
				PKT_CREATE_OBJECT anotherpktdata;
				std::cout << count << std::endl;
				if (count == 0)
				{
					pktdata.ID = clients[0].id;
					pktdata.WorldMatrix = m_pScene->m_pObjects[0]->m_xmf4x4World;
					std::cout << pktdata.ID << " : " << pktdata.WorldMatrix._41 << ", " << pktdata.WorldMatrix._42 << ", " << pktdata.WorldMatrix._43 << std::endl;
					pktdata.IsShooting = false;
					retval = send(client_sock, (char*)&pktdata, sizeof(PKT_PLAYER_INFO), 0);

					anotherpktdata.Object_Type = m_pScene->m_pObjects[1]->m_Object_Type;
					anotherpktdata.Object_Index = 1;
					anotherpktdata.WorldMatrix = m_pScene->m_pObjects[1]->m_xmf4x4World;
					retval = send(client_sock, (char*)&anotherpktdata, sizeof(PKT_CREATE_OBJECT), 0);
					std::cout << "count0 send\n";
				}
				else if (count == 1)
				{
					pktdata.ID = clients[1].id;
					pktdata.WorldMatrix = Objects[1]->m_xmf4x4World;
					pktdata.IsShooting = false;
					retval = send(client_sock, (char*)&pktdata, sizeof(PKT_PLAYER_INFO), 0);

					anotherpktdata.Object_Type = Objects[0]->m_Object_Type;
					anotherpktdata.Object_Index = 0;
					anotherpktdata.WorldMatrix = Objects[0]->m_xmf4x4World;
					retval = send(client_sock, (char*)&anotherpktdata, sizeof(PKT_CREATE_OBJECT), 0);
					std::cout << "count1 send\n";
				}

				Arg* arg = new Arg;
				arg->pthis = this;
				arg->client_socket = client_sock;

				client_Event[count] = CreateEvent(NULL, FALSE, FALSE, NULL);
				thread[count] = CreateThread(
					NULL, 0, client_thread,
					(LPVOID)arg, 0, NULL);
				count++;
			}
		}
		else if(count == 2)
		{
			Update_Arg* arg = new Update_Arg;
			arg->pthis = this;
			arg->pScene = m_pScene;
		
			update_thread = CreateThread(
				NULL, 0, Update,
				(LPVOID)arg, 0, NULL);
			count = -1;
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
	int retval;
	CGameObject** Objects = pScene->GetObjects(OBJECT_TYPE_OBSTACLE);

	while (true)
	{
		WaitForMultipleObjects(2, client_Event, TRUE, INFINITE);
		ResetEvent(Event);
		// 플레이어 정보를 기반으로 패킷 보내기
		while (true)
		{
			PKT_PLAYER_INFO pkt;
			//m.lock();
			if (msg_queue.empty())
				break;
			else
			{
				pkt = msg_queue.front();
				msg_queue.pop();
			}
			//m.unlock();

			pScene->m_pObjects[pkt.ID]->SetWorldTransf(pkt.WorldMatrix);

			PKT_ID pid = PKT_ID_PLAYER_INFO;
			Send_msg((char*)&pid, sizeof(PKT_ID), 0);
			//std::cout << "send pktid\n";
			
			Send_msg((char*)&pkt, sizeof(pkt), 0);
			//std::cout << "send pkt\n";
			

			if (pkt.IsShooting == true)
			{
				pid = PKT_ID_CREATE_OBJECT;
				PKT_CREATE_OBJECT bulletpkt;
				bulletpkt.Object_Type = OBJECT_TYPE_BULLET;
				bulletpkt.WorldMatrix = pScene->m_pObjects[pkt.ID]->GetWorldTransf();
				bulletpkt.Object_Index = pScene->GetIndex();
				Send_msg((char*)&pid, sizeof(PKT_ID), 0);
				Send_msg((char*)&bulletpkt, sizeof(PKT_CREATE_OBJECT), 0);
				std::cout << "index : " << bulletpkt.Object_Index << std::endl;
				CGameObject bullet;
				bullet.m_Object_Type = OBJECT_TYPE_BULLET;
				bullet.SetWorldTransf(pScene->m_pObjects[pkt.ID]->GetWorldTransf());
				bullet.SetMovingSpeed(1000.0f);
				pScene->AddObject(bullet);
			}
		}
		pScene->AnimateObjects(FIXED_FRAME);
		SetEvent(Event);
	}
}

DWORD __stdcall Framework::client_thread(LPVOID arg)
{
	Arg* t_arg = (Arg*)arg;
	std::cout << ((Arg*)arg)->pthis << std::endl;
	return t_arg->pthis->client_process(t_arg->client_socket);
}

DWORD Framework::client_process(SOCKET arg)
{
	SOCKET client_socket = arg;
	int retval;
	SOCKADDR_IN clientaddr;
	int addrlen;
	char id_buf[sizeof(PKT_ID)];

	// 클라이언트 정보 얻기
	addrlen = sizeof(clientaddr);
	getpeername(client_socket, (SOCKADDR*)&clientaddr, &addrlen);

	while (1)
	{
		char data_buf[sizeof(PKT_PLAYER_INFO)];
		retval = recvn(client_socket, data_buf, sizeof(PKT_PLAYER_INFO), 0);
		if (retval == SOCKET_ERROR)
		{
			std::cout << "thread ERROR : ";
			err_display("recv()");
			break;
		}
		//std::cout << retval << "바이트 받음\n";
		PKT_PLAYER_INFO p_info;
		memcpy(&p_info, &data_buf, sizeof(PKT_PLAYER_INFO));
		//std::cout << p_info.ID << " : " << p_info.WorldMatrix._41 << ", " << p_info.WorldMatrix._42 << ", " << p_info.WorldMatrix._43 << std::endl;
		// 받은 데이터 출력
		m.lock();
		msg_queue.push(PKT_PLAYER_INFO{ p_info.ID, p_info.WorldMatrix, p_info.IsShooting });	
		m.unlock();

		SetEvent(client_Event[p_info.ID]);
		WaitForSingleObject(Event, INFINITE);
	}
	return 0;
}