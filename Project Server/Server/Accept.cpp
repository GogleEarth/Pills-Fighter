#include "pch.h"
#include "Accept.h"
#include "Framework.h"

WSADATA wsa;
SOCKET listen_sock;
Framework rooms[10];

Accept::Accept()
{
}

Accept::~Accept()
{
}

void Accept::Build()
{
	int retval;

	for (int i = 0; i < 10; ++i)
		rooms[i].Build();

	// 윈속 초기화
	if (WSAStartup(WS22, &wsa) != 0)
		return;

	// socket()
	listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_sock == INVALID_SOCKET) exit(1);

	// bind()
	SOCKADDR_IN serveraddr;
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons(SERVERPORT);
	retval = bind(listen_sock, (SOCKADDR*)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR)
		exit(1);

	// listen()
	retval = listen(listen_sock, SOMAXCONN);
	if (retval == SOCKET_ERROR)
		exit(1);
}

void Accept::Release()
{
	closesocket(listen_sock);

	// 윈속 종료
	WSACleanup();
}

void Accept_Process()
{
	int addrlen;
	int retval;

	while (true)
	{
		SOCKET client_sock;
		SOCKADDR_IN clientaddr;
		// accept()
		addrlen = sizeof(clientaddr);
		client_sock = accept(listen_sock, (SOCKADDR*)&clientaddr, &addrlen);
		if (client_sock == INVALID_SOCKET)
		{
			std::cout << "Accept ERROR : ";
			break;
		}

		/* 여기서입장할 방 번호 받음 */
		//recv()

		if (rooms[0].count < MAX_CLIENT)
		{
			if (rooms[0].count == 0)
			{
				rooms[0].main_loop();
			}
			if (!rooms[0].game_start)
			{
				PKT_CLIENTID pkt_cid;
				pkt_cid.PktId = (char)PKT_ID_PLAYER_ID;
				pkt_cid.PktSize = (char)sizeof(PKT_CLIENTID);
				int id = rooms[0].findindex();
				pkt_cid.Team = id % 2;
				if (id != -1)
					pkt_cid.id = id;
				else
					std::cout << "풀방임.\n";
				if (send(client_sock, (char*)&pkt_cid, pkt_cid.PktSize, 0) == SOCKET_ERROR)
				{
					std::cout << "main_loop ERROR : ";
					while (true);
				}


				rooms[0].m.lock();
				int numclients = rooms[0].searchenables();
				rooms[0].m.unlock();

				if (numclients > 0)
				{
					PKT_PLAYER_IN pkt_pin;
					pkt_pin.PktId = (char)PKT_ID_PLAYER_IN;
					pkt_pin.PktSize = (char)sizeof(PKT_PLAYER_IN);
					for (int i = 0; i < MAX_CLIENT; ++i)
					{
						rooms[0].m.lock();
						bool enable = rooms[0].clients[i].enable;
						char team = rooms[0].clients[i].team;
						rooms[0].m.unlock();
						if (enable)
						{
							pkt_pin.id = i;
							pkt_pin.Team = team;
							std::cout << i << "번쨰 플레이어 정보를 새로 들어온 클라이언트에게 보냄\n";
							retval = send(client_sock, (char*)&pkt_pin, pkt_pin.PktSize, 0);
						}
					}
					rooms[0].m.lock();
					for (auto client : rooms[0].clients)
					{
						if (client.enable)
						{
							pkt_pin.id = id;
							pkt_pin.Team = id % 2;
							std::cout << id << "번쨰 플레이어의 입장 다른 플레이어에게 알려줌\n";
							retval = send(client.socket, (char*)&pkt_pin, pkt_pin.PktSize, 0);
						}
					}
					rooms[0].m.unlock();
				}

				rooms[0].m.lock();
				rooms[0].clients[id].id = id;
				rooms[0].clients[id].enable = true;
				rooms[0].clients[id].socket = client_sock;
				rooms[0].clients[id].selected_robot = ROBOT_TYPE_GM;
				rooms[0].clients[id].team = id % 2;
				rooms[0].m.unlock();

				Arg* arg = new Arg;
				arg->pthis = &rooms[0];
				arg->client_socket = client_sock;
				arg->id = id;

				rooms[0].thread[id] = CreateThread(
					NULL, 0, rooms[0].client_thread,
					(LPVOID)arg, 0, NULL);
				rooms[0].count++;
			}
			else
			{
				closesocket(client_sock);
			}
		}
		else
		{
			closesocket(client_sock);
		}
	}
}
