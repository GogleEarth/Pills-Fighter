#pragma once
class RobbyFramework
{
	std::vector<std::thread> threads;
	std::vector<PlayerInfo> clients;
	std::vector<RoomInfo> rooms;
	SOCKET listen_sock;
	SOCKET room_sock;
	WSADATA wsa;
public:
	RobbyFramework();
	~RobbyFramework();
	int Build();
	void Accept();
	void Send();
	void Recv();
	void Thread_func();

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

};

