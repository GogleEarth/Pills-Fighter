#include "pch.h"
#include "RobbyFramework.h"


RobbyFramework::RobbyFramework()
{
}


RobbyFramework::~RobbyFramework()
{
}

int RobbyFramework::Build()
{
	int retval;

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

	while (true)
	{
		if (clients.size < MAX_USER)
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
					clients.emplace_back(PlayerInfo{ id_buf, client_sock });
				else;
				

			}
		}
	}
}

void RobbyFramework::Send()
{
}

void RobbyFramework::Recv()
{
}

void RobbyFramework::Thread_func()
{
}
