#include "pch.h"
#include "Framawork.h"
#include "Protocol.h"

Framawork::Framawork()
{
}


Framawork::~Framawork()
{
}

void Framawork::init()
{
	iocp_ = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);

	repository_ = new CRepository();

	for (int i = 0; i < 1000; i++)
		clients_[i].in_use = false;

	for (int i = 0; i < 10; ++i)
		rooms_[i].init(repository_);
}

int Framawork::thread_process()
{
	while (true) {
		DWORD io_byte;
		ULONGLONG l_key;
		OVER_EX *over_ex;
		int is_error = GetQueuedCompletionStatus(iocp_, &io_byte,
			&l_key, reinterpret_cast<LPWSAOVERLAPPED *>(&over_ex),
			INFINITE);
		int key = static_cast<int>(l_key);
		if (0 == is_error) {
			int err_no = WSAGetLastError();
			if (64 == err_no) {
				disconnect_client(key);
				std::cout << "�÷��̾� ���� ����\n";
				continue;
			}
			else error_display("GQCS : ", err_no);
		}

		if (0 == io_byte) {
			disconnect_client(key);
			std::cout << "�÷��̾� ���� ����\n";
			continue;
		}

		if (EVENT_TYPE_RECV == over_ex->event_t) {
			std::wcout << "Packet from Client:" << key << std::endl;
			int rest = io_byte;
			char *ptr = over_ex->messageBuffer;
			char packet_size = 0;
			if (0 < clients_[key].prev_size)
				packet_size = clients_[key].packet_buffer[0];
			while (0 < rest) {
				if (0 == packet_size) packet_size = ptr[0];
				int required = packet_size - clients_[key].prev_size;
				if (required <= rest) {
					memcpy(clients_[key].packet_buffer + clients_[key].prev_size,
						ptr, required);
					process_packet(key, clients_[key].packet_buffer);
					rest -= required;
					ptr += required;
					packet_size = 0;
					clients_[key].prev_size = 0;
				}
				else {
					memcpy(clients_[key].packet_buffer + clients_[key].prev_size,
						ptr, rest);
					rest = 0;
					clients_[key].prev_size += rest;
				}
			}
			do_recv(key);
		}
		else if (EVENT_TYPE_SEND == over_ex->event_t) {
			delete over_ex;
		}
		else
		{
			std::cout << "UNKNOWN EVENT\n";
			while (true);
		}
	}
}

int Framawork::accept_process()
{
	WSADATA WSAData;
	if (WSAStartup(MAKEWORD(2, 2), &WSAData) != 0)
	{
		std::cout << "Error - Can not load 'winsock.dll' file\n";
		return 1;
	}

	SOCKET listenSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (listenSocket == INVALID_SOCKET)
	{
		std::cout << "Error - Invalid socket\n";
		return 1;
	}

	SOCKADDR_IN serverAddr;
	memset(&serverAddr, 0, sizeof(SOCKADDR_IN));
	serverAddr.sin_family = PF_INET;
	serverAddr.sin_port = htons(SERVERPORT);
	serverAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);

	if (::bind(listenSocket, (struct sockaddr*)&serverAddr,
		sizeof(SOCKADDR_IN)) == SOCKET_ERROR)
	{
		std::cout << "Error - Fail bind\n";
		closesocket(listenSocket);
		WSACleanup();
		return 1;
	}

	if (listen(listenSocket, 5) == SOCKET_ERROR)
	{
		std::cout << "Error - Fail listen\n";
		closesocket(listenSocket);
		WSACleanup();
		return 1;
	}

	SOCKADDR_IN clientAddr;
	int addrLen = sizeof(SOCKADDR_IN);
	memset(&clientAddr, 0, addrLen);
	SOCKET clientSocket;
	DWORD flags;

	while (1)
	{
		clientSocket = accept(listenSocket, (struct sockaddr *)&clientAddr, &addrLen);
		if (clientSocket == INVALID_SOCKET)
		{
			std::cout << "Error - Accept Failure\n";
			return 1;
		}
#define MAX_USER 1000
		int new_id = -1;
		for (int i = 0; i < MAX_USER; ++i)
			if (false == clients_[i].in_use) {
				new_id = i;
				break;
			}
		if (-1 == new_id) {
			std::cout << "MAX USER overflow\n";
			continue;
		}

		std::cout << "�÷��̾� ����\n";

		clients_[new_id].socket = clientSocket;
		clients_[new_id].prev_size = 0;
		clients_[new_id].in_use = true;
		ZeroMemory(&clients_[new_id].over_ex.over,
			sizeof(clients_[new_id].over_ex.over));
		flags = 0;

		CreateIoCompletionPort(reinterpret_cast<HANDLE>(clientSocket),
			iocp_, new_id, 0);

		for (int i = 0; i < 10; ++i)
		{
			if (rooms_[i].get_is_use())
			{
				PKT_ADD_ROOM pkt_ar;
				pkt_ar.PktId = PKT_ID_ADD_ROOM;
				pkt_ar.PktSize = sizeof(pkt_ar);
				pkt_ar.Room_num = i;
				send_packet_to_player(new_id, (char*)&pkt_ar);
			}
		}

		do_recv(new_id);
	}

	closesocket(listenSocket);

	WSACleanup();

	return 0;
}

