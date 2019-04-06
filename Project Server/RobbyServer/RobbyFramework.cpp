#include "pch.h"
#include "RobbyFramework.h"

std::vector<PlayerInfo*> clients;
std::vector<RoomInfo> rooms;
std::vector<std::thread> threads;

std::thread th[MAX_USER];
char room_num;
SOCKET room_sock;
std::mutex lock;

RobbyFramework::RobbyFramework()
{
}


RobbyFramework::~RobbyFramework()
{
}

int RobbyFramework::Build()
{
	int retval;
	room_num = 0;

	// 윈속 초기화
	if (WSAStartup(WS22, &wsa) != 0)
		return 1;

	// socket()
	listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_sock == INVALID_SOCKET)
	{
		std::cout << "socket() error\n";
		return 1;
	}
	

	// bind()
	SOCKADDR_IN serveraddr;
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons(SERVERPORT);
	retval = bind(listen_sock, (SOCKADDR*)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR)
	{
		std::cout << "bind() error\n";
		return 1;
	}

	// listen()
	retval = listen(listen_sock, SOMAXCONN);
	if (retval == SOCKET_ERROR)
	{
		std::cout << "listen() error\n";
		return 1;
	}
}

void RobbyFramework::Accept()
{
	int addrlen;
	int idlen;
	int retval;
	int client_num = 0;
	char id[MAX_BUFFER];
	while (true)
	{
		if (clients.size() < MAX_USER)
		{
			SOCKET client_sock;
			SOCKADDR_IN clientaddr;
			// accept()
			addrlen = sizeof(clientaddr);
			client_sock = accept(listen_sock, (SOCKADDR*)&clientaddr, &addrlen);
			if (client_sock == INVALID_SOCKET)
			{
				std::cout << "accept() ERROR : ";
				break;
			}
			else
			{
				retval = recvn(client_sock, (char*)&idlen, sizeof(int), 0);
				std::cout << idlen << "바이트 받아야함\n";
				retval = recvn(client_sock, id, idlen, 0);
				id[idlen] = '\0';
				std::cout << id << "\n";
				if (strcmp(id, "RoomServer") != 0)
				{
					if (client_num != 0)
					{
						//모든 플레이어를 접속한 플레이어에게 알려줌
						for (auto d : clients)
						{
							RoobyPacketType pkttype = RoobyPacketTypeLogIn;
							RoobyPacketLogIn* new_player = new RoobyPacketLogIn;
							strncpy(new_player->Player_id, d->Player_id, strlen(d->Player_id));
							new_player->Player_num = d->Player_num;
							int pktsize = sizeof(&new_player);
							send(client_sock, (char*)&pkttype, sizeof(RoobyPacketType), 0);
							send(client_sock, (char*)&pktsize, sizeof(int), 0);
							std::cout << pktsize << "바이트 보내야함 : " << new_player->Player_id << "\n";
							retval = send(client_sock, (char*)new_player, sizeof(new_player), 0);
							std::cout << retval << "바이트 보냄\n";
							delete new_player;
						}
					}
					if (client_num != 0)
					{
						//접속한 플레이어를 모든 플레이어에게 알려줌5
						RoobyPacketLogIn* new_player = new RoobyPacketLogIn;
						strncpy(new_player->Player_id, id, idlen);
						SendAllPlayer(RoobyPacketTypeLogIn);
						std::cout << "패킷타입 send\n";
						new_player->Player_num = client_num;
						int pktlen = sizeof(&new_player);
						std::cout << pktlen << "바이트 보내야함\n";
						SendAllPlayer((char*)&pktlen, sizeof(int));
						SendAllPlayer((char*)new_player, pktlen);
						delete new_player;
					}
					PlayerInfo* pinfo = new PlayerInfo;
					pinfo->client_sock = client_sock;
					pinfo->Player_num = client_num;
					strncpy(pinfo->Player_id, id, idlen);
					pinfo->Player_id[idlen] = '\0';
					clients.emplace_back(pinfo);
					th[client_num] = std::thread{ Recv,client_sock };
					std::cout << clients[client_num]->Player_id << std::endl;
					std::cout << "클라이언트 입장처리 완료\n";
					client_num++;
				}
				else
				{
					std::cout << "룸 서버 ID임\n";
				}
			}
		}
	}
	for (int i = 0; i < MAX_USER; ++i)
		th[i].join();
}

void RobbyFramework::Release()
{
	threads.clear();
	clients.clear();
	rooms.clear();
	closesocket(listen_sock);
	closesocket(room_sock);
	WSACleanup();
}

void SendAllPlayer(char* buf, int len)
{
	for (auto d : clients)
	{
		send(d->client_sock, buf, len, 0);
	}
}

void SendAllPlayer(RoobyPacketType pktType)
{
	for (auto d : clients)
	{
		send(d->client_sock, (char*)&pktType, sizeof(RoobyPacketType), 0);
	}
}

void Recv(SOCKET client_sock)
{
	int retval;

	RoobyPacketType PktID;

	while (true)
	{
		// 데이터 받기 # 패킷 식별 ID
		retval = recvn(client_sock, (char*)&PktID, sizeof(RoobyPacketType), 0);
		if (retval == SOCKET_ERROR)
		{
			std::cout << "[ERROR] 데이터 받기 # 패킷 식별 ID\n";
			break;
		}
		ProcessPacket(client_sock, PktID);
	}
}

void ProcessPacket(SOCKET client_sock, RoobyPacketType pktType)
{
	int retval;
	int PktSize = 0;
	char* buf;
	RoobyPacketCreateRoom room;
	switch (pktType)
	{
	case RoobyPacketTypeLogIn:
		break;
	case RoobyPacketTypeLogOut:
		break;
	case RoobyPacketTypeCreateRoom:
		lock.lock();
		room.room_num = room_num;
		lock.unlock();
		room.Player_num = 1;
		//send(room_sock, (char*)RoobyPacketTypeCreateRoom, sizeof(RoobyPacketType), 0);
		//send(room_sock, (char*)&room, sizeof(RoobyPacketCreateRoom), 0);
		rooms.emplace_back(RoomInfo{ room_num, 1 });
		SendAllPlayer(RoobyPacketTypeCreateRoom);
		SendAllPlayer((char*)&room, sizeof(room));
		for (auto d : clients)
		{
			if (client_sock == d->client_sock)
			{
				d->in_room = true;
				break;
			}
		}
		lock.lock();
		room_num++;
		lock.unlock();
		break;
	case RoobyPacketTypeDestroyRoom:
		break;
	case RoobyPacketTypeInvitePlayer:
		PktSize = sizeof(RoobyPacketInvitePlayer);
		buf = new char[PktSize];
		retval = recvn(client_sock, buf, PktSize, 0);
		((RoobyPacketInvitePlayer*)buf)->toPlayer_num;
		((RoobyPacketInvitePlayer*)buf)->fromPlayer_num;
		break;
	case RoobyPacketTypeRoomIn:
		break;
	case RoobyPacketTypeRoomOut:
		break;
	case RoobyPacketTypeRoomInfo:
		break;
	default:
		break;
	}
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