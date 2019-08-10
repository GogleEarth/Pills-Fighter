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
				clients_[key].in_room = false;
				lstrcpynW(clients_[key].name, L"라마바", MAX_NAME_LENGTH);
				std::cout << "플레이어 접속 종료\n";
				continue;
			}
			else error_display("GQCS : ", err_no);
		}

		if (0 == io_byte) {
			disconnect_client(key);
			clients_[key].in_room = false;
			lstrcpynW(clients_[key].name, L"라마바", MAX_NAME_LENGTH);
			std::cout << "플레이어 접속 종료\n";
			continue;
		}

		if (EVENT_TYPE_RECV == over_ex->event_t) {
			//std::wcout << "Packet from Client:" << key << std::endl;
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
		else if (EVENT_TYPE_SEND == over_ex->event_t) 
		{
			delete over_ex;
		}
		else if (EVENT_TYPE_ROOM_UPDATE == over_ex->event_t)
		{
			using namespace std;
			using namespace chrono;
			float elapsed_time;

			if (over_ex->elapsed_time <= 0.001f)
				elapsed_time = 0.016f;
			else
				elapsed_time = over_ex->elapsed_time;

			PKT_TIME_INFO pkt_ti;
			pkt_ti.PktId = PKT_ID_TIME_INFO;
			pkt_ti.PktSize = sizeof(PKT_TIME_INFO);
			pkt_ti.elapsedtime = elapsed_time;
			send_packet_to_room_player(key, (char*)&pkt_ti);

			if (!rooms_[key].get_game_end())
			{
				rooms_[key].room_update(elapsed_time);

				while (true)
				{
					auto data = rooms_[key].create_object_dequeue();
					if (data == nullptr) break;
					if (data->Object_Type == OBJECT_TYPE_METEOR)
					{
						data->Object_Index = rooms_[key].add_object(data->Object_Type, data->WorldMatrix);
						add_timer(data->Object_Index, key, EVENT_TYPE_OBJECT_MOVE, high_resolution_clock::now() + 16ms);
					}
					else if (data->Object_Type == OBJECT_TYPE_ITEM_AMMO || data->Object_Type == OBJECT_TYPE_ITEM_HEALING)
					{
						add_timer(data->Object_Index, key, EVENT_TYPE_ITEM, high_resolution_clock::now() + 16ms);
					}
					send_packet_to_room_player(key, (char*)data);
					delete data;
				}

				while (true)
				{
					auto data = rooms_[key].map_event_dequeue();
					if (data == nullptr) break;
					if (data->type == MAP_EVENT_TYPE_ALERT)
						std::cout << key << "번방 이벤트 경고\n";
					if (data->type == MAP_EVENT_TYPE_START)
						std::cout << key << "번방 이벤트 시작\n";
					if (data->type == MAP_EVENT_TYPE_END)
						std::cout << key << "번방 이벤트 끝\n";
					send_packet_to_room_player(key, (char*)data);
					delete data;
				}

				while (true)
				{
					auto data = rooms_[key].player_life_dequeue();
					if (data == nullptr) break;
					send_packet_to_team_player(key, (char*)data, rooms_[key].get_player_team(data->ID));
					delete data;
				}

				while (true)
				{
					auto data = rooms_[key].player_die_dequeue();
					if (data == nullptr) break;
					send_packet_to_room_player(key, (char*)data);
					using namespace std;
					using namespace chrono;
					add_timer(data->id, over_ex->room_num, EVENT_TYPE_RESPAWN, high_resolution_clock::now() + 16ms);
					delete data;
				}

				while (true)
				{
					auto data = rooms_[key].score_dequeue();
					if (data == nullptr) break;
					send_packet_to_room_player(key, (char*)data);

					if (rooms_[key].get_blue_score() <= 0 || rooms_[key].get_red_score() <= 0)
					{
						PKT_GAME_END pkt_ge;
						pkt_ge.PktId = PKT_ID_GAME_END;
						pkt_ge.PktSize = sizeof(PKT_GAME_END);
						if (rooms_[key].get_blue_score() <= 0)
							pkt_ge.WinTeam = 0;
						else
							pkt_ge.WinTeam = 1;

						rooms_[key].set_game_end(true);

						send_packet_to_room_player(key, (char*)&pkt_ge);
					}

					delete data;
				}

				while (true)
				{
					auto data = rooms_[key].create_effect_dequeue();
					if (data == nullptr) break;
					if (data->efType == EFFECT_TYPE_BEAM_HIT)
						send_packet_to_room_player(key, (char*)data);
					else
						send_packet_to_player(rooms_[key].get_player(data->id)->get_serverid(), (char*)data);
					delete data;
				}

				while (true)
				{
					auto data = rooms_[key].item_dequeue();
					if (data == nullptr) break;
					if (data->Item_type == ITEM_TYPE_AMMO1 || data->Item_type == ITEM_TYPE_AMMO2)
						data->Item_type = ITEM_TYPE_AMMO;
					send_packet_to_team_player(key, (char*)data, rooms_[key].get_player_team(data->ID));
					delete data;
				}
			}
			
			if (rooms_[key].get_num_player_in_room() > 0)
				add_timer(key, key, EVENT_TYPE_ROOM_UPDATE, high_resolution_clock::now() + 16ms);
			else
				rooms_[key].init();

			delete over_ex;
		}
		else if (EVENT_TYPE_OBJECT_MOVE == over_ex->event_t)
		{
			if (rooms_[over_ex->room_num].get_playing())
			{
				auto object = rooms_[over_ex->room_num].get_object(key);
				float elapsed_time;

				if (over_ex->elapsed_time <= 0.001f)
					elapsed_time = 0.016f;
				else
					elapsed_time = over_ex->elapsed_time;

				object->Animate(elapsed_time, rooms_[over_ex->room_num].get_map());
				rooms_[over_ex->room_num].check_collision_obstacles(key);
				rooms_[over_ex->room_num].check_collision_player(key);

				if (object->IsDelete())
				{
					PKT_DELETE_OBJECT pkt_do;
					pkt_do.PktId = PKT_ID_DELETE_OBJECT;
					pkt_do.PktSize = sizeof(PKT_DELETE_OBJECT);
					pkt_do.Object_Index = key;
					send_packet_to_room_player(over_ex->room_num, (char*)&pkt_do);

					OBJECT_TYPE type = object->GetObjectType();
					if (type == OBJECT_TYPE_MACHINE_BULLET
						|| type == OBJECT_TYPE_BZK_BULLET
						|| type == OBJECT_TYPE_BEAM_BULLET
						|| type == OBJECT_TYPE_METEOR)
					{
						PKT_CREATE_EFFECT pktCE;
						pktCE.PktId = PKT_ID_CREATE_EFFECT;
						pktCE.PktSize = (char)sizeof(PKT_CREATE_EFFECT);
						if (type == OBJECT_TYPE_BZK_BULLET
							|| type == OBJECT_TYPE_METEOR)
							pktCE.efType = EFFECT_TYPE_EXPLOSION;
						else
							pktCE.efType = EFFECT_TYPE_HIT;
						pktCE.EftAnitType = EFFECT_ANIMATION_TYPE_ONE;
						pktCE.xmf3Position = object->GetPosition();
						send_packet_to_room_player(over_ex->room_num, (char*)&pktCE);
					}
					object->SetUse(false);
				}
				else
				{
					PKT_UPDATE_OBJECT pkt_uo;
					pkt_uo.PktId = PKT_ID_UPDATE_OBJECT;
					pkt_uo.PktSize = sizeof(PKT_UPDATE_OBJECT);
					pkt_uo.Object_Index = key;
					pkt_uo.Object_Position = object->GetPosition();

					send_packet_to_room_player(over_ex->room_num, (char*)&pkt_uo);

					using namespace std;
					using namespace chrono;
					add_timer(key, over_ex->room_num, EVENT_TYPE_OBJECT_MOVE, high_resolution_clock::now() + 16ms);
				}
			}
			delete over_ex;
		}
		else if (EVENT_TYPE_ITEM == over_ex->event_t)
		{
			if(rooms_[over_ex->room_num].get_playing())
			{
				auto object = rooms_[over_ex->room_num].get_object(key);
				float elapsed_time;

				if (over_ex->elapsed_time <= 0.001f)
					elapsed_time = 0.016f;
				else
					elapsed_time = over_ex->elapsed_time;

				object->Animate(elapsed_time, rooms_[over_ex->room_num].get_map());
				rooms_[over_ex->room_num].check_collision_player(key);

				if (object->IsDelete())
				{
					PKT_DELETE_OBJECT pkt_do;
					pkt_do.PktId = PKT_ID_DELETE_OBJECT;
					pkt_do.PktSize = sizeof(PKT_DELETE_OBJECT);
					pkt_do.Object_Index = key;
					send_packet_to_room_player(over_ex->room_num, (char*)&pkt_do);

					object->SetUse(false);
				}
				else
				{
					using namespace std;
					using namespace chrono;
					add_timer(key, over_ex->room_num, EVENT_TYPE_ITEM, high_resolution_clock::now() + 16ms);
				}
			}
			delete over_ex;
		}
		else if (EVENT_TYPE_BEAM_RIFLE == over_ex->event_t)
		{
			if (rooms_[over_ex->room_num].get_playing())
			{
				auto object = rooms_[over_ex->room_num].get_object(key);
				float elapsed_time;

				if (over_ex->elapsed_time <= 0.001f)
					elapsed_time = 0.016f;
				else
					elapsed_time = over_ex->elapsed_time;

				object->Animate(elapsed_time, rooms_[over_ex->room_num].get_map());

				float distance = 1000.0f;
				rooms_[over_ex->room_num].check_collision_player_to_vector(key, 1000.0f, &distance);
				
				PKT_CREATE_EFFECT pkt_ce;
				pkt_ce.PktId = PKT_ID_CREATE_EFFECT;
				pkt_ce.PktSize = sizeof(PKT_CREATE_EFFECT);
				pkt_ce.EftAnitType = EFFECT_ANIMATION_TYPE_ONE;
				pkt_ce.efType = EFFECT_TYPE_BEAM_RIFLE;
				pkt_ce.xmf3Look = object->GetLook();
				pkt_ce.xmf3Position = object->GetPosition();
				pkt_ce.fDistance = distance;

				send_packet_to_room_player(over_ex->room_num, (char*)&pkt_ce);

				if (object->IsDelete())
				{
					object->SetUse(false);
				}
				else
				{
					using namespace std;
					using namespace chrono;
					add_timer(key, over_ex->room_num, EVENT_TYPE_BEAM_RIFLE, high_resolution_clock::now() + 16ms);
				}
			}
			delete over_ex;
		}
		else if (EVENT_TYPE_GM_GUN == over_ex->event_t)
		{
			if (rooms_[over_ex->room_num].get_playing())
			{
				auto object = rooms_[over_ex->room_num].get_object(key);
				float elapsed_time;

				if (over_ex->elapsed_time <= 0.001f)
					elapsed_time = 0.016f;
				else
					elapsed_time = over_ex->elapsed_time;

				object->Animate(elapsed_time, rooms_[over_ex->room_num].get_map());

				float distance = 600.0f;
				rooms_[over_ex->room_num].check_collision_player_to_vector(key, 600.0f, &distance);

				PKT_CREATE_EFFECT pkt_ce;
				pkt_ce.PktId = PKT_ID_CREATE_EFFECT;
				pkt_ce.PktSize = sizeof(PKT_CREATE_EFFECT);
				pkt_ce.EftAnitType = EFFECT_ANIMATION_TYPE_ONE;
				pkt_ce.efType = EFFECT_TYPE_GM_GUN;
				pkt_ce.xmf3Look = object->GetLook();
				pkt_ce.xmf3Position = object->GetPosition();
				pkt_ce.fDistance = distance;

				send_packet_to_room_player(over_ex->room_num, (char*)&pkt_ce);

				if (object->IsDelete())
				{
					object->SetUse(false);
				}
				else
				{
					using namespace std;
					using namespace chrono;
					add_timer(key, over_ex->room_num, EVENT_TYPE_GM_GUN, high_resolution_clock::now() + 16ms);
				}
			}
			delete over_ex;
		}
		else if (EVENT_TYPE_SABER == over_ex->event_t)
		{
			if (rooms_[over_ex->room_num].get_playing())
			{
				auto object = rooms_[over_ex->room_num].get_object(key);
				float elapsed_time;

				if (over_ex->elapsed_time <= 0.001f)
					elapsed_time = 0.016f;
				else
					elapsed_time = over_ex->elapsed_time;

				object->Animate(elapsed_time, rooms_[over_ex->room_num].get_map());
				rooms_[over_ex->room_num].check_saber_collision_player(key);

				if (object->IsDelete())
				{
					object->SetUse(false);
				}
				else
				{
					using namespace std;
					using namespace chrono;
					add_timer(key, over_ex->room_num, EVENT_TYPE_SABER, high_resolution_clock::now() + 16ms);
				}
			}
			delete over_ex;
		}
		else if (EVENT_TYPE_RESPAWN == over_ex->event_t)
		{
			if (rooms_[over_ex->room_num].get_playing())
			{
				auto object = rooms_[over_ex->room_num].get_object(key);

				if (!object->get_is_die())
				{
					PKT_PLAYER_RESPAWN pkt_rp;
					pkt_rp.PktId = PKT_ID_PLAYER_RESPAWN;
					pkt_rp.PktSize = sizeof(PKT_PLAYER_RESPAWN);
					pkt_rp.id = key;
					pkt_rp.hp = object->GetMaxHitPoint();
					pkt_rp.point = rooms_[over_ex->room_num].get_respawn_point(key);
					pkt_rp.team = object->get_team();

					send_packet_to_room_player(over_ex->room_num, (char*)&pkt_rp);
				}
				else
				{
					using namespace std;
					using namespace chrono;
					add_timer(key, over_ex->room_num, EVENT_TYPE_RESPAWN, high_resolution_clock::now() + 16ms);
				}
			}
			delete over_ex;
		}
		else if (EVENT_TYPE_BEAM_SNIPER == over_ex->event_t)
		{
		if (rooms_[over_ex->room_num].get_playing())
		{
			auto object = rooms_[over_ex->room_num].get_object(key);
			float elapsed_time;

			if (over_ex->elapsed_time <= 0.001f)
				elapsed_time = 0.016f;
			else
				elapsed_time = over_ex->elapsed_time;

			object->Animate(elapsed_time, rooms_[over_ex->room_num].get_map());

			float distance = 2000.0f;
			rooms_[over_ex->room_num].check_collision_player_to_vector(key, 2000.0f, &distance);
			
			PKT_CREATE_EFFECT pkt_ce;
			pkt_ce.PktId = PKT_ID_CREATE_EFFECT;
			pkt_ce.PktSize = sizeof(PKT_CREATE_EFFECT);
			pkt_ce.EftAnitType = EFFECT_ANIMATION_TYPE_ONE;
			pkt_ce.efType = EFFECT_TYPE_BEAM_SNIPER;
			pkt_ce.xmf3Look = object->GetLook();
			pkt_ce.xmf3Position = object->GetPosition();
			pkt_ce.fDistance = distance;

			send_packet_to_room_player(over_ex->room_num, (char*)&pkt_ce);

			if (object->IsDelete())
			{
				object->SetUse(false);
			}
			else
			{
				using namespace std;
				using namespace chrono;
				add_timer(key, over_ex->room_num, EVENT_TYPE_BEAM_SNIPER, high_resolution_clock::now() + 16ms);
			}
		}
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

		accept_l.lock();
		int new_id = -1;
		for (int i = 0; i < MAX_USER; ++i)
			if (false == clients_[i].in_use) {
				new_id = i;
				break;
			}
		accept_l.unlock();

		if (-1 == new_id) {
			std::cout << "MAX USER overflow\n";
			continue;
		}

		std::cout << "플레이어 접속\n";

		clients_[new_id].socket = clientSocket;
		clients_[new_id].prev_size = 0;
		clients_[new_id].in_use = true;
		ZeroMemory(&clients_[new_id].over_ex.over,
			sizeof(clients_[new_id].over_ex.over));
		flags = 0;

		CreateIoCompletionPort(reinterpret_cast<HANDLE>(clientSocket),
			iocp_, new_id, 0);

		PKT_CHANGE_NAME pkt_cn;
		pkt_cn.PktId = PKT_ID_CHANGE_NAME;
		pkt_cn.PktSize = sizeof(PKT_CHANGE_NAME);
		lstrcpynW(pkt_cn.name, clients_[new_id].name, MAX_NAME_LENGTH);

		send_packet_to_player(new_id, (char*)&pkt_cn);

		for (int i = 0; i < 10; ++i)
		{
			if (rooms_[i].get_is_use())
			{
				PKT_ADD_ROOM pkt_ar;
				pkt_ar.PktId = PKT_ID_ADD_ROOM;
				pkt_ar.PktSize = sizeof(pkt_ar);
				pkt_ar.Room_num = i;
				lstrcpynW(pkt_ar.name, rooms_[i].get_name(), MAX_ROOM_NAME_LENGTH);
				send_packet_to_player(new_id, (char*)&pkt_ar);

				PKT_CHANGE_ROOM_INFO pkt_cmi;
				pkt_cmi.PktSize = sizeof(PKT_CHANGE_ROOM_INFO);
				pkt_cmi.PktId = PKT_ID_CHANGE_ROOM_INFO;
				pkt_cmi.Room_num = i;
				pkt_cmi.numpeople = rooms_[i].get_num_player_in_room();
				pkt_cmi.map = rooms_[i].get_map();
				send_packet_to_player(new_id, (char*)&pkt_cmi);
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
		std::this_thread::sleep_for(16ms);
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
			over_ex->elapsed_time = (float)(duration_cast<milliseconds>(std::chrono::high_resolution_clock::now() - ev.start_time).count()) / 1000.0f;
			over_ex->room_num = ev.room_num;
			PostQueuedCompletionStatus(iocp_, 1, ev.obj_id, &over_ex->over);
		}
	}
	return 0;
}

void Framawork::add_timer(int obj_id, int room_num, EVENT_TYPE et, std::chrono::high_resolution_clock::time_point start_time)
{
	timer_l.lock();
	timer_queue.emplace(EVENT_ST{ obj_id, room_num, et, start_time });
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
	std::wcout << L"에러 [" << err_no << L"]  " << lpMsgBuf << "\n";
	LocalFree(lpMsgBuf);
}

void Framawork::disconnect_client(int id)
{
	int room_num = search_client_in_room(clients_[id].socket);

	if (room_num != -1)
	{
		PKT_PLAYER_OUT packet;
		packet.id = rooms_[room_num].find_player_by_socket(clients_[id].socket);
		packet.PktId = PKT_ID_PLAYER_OUT;
		packet.PktSize = sizeof(PKT_PLAYER_OUT);
		send_packet_to_room_player(room_num, (char*)&packet);
		rooms_[room_num].disconnect_client(clients_[id].socket);

		if (rooms_[room_num].get_num_player_in_room() <= 0)
		{
			PKT_ROOM_DELETE pkt_rd;
			pkt_rd.PktId = PKT_ID_DELETE_ROOM;
			pkt_rd.PktSize = sizeof(PKT_ROOM_DELETE);
			pkt_rd.Room_num = room_num;
			send_packet_to_all_player((char*)&pkt_rd);
			rooms_[room_num].set_is_use(false);
		}
		else
		{
			PKT_CHANGE_ROOM_INFO pkt_cri;
			pkt_cri.PktId = PKT_ID_CHANGE_ROOM_INFO;
			pkt_cri.PktSize = sizeof(PKT_CHANGE_ROOM_INFO);
			pkt_cri.Room_num = room_num;
			pkt_cri.numpeople = rooms_[room_num].get_num_player_in_room();
			pkt_cri.map = rooms_[room_num].get_map();
			send_packet_to_all_player((char*)&pkt_cri);
		}
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

void Framawork::process_packet(int id, char* packet)
{
	switch (packet[1])
	{
	case PKT_ID_PLAYER_INFO:
	{
		int room_num = search_client_in_room(clients_[id].socket);
		if (room_num != -1)
		{
			int player = rooms_[room_num].find_player_by_socket(clients_[id].socket);
			if (rooms_[room_num].get_playing())
			{
				rooms_[room_num].set_player_worldmatrix(player, ((PKT_PLAYER_INFO*)packet)->WorldMatrix);
				if (((PKT_PLAYER_INFO*)packet)->Player_Up_Animation == ANIMATION_TYPE_BEAM_SABER_1_ONE ||
					((PKT_PLAYER_INFO*)packet)->Player_Up_Animation == ANIMATION_TYPE_BEAM_SABER_2_ONE ||
					((PKT_PLAYER_INFO*)packet)->Player_Up_Animation == ANIMATION_TYPE_BEAM_SABER_3_ONE)
				{
					if (((PKT_PLAYER_INFO*)packet)->Player_Up_Animation == ANIMATION_TYPE_BEAM_SABER_1_ONE)
						if (((PKT_PLAYER_INFO*)packet)->UpAnimationPosition > 0.33f && ((PKT_PLAYER_INFO*)packet)->UpAnimationPosition < 0.34f)
						{
							//std::cout << ((PKT_PLAYER_INFO*)packet)->UpAnimationPosition << "\n";
							//std::cout << "1번 판정 생성\n";
							int Index = rooms_[room_num].add_object(OBJECT_TYPE_SABER, ((PKT_PLAYER_INFO*)packet)->WorldMatrix, player);
							using namespace std;
							using namespace chrono;
							add_timer(Index, room_num, EVENT_TYPE_SABER, high_resolution_clock::now() + 16ms);
						}
					if (((PKT_PLAYER_INFO*)packet)->Player_Up_Animation == ANIMATION_TYPE_BEAM_SABER_2_ONE)
						if (((PKT_PLAYER_INFO*)packet)->UpAnimationPosition > 0.33f && ((PKT_PLAYER_INFO*)packet)->UpAnimationPosition < 0.34f)
						{
							//std::cout << ((PKT_PLAYER_INFO*)packet)->UpAnimationPosition << "\n";
							//std::cout << "2번 판정 생성\n";
							int Index = rooms_[room_num].add_object(OBJECT_TYPE_SABER, ((PKT_PLAYER_INFO*)packet)->WorldMatrix, player);
							using namespace std;
							using namespace chrono;
							add_timer(Index, room_num, EVENT_TYPE_SABER, high_resolution_clock::now() + 16ms);
						}
					if (((PKT_PLAYER_INFO*)packet)->Player_Up_Animation == ANIMATION_TYPE_BEAM_SABER_3_ONE)
						if (((PKT_PLAYER_INFO*)packet)->UpAnimationPosition > 0.51f && ((PKT_PLAYER_INFO*)packet)->UpAnimationPosition < 0.52f)
						{
							//std::cout << ((PKT_PLAYER_INFO*)packet)->UpAnimationPosition << "\n";
							//std::cout << "3번 판정 생성\n";
							int Index = rooms_[room_num].add_object(OBJECT_TYPE_SABER, ((PKT_PLAYER_INFO*)packet)->WorldMatrix, player);
							using namespace std;
							using namespace chrono;
							add_timer(Index, room_num, EVENT_TYPE_SABER, high_resolution_clock::now() + 16ms);
						}
				}
				send_packet_to_room_player(room_num, packet);
			}
		}
		break;
	}
	case PKT_ID_LOAD_COMPLETE:
	{
		int room_num = search_client_in_room(clients_[id].socket);
		if (room_num != -1)
		{
			std::cout << room_num << "번방의 " << id << "번 플레이어" << "로드완료\n";
			rooms_[room_num].player_load_complete(clients_[id].socket);
			if (rooms_[room_num].all_load_complete())
			{
				PKT_LOAD_COMPLETE pktgamestate;
				pktgamestate.PktID = (char)PKT_ID_LOAD_COMPLETE_ALL;
				pktgamestate.PktSize = (char)sizeof(PKT_GAME_STATE);
				send_packet_to_room_player(room_num, (char*)&pktgamestate);
				std::cout << room_num << "번방 전원 로드 완료\n";

				Player* players = rooms_[room_num].get_players();
				for (int i = 0; i < MAX_CLIENT; ++i)
				{
					if (players[i].get_use())
					{
						PKT_PLAYER_INFO pktdata;
						pktdata.PktId = (char)PKT_ID_PLAYER_INFO;
						pktdata.PktSize = (char)sizeof(PKT_PLAYER_INFO);
						pktdata.ID = i;
						pktdata.WorldMatrix = rooms_[room_num].get_player_worldmatrix(i);
						rooms_[room_num].set_player_is_play(i, true);
						rooms_[room_num].set_object_id(i);
						send_packet_to_player(players[i].get_serverid(), (char*)&pktdata);

						PKT_CREATE_OBJECT anotherpktdata;
						for (int j = 0; j < MAX_CLIENT; ++j)
						{
							if (!players[j].get_use()) continue;
							if (i == j) continue;

							anotherpktdata.PktId = (char)PKT_ID_CREATE_OBJECT;
							anotherpktdata.PktSize = (char)sizeof(PKT_CREATE_OBJECT);
							anotherpktdata.Object_Type = OBJECT_TYPE_PLAYER;
							anotherpktdata.Object_Index = j;
							anotherpktdata.WorldMatrix = rooms_[room_num].get_player_worldmatrix(j);
							anotherpktdata.Robot_Type = (ROBOT_TYPE)players[j].get_robot();
							send_packet_to_player(players[i].get_serverid(), (char*)&anotherpktdata);

						}

						PKT_PLAYER_RESPAWN pkt_rp;
						pkt_rp.PktId = PKT_ID_PLAYER_RESPAWN;
						pkt_rp.PktSize = sizeof(PKT_PLAYER_RESPAWN);
						pkt_rp.id = i;
						pkt_rp.hp = rooms_[room_num].get_object(i)->GetMaxHitPoint();
						pkt_rp.point = rooms_[room_num].get_respawn_point(i);
						pkt_rp.team = rooms_[room_num].get_object(i)->get_team();
						send_packet_to_player(players[i].get_serverid(), (char*)&pkt_rp);
					}
				}

				rooms_[room_num].set_blue_score(MAX_SCORE);
				rooms_[room_num].set_red_score(MAX_SCORE);

				PKT_SCORE scorepkt;
				scorepkt.PktSize = sizeof(PKT_SCORE);
				scorepkt.PktId = PKT_ID_SCORE;
				scorepkt.BlueScore = rooms_[room_num].get_blue_score();
				scorepkt.RedScore = rooms_[room_num].get_red_score();
				send_packet_to_room_player(room_num, (char*)&scorepkt);
			}
		}
		break;
	}
	case PKT_ID_CREATE_ROOM:
	{
		int room_num = find_empty_room();
		if (room_num != -1)
		{
			PKT_CREATE_ROOM_OK pkt_cro;
			pkt_cro.PktId = PKT_ID_CREATE_ROOM_OK;
			pkt_cro.PktSize = sizeof(PKT_CREATE_ROOM_OK);

			send_packet_to_player(id, (char*)&pkt_cro);
			rooms_[room_num].set_is_use(true);
			rooms_[room_num].add_player(id, clients_[id].socket, 0);
			rooms_[room_num].set_map(4);
			rooms_[room_num].set_name(reinterpret_cast<PKT_CREATE_ROOM*>(packet)->name);
			clients_[id].in_room = true;

			PKT_ADD_ROOM pkt_ar;
			pkt_ar.PktId = PKT_ID_ADD_ROOM;
			pkt_ar.PktSize = sizeof(pkt_ar);
			pkt_ar.Room_num = room_num;
			lstrcpynW(pkt_ar.name, reinterpret_cast<PKT_CREATE_ROOM*>(packet)->name, MAX_ROOM_NAME_LENGTH);

			send_packet_to_all_player((char*)&pkt_ar);

			PKT_CHANGE_ROOM_INFO pkt_cmi;
			pkt_cmi.PktSize = sizeof(PKT_CHANGE_ROOM_INFO);
			pkt_cmi.PktId = PKT_ID_CHANGE_ROOM_INFO;
			pkt_cmi.Room_num = room_num;
			pkt_cmi.numpeople = rooms_[room_num].get_num_player_in_room();
			pkt_cmi.map = rooms_[room_num].get_map();
			send_packet_to_all_player((char*)&pkt_cmi);

			std::cout << id << "번 플레이어" << room_num << "번 방 생성\n";
		}
		else
			std::cout << "더이상 방을 생성할 수 없음\n";
		break;
	}
	case PKT_ID_ROOM_IN:
	{
		int room_num = reinterpret_cast<PKT_ROOM_IN*>(packet)->Room_num;
		if (rooms_[room_num].get_num_player_in_room() < MAX_CLIENT && !rooms_[room_num].get_playing())
		{
			PKT_ROOM_IN_OK pkt_rio;
			pkt_rio.PktId = PKT_ID_ROOM_IN_OK;
			pkt_rio.PktSize = sizeof(PKT_ROOM_IN_OK);
			int player_id = rooms_[room_num].findindex();
			pkt_rio.index = player_id;
			pkt_rio.map = rooms_[room_num].get_map();
			char empty_slot = rooms_[room_num].get_empty_slot();
			pkt_rio.slot = empty_slot;
			rooms_[room_num].add_player(id, clients_[id].socket, empty_slot);
			send_packet_to_player(id, (char*)&pkt_rio);
			clients_[id].in_room = true;

			// 새로들어온 애한테 원래있던애들 알려주기
			for (int i = 0; i < MAX_CLIENT; ++i)
			{
				if (i == player_id) continue;
				auto player = rooms_[room_num].get_player(i);
				if (player->get_use())
				{
					PKT_PLAYER_IN pkt_pin;
					pkt_pin.PktId = (char)PKT_ID_PLAYER_IN;
					pkt_pin.PktSize = (char)sizeof(PKT_PLAYER_IN);
					pkt_pin.id = i;
					pkt_pin.Team = player->get_team();
					pkt_pin.robot = player->get_robot();
					pkt_pin.slot = player->get_slot();
					lstrcpynW(pkt_pin.name, clients_[player->get_serverid()].name, MAX_NAME_LENGTH);

					send_packet_to_player(id, (char*)&pkt_pin);
				}
			}

			PKT_PLAYER_IN pkt_pin;
			pkt_pin.PktId = (char)PKT_ID_PLAYER_IN;
			pkt_pin.PktSize = (char)sizeof(PKT_PLAYER_IN);
			pkt_pin.id = player_id;
			pkt_pin.Team = pkt_rio.slot % 2;
			pkt_pin.robot = ROBOT_TYPE_GM;
			pkt_pin.slot = pkt_rio.slot;
			lstrcpynW(pkt_pin.name, clients_[id].name, MAX_NAME_LENGTH);

			// 원래있던애들한테 새로들어온애 알려주기
			for (int i = 0; i < MAX_CLIENT; ++i)
			{
				if (i == player_id) continue;
				auto player = rooms_[room_num].get_player(i);
				if (!player->get_use()) continue;
				send_packet_to_player(player->get_serverid(), (char*)&pkt_pin);
			}

			PKT_CHANGE_ROOM_INFO pkt_cmi;
			pkt_cmi.PktSize = sizeof(PKT_CHANGE_ROOM_INFO);
			pkt_cmi.PktId = PKT_ID_CHANGE_ROOM_INFO;
			pkt_cmi.Room_num = room_num;
			pkt_cmi.numpeople = rooms_[room_num].get_num_player_in_room();
			pkt_cmi.map = rooms_[room_num].get_map();
			send_packet_to_all_player((char*)&pkt_cmi);

			std::cout << id << "번 플레이어" << room_num << "번 방 입장\n";
		}
		else
			std::cout << id << "번 플레이어" << reinterpret_cast<PKT_ROOM_IN*>(packet)->Room_num << "번 방에 참가 실패\n";
		break;
	}
	case PKT_ID_SHOOT:
	{
		int room_num = search_client_in_room(clients_[id].socket);
		if (room_num != -1)
		{
			using namespace std;
			using namespace chrono;

			if (reinterpret_cast<PKT_SHOOT*>(packet)->Player_Weapon == WEAPON_TYPE_GM_GUN)
			{
				int index;
				auto pkt_ce = rooms_[room_num].shoot(reinterpret_cast<PKT_SHOOT*>(packet)->ID,
					reinterpret_cast<PKT_SHOOT*>(packet)->BulletWorldMatrix,
					reinterpret_cast<PKT_SHOOT*>(packet)->Player_Weapon, 600.0f, &index);
				add_timer(index, room_num, EVENT_TYPE_GM_GUN, high_resolution_clock::now() + 16ms);
				delete pkt_ce;
			}
			else if (reinterpret_cast<PKT_SHOOT*>(packet)->Player_Weapon == WEAPON_TYPE_BEAM_RIFLE)
			{
				int index;
				auto pkt_ce = rooms_[room_num].shoot(reinterpret_cast<PKT_SHOOT*>(packet)->ID,
					reinterpret_cast<PKT_SHOOT*>(packet)->BulletWorldMatrix,
					reinterpret_cast<PKT_SHOOT*>(packet)->Player_Weapon, 1000.0f, &index);
				add_timer(index, room_num, EVENT_TYPE_BEAM_RIFLE, high_resolution_clock::now() + 16ms);
				delete pkt_ce;
			}
			else if (reinterpret_cast<PKT_SHOOT*>(packet)->Player_Weapon == WEAPON_TYPE_BEAM_SNIPER)
			{
				int index;
				auto pkt_ce = rooms_[room_num].shoot(reinterpret_cast<PKT_SHOOT*>(packet)->ID,
					reinterpret_cast<PKT_SHOOT*>(packet)->BulletWorldMatrix,
					reinterpret_cast<PKT_SHOOT*>(packet)->Player_Weapon, 2000.0f, &index);
				add_timer(index, room_num, EVENT_TYPE_BEAM_SNIPER, high_resolution_clock::now() + 16ms);
				delete pkt_ce;
			}
			else
			{
				auto pkt_co = rooms_[room_num].shoot(reinterpret_cast<PKT_SHOOT*>(packet)->ID,
					reinterpret_cast<PKT_SHOOT*>(packet)->BulletWorldMatrix,
					reinterpret_cast<PKT_SHOOT*>(packet)->Player_Weapon);

				send_packet_to_room_player(room_num, (char*)pkt_co);
				add_timer(pkt_co->Object_Index, room_num, EVENT_TYPE_OBJECT_MOVE, high_resolution_clock::now() + 16ms);
				delete pkt_co;
			}
		}
		break;
	}
	case PKT_ID_LOBBY_PLAYER_INFO:
	{
		int room_num = search_client_in_room(clients_[id].socket);
		if (room_num != -1)
		{
			rooms_[room_num].set_player_lobby_info(reinterpret_cast<PKT_LOBBY_PLAYER_INFO*>(packet)->id, reinterpret_cast<PKT_LOBBY_PLAYER_INFO*>(packet)->selected_robot,
				reinterpret_cast<PKT_LOBBY_PLAYER_INFO*>(packet)->Team, reinterpret_cast<PKT_LOBBY_PLAYER_INFO*>(packet)->slot);
			reinterpret_cast<PKT_LOBBY_PLAYER_INFO*>(packet)->Team = rooms_[room_num].get_player_team(reinterpret_cast<PKT_LOBBY_PLAYER_INFO*>(packet)->id);
			send_packet_to_room_player(room_num, packet);
		}
		break;
	}
	case PKT_ID_CHANGE_MAP:
	{
		int room_num = search_client_in_room(clients_[id].socket);
		if (room_num != -1)
		{
			rooms_[room_num].set_map(reinterpret_cast<PKT_CHANGE_MAP*>(packet)->map);

			PKT_CHANGE_MAP pkt_cm;
			pkt_cm.map = reinterpret_cast<PKT_CHANGE_MAP*>(packet)->map;
			pkt_cm.PktId = PKT_ID_CHANGE_MAP;
			pkt_cm.PktSize = sizeof(PKT_CHANGE_MAP);
			send_packet_to_room_player(room_num, (char*)&pkt_cm);

			PKT_CHANGE_ROOM_INFO pkt_cmi;
			pkt_cmi.PktSize = sizeof(PKT_CHANGE_ROOM_INFO);
			pkt_cmi.PktId = PKT_ID_CHANGE_ROOM_INFO;
			pkt_cmi.Room_num = room_num;
			pkt_cmi.numpeople = rooms_[room_num].get_num_player_in_room();
			pkt_cmi.map = rooms_[room_num].get_map();
			send_packet_to_all_player((char*)&pkt_cmi);
		}
		break;
	}
	case PKT_ID_GAME_START:
	{
		int room_num = search_client_in_room(clients_[id].socket);
		if (room_num != -1)
		{
			PKT_GAME_START pkt_gs;
			pkt_gs.map = rooms_[room_num].get_map();
			pkt_gs.PktID = PKT_ID_GAME_START;
			pkt_gs.PktSize = sizeof(PKT_GAME_START);
			send_packet_to_room_player(room_num, (char*)&pkt_gs);

			rooms_[room_num].start_game();

			using namespace std;
			using namespace chrono;
			add_timer(room_num, room_num, EVENT_TYPE_ROOM_UPDATE, high_resolution_clock::now() + 16ms);
		}
		break;
	}
	case PKT_ID_LEAVE_ROOM:
	{
		int room_num = search_client_in_room(clients_[id].socket);
		if (room_num != -1)
		{
			PKT_PLAYER_OUT packet;
			packet.id = rooms_[room_num].find_player_by_socket(clients_[id].socket);
			packet.PktId = PKT_ID_PLAYER_OUT;
			packet.PktSize = sizeof(PKT_PLAYER_OUT);
			send_packet_to_room_player(room_num, (char*)&packet);

			rooms_[room_num].disconnect_client(clients_[id].socket);
			clients_[id].in_room = false;

			if (rooms_[room_num].get_num_player_in_room() > 0)
			{
				PKT_CHANGE_ROOM_INFO pkt_cmi;
				pkt_cmi.PktSize = sizeof(PKT_CHANGE_ROOM_INFO);
				pkt_cmi.PktId = PKT_ID_CHANGE_ROOM_INFO;
				pkt_cmi.Room_num = room_num;
				pkt_cmi.numpeople = rooms_[room_num].get_num_player_in_room();
				pkt_cmi.map = rooms_[room_num].get_map();
				send_packet_to_all_player((char*)&pkt_cmi);
			}
			else
			{
				rooms_[room_num].set_is_use(false);
				PKT_ROOM_DELETE pkt_rd;
				pkt_rd.PktId = PKT_ID_DELETE_ROOM;
				pkt_rd.PktSize = sizeof(PKT_ROOM_DELETE);
				pkt_rd.Room_num = room_num;
				send_packet_to_all_player((char*)&pkt_rd);
			}

			for (int i = 0; i < 10; ++i)
			{
				if (rooms_[i].get_is_use())
				{
					PKT_ADD_ROOM pkt_ar;
					pkt_ar.PktId = PKT_ID_ADD_ROOM;
					pkt_ar.PktSize = sizeof(pkt_ar);
					pkt_ar.Room_num = i;
					lstrcpynW(pkt_ar.name, rooms_[i].get_name(), MAX_ROOM_NAME_LENGTH);
					send_packet_to_player(id, (char*)&pkt_ar);

					PKT_CHANGE_ROOM_INFO pkt_cmi;
					pkt_cmi.PktSize = sizeof(PKT_CHANGE_ROOM_INFO);
					pkt_cmi.PktId = PKT_ID_CHANGE_ROOM_INFO;
					pkt_cmi.Room_num = i;
					pkt_cmi.numpeople = rooms_[i].get_num_player_in_room();
					pkt_cmi.map = rooms_[i].get_map();
					send_packet_to_player(id, (char*)&pkt_cmi);
				}
			}
		}
		break;
	}
	case PKT_ID_MOVE_TEAM:
	{
		int room_num = search_client_in_room(clients_[id].socket);
		if (room_num != -1)
		{
			int player = rooms_[room_num].find_player_by_socket(clients_[id].socket);
			rooms_[room_num].change_team(player, reinterpret_cast<PKT_MOVE_TEAM*>(packet)->team);

			auto player_data = rooms_[room_num].get_player(player);
			PKT_LOBBY_PLAYER_INFO pkt_lpi;
			pkt_lpi.id = player;
			pkt_lpi.PktId = PKT_ID_LOBBY_PLAYER_INFO;
			pkt_lpi.PktSize = sizeof(PKT_LOBBY_PLAYER_INFO);
			pkt_lpi.selected_robot = player_data->get_robot();
			pkt_lpi.slot = player_data->get_slot();
			pkt_lpi.Team = player_data->get_team();

			send_packet_to_room_player(room_num, (char*)&pkt_lpi);
		}
		break;
	}
	case PKT_ID_CHANGE_NAME:
	{
		PKT_CHANGE_NAME* pkt_cn = reinterpret_cast<PKT_CHANGE_NAME*>(packet);
		std::wcout << id << L"번 플레이어의 이름을 " << clients_[id].name << L"에서 " << pkt_cn->name << L"로 변경\n";
		lstrcpynW(clients_[id].name, pkt_cn->name, MAX_NAME_LENGTH);
		break;
	}
	case PKT_ID_MOVE_TO_MAIN_LOBBY:
	{
		int room_num = search_client_in_room(clients_[id].socket);
		if (room_num != -1)
		{
			rooms_[room_num].disconnect_client(clients_[id].socket);
			clients_[id].in_room = false;

			for (int i = 0; i < 10; ++i)
			{
				if (rooms_[i].get_is_use())
				{
					PKT_ADD_ROOM pkt_ar;
					pkt_ar.PktId = PKT_ID_ADD_ROOM;
					pkt_ar.PktSize = sizeof(pkt_ar);
					pkt_ar.Room_num = i;
					lstrcpynW(pkt_ar.name, rooms_[i].get_name(), MAX_ROOM_NAME_LENGTH);
					send_packet_to_player(id, (char*)&pkt_ar);

					PKT_CHANGE_ROOM_INFO pkt_cmi;
					pkt_cmi.PktSize = sizeof(PKT_CHANGE_ROOM_INFO);
					pkt_cmi.PktId = PKT_ID_CHANGE_ROOM_INFO;
					pkt_cmi.Room_num = i;
					pkt_cmi.numpeople = rooms_[i].get_num_player_in_room();
					pkt_cmi.map = rooms_[i].get_map();
					send_packet_to_player(id, (char*)&pkt_cmi);
				}
			}

			if (rooms_[room_num].get_num_player_in_room() > 0)
			{
				PKT_CHANGE_ROOM_INFO pkt_cmi;
				pkt_cmi.PktSize = sizeof(PKT_CHANGE_ROOM_INFO);
				pkt_cmi.PktId = PKT_ID_CHANGE_ROOM_INFO;
				pkt_cmi.Room_num = room_num;
				pkt_cmi.numpeople = rooms_[room_num].get_num_player_in_room();
				pkt_cmi.map = rooms_[room_num].get_map();
				send_packet_to_all_player((char*)&pkt_cmi);
			}
			else
			{
				rooms_[room_num].end_game();
				rooms_[room_num].set_is_use(false);
				PKT_ROOM_DELETE pkt_rd;
				pkt_rd.PktId = PKT_ID_DELETE_ROOM;
				pkt_rd.PktSize = sizeof(PKT_ROOM_DELETE);
				pkt_rd.Room_num = room_num;
				send_packet_to_all_player((char*)&pkt_rd);
			}
		}
		break;
	}
	default:
		std::wcout << L"정의되지 않은 패킷 도착 오류!!\n";
		break;
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
			//while (true);
		}
	}
}

void Framawork::send_packet_to_all_player(char* packet)
{
	for (int i = 0; i < 1000; ++i)
	{
		if (!clients_[i].in_use) continue;
		if (clients_[i].in_room) continue;
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

void Framawork::send_packet_to_team_player(int room, char * packet, char team)
{
	for (int i = 0; i < 8; i++)
	{
		Player* player = rooms_[room].get_player(i);
		if (!player->get_use()) continue;
		if (player->get_team() != team) continue;
		send_packet_to_player(player->get_serverid(), packet);
		//std::cout << (int)team << "팀의 " << i << "번째 플레이어에게 보냄\n";
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
