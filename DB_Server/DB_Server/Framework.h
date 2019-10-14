#pragma once

#define MAX_BUFFER 255

class Framework
{
	SQLHENV henv_;
	SQLHDBC hdbc_;
	SQLHSTMT hstmt_;

	SOCKET listen_socket_;
	SOCKET main_socket_;
	char recv_buffer_[MAX_BUFFER + 1];
	int prev_size_ = 0;
public:
	Framework();
	~Framework();
	bool init();
	bool accpet_process();
	void process_packet();
	void recvn();
};

