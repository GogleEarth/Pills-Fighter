#include "pch.h"
#include "RobbyFramework.h"

std::vector<PlayerInfo> clients;
std::vector<RoomInfo> rooms;
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
	char idlen_buf[sizeof(int)];
	char id_buf[MAX_BUFFER];
	int retval;
	int client_num = 0;
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
				retval = recvn(client_sock, idlen_buf, sizeof(int), 0);
				int id_len = (int)idlen_buf;
				retval = recvn(client_sock, id_buf, id_len, 0);
				if (!strcmp(id_buf, "RoomServer"))
				{
					RoobyPacketLogIn new_player;
					if (client_num != 0)
					{
						//접속한 플레이어를 모든 플레이어에게 알려줌
						SendAllPlayer(RoobyPacketTypeLogIn);
						strcpy(new_player.Player_id, id_buf);
						new_player.Player_num = client_num;
						for (auto d : clients)
						{
							send(d.client_sock, (char*)sizeof(new_player), sizeof(int), 0);
						}
						SendAllPlayer((char*)&new_player);
					}
					if (client_num != 0)
					{
						//모든 플레이어를 접속한 플레이어에게 알려줌
						for (auto d : clients)
						{
							send(client_sock,(char*)RoobyPacketTypeLogIn,sizeof(RoobyPacketType),0);
							strcpy(new_player.Player_id, id_buf);
							new_player.Player_num = client_num;
							send(client_sock, (char*)sizeof(new_player), sizeof(int), 0);
							send(client_sock, (char*)&new_player, sizeof(new_player), 0);
						}
					}
					clients.emplace_back(PlayerInfo{ client_num, id_buf, client_sock });
					client_num++;
					threads.emplace_back(std::thread(Recv, client_sock));
					threads.rend()->join();
				}
				else;
			}
		}
	}
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

void SendAllPlayer(char* buf)
{
	for (auto d : clients)
	{
		send(d.client_sock, buf, sizeof(buf), 0);
	}
}

void SendAllPlayer(RoobyPacketType pktType)
{
	for (auto d : clients)
	{
		send(d.client_sock, (char*)pktType, sizeof(RoobyPacketType), 0);
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
		SendAllPlayer((char*)&room);
		for (auto d : clients)
		{
			if (client_sock == d.client_sock)
			{
				d.in_room = true;
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