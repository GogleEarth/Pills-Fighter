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

int Find_Room_Num()
{
	int ret = -1;
	for (int i = 0; i < 10; i++)
	{
		if (rooms[i].get_players() > 0) continue;
		ret = i;
		return ret;
	}
	return ret;
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

		int room_num;
		PKT_ID iPktID;
		int nPktSize = 0;
		char* buf;
		retval = recvn(client_sock, (char*)&iPktID, sizeof(PKT_ID), 0);
		if (iPktID == PKT_ID_CREATE_ROOM) nPktSize = sizeof(PKT_CREATE_ROOM); // 플레이어 정보 [ 행렬, 상태 ]
		else if (iPktID == PKT_ID_ROOM_IN) nPktSize = sizeof(PKT_ROOM_IN);
		//recv()
		buf = new char[nPktSize];
		retval = recvn(client_sock, buf, nPktSize, 0);
		if (iPktID == PKT_ID_CREATE_ROOM)
		{
			room_num = Find_Room_Num();
		}
		else if (iPktID == PKT_ID_ROOM_IN)
		{
			room_num = ((PKT_ROOM_IN*)buf)->Room_num;
			if (room_num > 10 || room_num < 0)
			{
				std::cout << "잘못된 방 번호\n";
				closesocket(client_sock);
				continue;
			}
		}
		else
		{
			std::cout << "알수없는 패킷\n";
			closesocket(client_sock);
			continue;
		}



		if (rooms[room_num].get_players() < MAX_CLIENT)
		{
			if (rooms[room_num].get_players() == 0)
			{
				rooms[room_num].main_loop();
			}
			if (!rooms[room_num].game_start)
			{
				PKT_CLIENTID pkt_cid;
				pkt_cid.PktId = (char)PKT_ID_PLAYER_ID;
				pkt_cid.PktSize = (char)sizeof(PKT_CLIENTID);
				int id = rooms[room_num].findindex();
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


				rooms[room_num].m.lock();
				int numclients = rooms[room_num].searchenables();
				rooms[room_num].m.unlock();

				if (numclients > 0)
				{
					PKT_PLAYER_IN pkt_pin;
					pkt_pin.PktId = (char)PKT_ID_PLAYER_IN;
					pkt_pin.PktSize = (char)sizeof(PKT_PLAYER_IN);
					for (int i = 0; i < MAX_CLIENT; ++i)
					{
						rooms[room_num].m.lock();
						bool enable = rooms[room_num].clients[i].enable;
						char team = rooms[room_num].clients[i].team;
						rooms[room_num].m.unlock();
						if (enable)
						{
							pkt_pin.id = i;
							pkt_pin.Team = team;
							std::cout << i << "번쨰 플레이어 정보를 새로 들어온 클라이언트에게 보냄\n";
							retval = send(client_sock, (char*)&pkt_pin, pkt_pin.PktSize, 0);
						}
					}
					rooms[room_num].m.lock();
					for (auto client : rooms[room_num].clients)
					{
						if (client.enable)
						{
							pkt_pin.id = id;
							pkt_pin.Team = id % 2;
							std::cout << id << "번쨰 플레이어의 입장 다른 플레이어에게 알려줌\n";
							retval = send(client.socket, (char*)&pkt_pin, pkt_pin.PktSize, 0);
						}
					}
					rooms[room_num].m.unlock();
				}

				rooms[room_num].m.lock();
				rooms[room_num].clients[id].id = id;
				rooms[room_num].clients[id].enable = true;
				rooms[room_num].clients[id].socket = client_sock;
				rooms[room_num].clients[id].selected_robot = ROBOT_TYPE_GM;
				rooms[room_num].clients[id].team = id % 2;
				rooms[room_num].m.unlock();

				Arg* arg = new Arg;
				arg->pthis = &rooms[room_num];
				arg->client_socket = client_sock;
				arg->id = id;

				rooms[0].thread[id] = CreateThread(
					NULL, 0, rooms[room_num].client_thread,
					(LPVOID)arg, 0, NULL);
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
