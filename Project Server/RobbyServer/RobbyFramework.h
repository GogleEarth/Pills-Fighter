#pragma once

int recvn(SOCKET s, char * buf, int len, int flags);
void SendAllPlayer(char* buf);
void SendAllPlayer(RoobyPacketType pktType);
void Recv(SOCKET client_sock);
void ProcessPacket(SOCKET client_sock, RoobyPacketType pktType);

class RobbyFramework
{
	std::vector<std::thread> threads;
	SOCKET listen_sock;
	WSADATA wsa;
public:
	RobbyFramework();
	~RobbyFramework();
	int Build();
	void Accept();
	void Release();
};
