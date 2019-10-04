#pragma once
#include "Room.h"

enum EVENT_TYPE {
	EVENT_TYPE_SEND,
	EVENT_TYPE_RECV,
	EVENT_TYPE_ROOM_UPDATE,
	EVENT_TYPE_LOAD_ALL,
	EVENT_TYPE_OBJECT_MOVE,
	EVENT_TYPE_ITEM,
	EVENT_TYPE_BEAM_RIFLE,
	EVENT_TYPE_GM_GUN,
	EVENT_TYPE_SABER,
	EVENT_TYPE_RESPAWN,
	EVENT_TYPE_BEAM_SNIPER
};

#define MAX_BUFFER 255
#define MAX_USER 1000

struct Overlapped 
{
	WSAOVERLAPPED	overlapped_;
	WSABUF			wsa_buffer_;
	char			packet_buffer_[MAX_BUFFER];
	EVENT_TYPE		event_type_;
	float			elapsed_time_;
	int				room_num_;
};

class Client
{
public:
	bool in_use_;
	Overlapped over_ex;
	SOCKET socket_;
	char buffer_[MAX_BUFFER];
	int	prev_size_;
	bool in_room_;
	wchar_t name_[MAX_NAME_LENGTH];
	Client() {
		socket_ = INVALID_SOCKET;
		in_use_ = false;
		over_ex.wsa_buffer_.len = MAX_BUFFER;
		over_ex.wsa_buffer_.buf = over_ex.packet_buffer_;
		over_ex.event_type_ = EVENT_TYPE_RECV;
		in_room_ = false;
		lstrcpynW(name_, L"¶ó¸¶¹Ù", MAX_NAME_LENGTH);
	}
};

struct Event {
	int object_index_;
	int room_num_;
	EVENT_TYPE type_;
	std::chrono::high_resolution_clock::time_point  start_time_;

	constexpr bool operator < (const Event& right) const
	{	// apply operator< to operands
		return (start_time_ > right.start_time_);
	}
};

class Framawork
{
	Room rooms_[10];
	HANDLE iocp_;
	std::mutex accept_l;
	Client clients_[MAX_USER];
	std::mutex timer_l;
	std::priority_queue <Event> timer_queue;

	Repository* repository_;
public:
	Framawork();
	~Framawork();

	void init();

	int thread_process();
	int accept_process();
	int timer_process();

	void add_event(int obj_id, int room_num, EVENT_TYPE et, std::chrono::high_resolution_clock::time_point start_time);
	void do_recv(int id);
	void error_display(const char *mess, int err_no);
	void disconnect_client(int id);
	int search_client_in_room(SOCKET socket);
	void process_packet(int id, char* packet);
	void send_packet_to_player(int id, char* packet);
	void send_packet_to_all_player(char* packet);
	void send_packet_to_room_player(int room, char* packet);
	void send_packet_to_team_player(int room, char* packet, char team);
	int find_empty_room();
};

