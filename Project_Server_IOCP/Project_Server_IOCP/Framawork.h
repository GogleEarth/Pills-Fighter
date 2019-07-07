#pragma once
#include "Room.h"

enum EVENT_TYPE {
	EVENT_TYPE_SEND,
	EVENT_TYPE_RECV,
	EVENT_TYPE_ROOM_UPDATE,
	EVENT_TYPE_LOAD_ALL
};

#define MAX_BUFFER 1024

struct OVER_EX {
	WSAOVERLAPPED	over;
	WSABUF			dataBuffer;
	char			messageBuffer[MAX_BUFFER];
	EVENT_TYPE		event_t;
};

class Client
{
public:
	bool in_use;
	OVER_EX over_ex;
	SOCKET socket;
	char packet_buffer[MAX_BUFFER];
	int	prev_size;

	Client() {
		in_use = false;
		over_ex.dataBuffer.len = MAX_BUFFER;
		over_ex.dataBuffer.buf = over_ex.messageBuffer;
		over_ex.event_t = EVENT_TYPE_RECV;
	}
};

struct EVENT_ST {
	int obj_id;
	EVENT_TYPE type;
	std::chrono::high_resolution_clock::time_point  start_time;

	constexpr bool operator < (const EVENT_ST& _Left) const
	{	// apply operator< to operands
		return (start_time > _Left.start_time);
	}
};

class Framawork
{
	Room rooms_[10];
	HANDLE iocp_;
	Client clients_[1000];
	std::mutex timer_l;
	std::priority_queue <EVENT_ST> timer_queue;

	CRepository* repository_;
public:
	Framawork();
	~Framawork();

	void init();

	int thread_process();
	int accept_process();
	int timer_process();

	void add_timer(int obj_id, EVENT_TYPE et, std::chrono::high_resolution_clock::time_point start_time);
	void do_recv(int id);
	void error_display(const char *mess, int err_no);
	void disconnect_client(int id);
	int search_client_in_room(SOCKET socket);
	void process_packet(int id, char* packet);
	void send_packet_to_player(int id, char* packet);
	void send_packet_to_all_player(char* packet);
	void send_packet_to_room_player(int room, char* packet);
	int find_empty_room();
};