int Framawork::timer_process()
{
	while (true) {
		using namespace std;
		using namespace chrono;
		std::this_thread::sleep_for(10ms);
		while (true) {
			timer_l.lock();
			if (true == timer_queue.empty()) {
				timer_l.unlock();
				break;
			}
			EVENT_ST ev = timer_queue.top();

			if (ev.start_time > std::chrono::high_resolution_clock::now()) {
				timer_l.unlock();
				break;
			}
			timer_queue.pop();
			timer_l.unlock();
			OVER_EX *over_ex = new OVER_EX;
			over_ex->event_t = ev.type;
			PostQueuedCompletionStatus(iocp_, 1, ev.obj_id, &over_ex->over);
		}
	}
	return 0;
}

void Framawork::add_timer(int obj_id, EVENT_TYPE et, std::chrono::high_resolution_clock::time_point start_time)
{
	timer_l.lock();
	timer_queue.emplace(EVENT_ST{ obj_id, et, start_time });
	timer_l.unlock();
}

void Framawork::do_recv(int id)
{
	DWORD flags = 0;

	if (WSARecv(clients_[id].socket, &clients_[id].over_ex.dataBuffer, 1,
		NULL, &flags, &(clients_[id].over_ex.over), 0))
	{
		if (WSAGetLastError() != WSA_IO_PENDING)
		{
			std::cout << "Error - IO pending Failure\n";
			while (true);
		}
	}
}

void Framawork::error_display(const char *mess, int err_no)
{
	WCHAR *lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, err_no,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	std::cout << mess;
	std::wcout << L"���� [" << err_no << L"]  " << lpMsgBuf << "\n";
	LocalFree(lpMsgBuf);
}

void Framawork::disconnect_client(int id)
{
	int room_num = search_client_in_room(clients_[id].socket);

	if (room_num != -1)
	{
		rooms_[room_num].disconnect_client(clients_[id].socket);
		PKT_PLAYER_OUT packet;
		packet.id = rooms_[room_num].find_player_by_socket(clients_[id].socket);
		packet.PktId = PKT_ID_PLAYER_OUT;
		packet.PktSize = sizeof(PKT_PLAYER_OUT);
		send_packet_to_room_player(room_num, (char*)&packet);
	}

	closesocket(clients_[id].socket);
	clients_[id].in_use = false;
}

int Framawork::search_client_in_room(SOCKET socket)
{
	int room_num = -1;
	for (int i = 0; i < 10; ++i)
	{
		if (!rooms_[i].get_is_use()) continue;
		if (!rooms_[i].search_client(socket)) continue;
		room_num = i;
		break;
	}
	return room_num;
}

/*
PKT_ID_PLAYER_INFO,
PKT_ID_PLAYER_LIFE,
PKT_ID_CREATE_OBJECT,
PKT_ID_DELETE_OBJECT,
PKT_ID_TIME_INFO,
PKT_ID_SEND_COMPLETE,
PKT_ID_UPDATE_OBJECT,
PKT_ID_CREATE_EFFECT,
PKT_ID_GAME_STATE,
PKT_ID_PLAYER_IN,
PKT_ID_PLAYER_OUT,
PKT_ID_LOBBY_PLAYER_INFO,
PKT_ID_LOAD_COMPLETE,
PKT_ID_LOAD_COMPLETE_ALL,
PKT_ID_PLAYER_ID,
PKT_ID_GAME_START,
PKT_ID_SHOOT,
PKT_ID_SCORE,
PKT_ID_GAME_END,
PKT_ID_PICK_ITEM,
PKT_ID_CREATE_ROOM,
PKT_ID_ROOM_IN
*/
void Framawork::process_packet(int id, char* packet)
{
	switch (packet[1])
	{
	case PKT_ID_PLAYER_INFO:
	{
		//rooms_[reinterpret_cast<PKT_PLAYER_INFO*>(packet)->RoomNum].player_info_inqueue(packet);
		break;
	}
	case PKT_ID_LOAD_COMPLETE:
	{
		//rooms_[reinterpret_cast<PKT_LOAD_COMPLETE*>(packet)->RoomNum].player_load_complete(id);
		//if (rooms_[reinterpret_cast<PKT_SEND_COMPLETE*>(packet)->RoomNum].all_load_complete())
		//	add_timer(reinterpret_cast<PKT_SEND_COMPLETE*>(packet)->RoomNum, EVENT_TYPE_LOAD_ALL, std::chrono::high_resolution_clock::now());
		break;
	}
	case PKT_ID_SEND_COMPLETE:
	{
		//rooms_[reinterpret_cast<PKT_SEND_COMPLETE*>(packet)->RoomNum].player_send_complete(id);
		//if (rooms_[reinterpret_cast<PKT_SEND_COMPLETE*>(packet)->RoomNum].all_send_complete())
		//	add_timer(reinterpret_cast<PKT_SEND_COMPLETE*>(packet)->RoomNum, EVENT_TYPE_ROOM_UPDATE, std::chrono::high_resolution_clock::now());
		break;
	}
	case PKT_ID_CREATE_ROOM:
	{
		int room_num = find_empty_room();
		if (room_num != -1)
		{
			PKT_CLIENTID pkt_cid;
			pkt_cid.PktId = (char)PKT_ID_PLAYER_ID;
			pkt_cid.PktSize = (char)sizeof(PKT_CLIENTID);
			int player_id = rooms_[room_num].findindex();
			pkt_cid.id = player_id;
			pkt_cid.Team = player_id % 2;
			
			PKT_CREATE_ROOM_OK pkt_cro;
			pkt_cro.PktId = PKT_ID_CREATE_ROOM_OK;
			pkt_cro.PktSize = sizeof(PKT_CREATE_ROOM_OK);

			send_packet_to_player(id, (char*)&pkt_cro);
			send_packet_to_player(id, (char*)&pkt_cid);
			rooms_[room_num].set_is_use(true);
			rooms_[room_num].add_player(id, clients_[id].socket);

			PKT_ADD_ROOM pkt_ar;
			pkt_ar.PktId = PKT_ID_ADD_ROOM;
			pkt_ar.PktSize = sizeof(pkt_ar);
			pkt_ar.Room_num = room_num;
			send_packet_to_all_player((char*)&pkt_ar);
		}
		else
			std::cout << "���̻� ���� ������ �� ����\n";
		break;
	}
	case PKT_ID_ROOM_IN:
	{
		int room_num = reinterpret_cast<PKT_ROOM_IN*>(packet)->Room_num;
		if (rooms_[room_num].get_num_player_in_room() < MAX_CLIENT)
		{
			PKT_ROOM_IN_OK pkt_rio;
			pkt_rio.PktId = PKT_ID_ROOM_IN_OK;
			pkt_rio.PktSize = sizeof(PKT_ROOM_IN_OK);

			PKT_CLIENTID pkt_cid;
			pkt_cid.PktId = (char)PKT_ID_PLAYER_ID;
			pkt_cid.PktSize = (char)sizeof(PKT_CLIENTID);
			int player_id = rooms_[room_num].findindex();
			pkt_cid.id = player_id;
			pkt_cid.Team = player_id % 2;

			send_packet_to_player(id, (char*)&pkt_rio);

			PKT_PLAYER_IN pkt_pin;
			pkt_pin.PktId = (char)PKT_ID_PLAYER_IN;
			pkt_pin.PktSize = (char)sizeof(PKT_PLAYER_IN);
			for (int i = 0; i < MAX_CLIENT; ++i)
			{
				auto player = rooms_[room_num].get_player(i);
				if (player->get_use())
				{
					pkt_pin.id = i;
					pkt_pin.Team = player->get_team();
					send_packet_to_player(id, (char*)&pkt_pin);
				}
			}

			pkt_pin.id = player_id;
			pkt_pin.Team = player_id % 2;
			send_packet_to_room_player(room_num, (char*)&pkt_pin);

			send_packet_to_player(id, (char*)&pkt_cid);
			rooms_[room_num].add_player(id, clients_[id].socket);
		}
		else
			std::cout << reinterpret_cast<PKT_ROOM_IN*>(packet)->Room_num << "�� �濡 ���̻� ���� �Ұ�\n";
		break;
	}
	case PKT_ID_SHOOT:
	{
		//rooms_[reinterpret_cast<PKT_SEND_COMPLETE*>(packet)->RoomNum].shoot(id);
		break;
	}
	case PKT_ID_LOBBY_PLAYER_INFO:
	{
		int room_num = search_client_in_room(clients_[id].socket);
		rooms_[room_num].set_player_lobby_info(reinterpret_cast<PKT_LOBBY_PLAYER_INFO*>(packet)->id, reinterpret_cast<PKT_LOBBY_PLAYER_INFO*>(packet)->selected_robot,
				reinterpret_cast<PKT_LOBBY_PLAYER_INFO*>(packet)->Team);
		
		send_packet_to_room_player(room_num, packet);
		break;
	}
	case PKT_ID_GAME_START:
	{
		std::cout << "gamestartpacket\n";
		break;
	}
	default:
		std::wcout << L"���ǵ��� ���� ��Ŷ ���� ����!!\n";
		while (true);
	}
}

void Framawork::send_packet_to_player(int id, char* packet)
{
	char *p = reinterpret_cast<char *>(packet);
	OVER_EX *ov = new OVER_EX;
	ov->dataBuffer.len = p[0];
	ov->dataBuffer.buf = ov->messageBuffer;
	ov->event_t = EVENT_TYPE_SEND;
	memcpy(ov->messageBuffer, p, p[0]);
	ZeroMemory(&ov->over, sizeof(ov->over));
	int error = WSASend(clients_[id].socket, &ov->dataBuffer, 1, 0, 0,
		&ov->over, NULL);
	if (0 != error) {
		int err_no = WSAGetLastError();
		if (err_no != WSA_IO_PENDING)
		{
			std::cout << "Error - IO pending Failure\n";
			error_display("WSASend in send_packet()  ", err_no);
			while (true);
		}
	}
}

void Framawork::send_packet_to_all_player(char* packet)
{
	for (int i = 0; i < 1000; ++i)
	{
		if (!clients_[i].in_use) continue;
		send_packet_to_player(i, packet);
	}
}

void Framawork::send_packet_to_room_player(int room, char *packet)
{
	for (int i = 0; i < 8; i++)
	{
		Player* player = rooms_[room].get_player(i);
		if (!player->get_use()) continue;
		send_packet_to_player(player->get_serverid(), packet);
	}
}

int Framawork::find_empty_room()
{
	int room_num = -1;
	for (int i = 0; i < 10; ++i)
	{
		if (rooms_[i].get_is_use()) continue;
		room_num = i;
		break;
	}
	return room_num;
}
