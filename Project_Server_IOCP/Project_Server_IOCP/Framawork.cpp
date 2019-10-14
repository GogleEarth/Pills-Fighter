#include "pch.h"
#include "Framawork.h"
#include "Protocol.h"
#include "DBServer_Protocol.h"

Framawork::Framawork()
{
}


Framawork::~Framawork()
{
}

void Framawork::init()
{
	iocp_ = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);

	repository_ = new Repository();

	for (int i = 0; i < 1000; i++)
		clients_[i].in_use_ = false;

	for (int i = 0; i < 10; ++i)
		rooms_[i].init(repository_);

#ifdef WITH_DATA_BASE
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		std::cout << "DBServer Connect Fail!\n";
		return;
	}

	dbserver_socket_ = socket(AF_INET, SOCK_STREAM, 0);
	if (dbserver_socket_ == INVALID_SOCKET)
	{
		std::cout << "DBServer Connect Fail!\n";
		return;
	}

	SOCKADDR_IN serveraddr;
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = inet_addr(DBSERVERIP);
	serveraddr.sin_port = htons(DBSERVERPORT);

	if (connect(dbserver_socket_, (SOCKADDR *)&serveraddr, sizeof(serveraddr)) == SOCKET_ERROR)
	{
		std::cout << "DBServer Connect Fail!\n";
		return;
	}

	clients_[DBSERVER_KEY].socket_ = dbserver_socket_;
	clients_[DBSERVER_KEY].prev_size_ = 0;
	clients_[DBSERVER_KEY].in_use_ = true;
	clients_[DBSERVER_KEY].is_server_ = true;
	ZeroMemory(&clients_[DBSERVER_KEY].over_ex.overlapped_,
		sizeof(clients_[DBSERVER_KEY].over_ex.overlapped_));

	CreateIoCompletionPort(reinterpret_cast<HANDLE>(clients_[DBSERVER_KEY].socket_),
		iocp_, DBSERVER_KEY, 0);

	do_recv(DBSERVER_KEY);

	std::cout << "DBServer Connect OK!\n";
#endif
}

int Framawork::thread_process()
{
	while (true) {
		DWORD io_byte;
		ULONGLONG key;
		Overlapped *overlapped;
		bool is_error = GetQueuedCompletionStatus(iocp_, &io_byte,
			&key, reinterpret_cast<LPWSAOVERLAPPED *>(&overlapped),
			INFINITE);
		int index = static_cast<int>(key);
		if (false == is_error) {
			int err_no = WSAGetLastError();
			if (64 == err_no) {

				disconnect_client(index);
				std::wcout << index << L"번 플레이어 접속 종료\n";
				lstrcpynW(clients_[index].name_, L"라마바", MAX_NAME_LENGTH);
				continue;
			}
			else error_display("GQCS : ", err_no);
		}

		if (0 == io_byte) {
			disconnect_client(index);
			std::wcout << index << L"번 플레이어 접속 종료\n";
			lstrcpynW(clients_[index].name_, L"라마바", MAX_NAME_LENGTH);

			continue;
		}

		if (EVENT_TYPE_RECV == overlapped->event_type_) {
			int rest = io_byte;
			char *buffer = overlapped->packet_buffer_;
			char packet_size = 0;
			if (clients_[index].prev_size_ > 0)
				packet_size = clients_[index].buffer_[0];
			while (rest > 0) {
				if (packet_size == 0) packet_size = buffer[0];
				int required = packet_size - clients_[index].prev_size_;
				if (required <= rest) {
					memcpy(clients_[index].buffer_ + clients_[index].prev_size_,
						buffer, required);
					process_packet(index, clients_[index].buffer_);
					rest -= required;
					buffer += required;
					packet_size = 0;
					clients_[index].prev_size_ = 0;
				}
				else {
					memcpy(clients_[index].buffer_ + clients_[index].prev_size_,
						buffer, rest);
					rest = 0;
					clients_[index].prev_size_ += rest;
				}
			}
			do_recv(index);
		}
		else if (EVENT_TYPE_SEND == overlapped->event_type_) 
		{
			delete overlapped;
		}
		else if (EVENT_TYPE_ROOM_UPDATE == overlapped->event_type_)
		{
			using namespace std::chrono;
			float elapsed_time;

			if (overlapped->elapsed_time_ <= 0.001f)
				elapsed_time = 0.016f;
			else
				elapsed_time = overlapped->elapsed_time_;

			if (!rooms_[index].get_game_end())
			{
				rooms_[index].room_update(elapsed_time);

				while (true)
				{
					auto data = rooms_[index].create_object_dequeue();
					if (data == nullptr) break;
					if (data->Object_Type == OBJECT_TYPE_METEOR)
					{
						data->Object_Index = rooms_[index].add_object(data->Object_Type, data->WorldMatrix);
						add_event(data->Object_Index, index, EVENT_TYPE_OBJECT_MOVE, high_resolution_clock::now() + 16ms);
					}
					else if (data->Object_Type == OBJECT_TYPE_ITEM_AMMO || data->Object_Type == OBJECT_TYPE_ITEM_HEALING)
					{
						add_event(data->Object_Index, index, EVENT_TYPE_ITEM, high_resolution_clock::now() + 16ms);
					}
					send_packet_to_room_player(index, (char*)data);
					delete data;
				}

				while (true)
				{
					auto data = rooms_[index].map_event_dequeue();
					if (data == nullptr) break;
					if (data->type == MAP_EVENT_TYPE_ALERT)
						std::cout << index << "번방 이벤트 경고\n";
					if (data->type == MAP_EVENT_TYPE_START)
						std::cout << index << "번방 이벤트 시작\n";
					if (data->type == MAP_EVENT_TYPE_END)
						std::cout << index << "번방 이벤트 끝\n";
					send_packet_to_room_player(index, (char*)data);
					delete data;
				}

				while (true)
				{
					auto data = rooms_[index].player_life_dequeue();
					if (data == nullptr) break;
					send_packet_to_team_player(index, (char*)data, rooms_[index].get_player_team(data->ID));
					delete data;
				}

				while (true)
				{
					auto data = rooms_[index].player_die_dequeue();
					if (data == nullptr) break;
					send_packet_to_room_player(index, (char*)data);
					using namespace std::chrono;
					add_event(data->id, overlapped->room_num_, EVENT_TYPE_RESPAWN, high_resolution_clock::now() + 16ms);
					delete data;
				}

				while (true)
				{
					auto data = rooms_[index].kill_message_dequeue();
					if (data == nullptr) break;
					send_packet_to_room_player(index, (char*)data);
					delete data;
				}

				while (true)
				{
					auto data = rooms_[index].score_dequeue();
					if (data == nullptr) break;
					send_packet_to_room_player(index, (char*)data);

					if (rooms_[index].get_blue_score() <= 0 || rooms_[index].get_red_score() <= 0)
					{
						PKT_GAME_END pkt_ge;
						pkt_ge.PktId = PKT_ID_GAME_END;
						pkt_ge.PktSize = sizeof(PKT_GAME_END);
						if (rooms_[index].get_blue_score() <= 0)
							pkt_ge.WinTeam = 0;
						else
							pkt_ge.WinTeam = 1;

						rooms_[index].set_game_end(true);

						send_packet_to_room_player(index, (char*)&pkt_ge);
					}

					delete data;
				}

				while (true)
				{
					auto data = rooms_[index].create_effect_dequeue();
					if (data == nullptr) break;
					if (data->efType == EFFECT_TYPE_BEAM_HIT)
						send_packet_to_room_player(index, (char*)data);
					else
						send_packet_to_player(rooms_[index].get_player(data->id)->get_serverid(), (char*)data);
					delete data;
				}

				while (true)
				{
					auto data = rooms_[index].item_dequeue();
					if (data == nullptr) break;
					if (data->Item_type == ITEM_TYPE_AMMO1 || data->Item_type == ITEM_TYPE_AMMO2)
						data->Item_type = ITEM_TYPE_AMMO;
					send_packet_to_team_player(index, (char*)data, rooms_[index].get_player_team(data->ID));
					delete data;
				}
			}
			
			if (rooms_[index].get_num_player_in_room() > 0)
				add_event(index, index, EVENT_TYPE_ROOM_UPDATE, high_resolution_clock::now() + 16ms);
			else
				rooms_[index].init();

			delete overlapped;
		}
		else if (EVENT_TYPE_OBJECT_MOVE == overlapped->event_type_)
		{
			if (rooms_[overlapped->room_num_].get_playing())
			{
				auto object = rooms_[overlapped->room_num_].get_object(index);
				float elapsed_time;

				if (overlapped->elapsed_time_ <= 0.001f)
					elapsed_time = 0.016f;
				else
					elapsed_time = overlapped->elapsed_time_;

				object->animate(elapsed_time, rooms_[overlapped->room_num_].get_map());
				rooms_[overlapped->room_num_].check_collision_obstacles(index);
				rooms_[overlapped->room_num_].check_collision_player(index);

				if (object->is_delete())
				{
					PKT_DELETE_OBJECT pkt_do;
					pkt_do.PktId = PKT_ID_DELETE_OBJECT;
					pkt_do.PktSize = sizeof(PKT_DELETE_OBJECT);
					pkt_do.Object_Index = index;
					send_packet_to_room_player(overlapped->room_num_, (char*)&pkt_do);

					OBJECT_TYPE type = object->get_object_type();
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
						pktCE.xmf3Position = object->get_position();
						send_packet_to_room_player(overlapped->room_num_, (char*)&pktCE);
					}
					object->set_use(false);
				}
				else
				{
					PKT_UPDATE_OBJECT pkt_uo;
					pkt_uo.PktId = PKT_ID_UPDATE_OBJECT;
					pkt_uo.PktSize = sizeof(PKT_UPDATE_OBJECT);
					pkt_uo.Object_Index = index;
					pkt_uo.Object_Position = object->get_position();

					send_packet_to_room_player(overlapped->room_num_, (char*)&pkt_uo);

					using namespace std::chrono;
					add_event(index, overlapped->room_num_, EVENT_TYPE_OBJECT_MOVE, high_resolution_clock::now() + 16ms);
				}
			}
			delete overlapped;
		}
		else if (EVENT_TYPE_ITEM == overlapped->event_type_)
		{
			if(rooms_[overlapped->room_num_].get_playing())
			{
				auto object = rooms_[overlapped->room_num_].get_object(index);
				float elapsed_time;

				if (overlapped->elapsed_time_ <= 0.001f)
					elapsed_time = 0.016f;
				else
					elapsed_time = overlapped->elapsed_time_;

				object->animate(elapsed_time, rooms_[overlapped->room_num_].get_map());
				rooms_[overlapped->room_num_].check_collision_player(index);

				if (object->is_delete())
				{
					PKT_DELETE_OBJECT pkt_do;
					pkt_do.PktId = PKT_ID_DELETE_OBJECT;
					pkt_do.PktSize = sizeof(PKT_DELETE_OBJECT);
					pkt_do.Object_Index = index;
					send_packet_to_room_player(overlapped->room_num_, (char*)&pkt_do);

					object->set_use(false);
				}
				else
				{
					using namespace std::chrono;
					add_event(index, overlapped->room_num_, EVENT_TYPE_ITEM, high_resolution_clock::now() + 16ms);
				}
			}
			delete overlapped;
		}
		else if (EVENT_TYPE_BEAM_RIFLE == overlapped->event_type_)
		{
			if (rooms_[overlapped->room_num_].get_playing())
			{
				auto object = rooms_[overlapped->room_num_].get_object(index);
				float elapsed_time;

				if (overlapped->elapsed_time_ <= 0.001f)
					elapsed_time = 0.016f;
				else
					elapsed_time = overlapped->elapsed_time_;

				object->animate(elapsed_time, rooms_[overlapped->room_num_].get_map());

				float distance = 1000.0f;
				rooms_[overlapped->room_num_].check_collision_player_to_vector(index, 1000.0f, &distance);
				
				PKT_CREATE_EFFECT pkt_ce;
				pkt_ce.PktId = PKT_ID_CREATE_EFFECT;
				pkt_ce.PktSize = sizeof(PKT_CREATE_EFFECT);
				pkt_ce.EftAnitType = EFFECT_ANIMATION_TYPE_ONE;
				pkt_ce.efType = EFFECT_TYPE_BEAM_RIFLE;
				pkt_ce.xmf3Look = object->get_look();
				pkt_ce.xmf3Position = object->get_position();
				pkt_ce.fDistance = distance;

				send_packet_to_room_player(overlapped->room_num_, (char*)&pkt_ce);

				if (object->is_delete())
				{
					object->set_use(false);
				}
				else
				{
					using namespace std::chrono;
					add_event(index, overlapped->room_num_, EVENT_TYPE_BEAM_RIFLE, high_resolution_clock::now() + 16ms);
				}
			}
			delete overlapped;
		}
		else if (EVENT_TYPE_GM_GUN == overlapped->event_type_)
		{
			if (rooms_[overlapped->room_num_].get_playing())
			{
				auto object = rooms_[overlapped->room_num_].get_object(index);
				float elapsed_time;

				if (overlapped->elapsed_time_ <= 0.001f)
					elapsed_time = 0.016f;
				else
					elapsed_time = overlapped->elapsed_time_;

				object->animate(elapsed_time, rooms_[overlapped->room_num_].get_map());

				float distance = 600.0f;
				rooms_[overlapped->room_num_].check_collision_player_to_vector(index, 600.0f, &distance);

				PKT_CREATE_EFFECT pkt_ce;
				pkt_ce.PktId = PKT_ID_CREATE_EFFECT;
				pkt_ce.PktSize = sizeof(PKT_CREATE_EFFECT);
				pkt_ce.EftAnitType = EFFECT_ANIMATION_TYPE_ONE;
				pkt_ce.efType = EFFECT_TYPE_GM_GUN;
				pkt_ce.xmf3Look = object->get_look();
				pkt_ce.xmf3Position = object->get_position();
				pkt_ce.fDistance = distance;

				send_packet_to_room_player(overlapped->room_num_, (char*)&pkt_ce);

				if (object->is_delete())
				{
					object->set_use(false);
				}
				else
				{
					using namespace std::chrono;
					add_event(index, overlapped->room_num_, EVENT_TYPE_GM_GUN, high_resolution_clock::now() + 16ms);
				}
			}
			delete overlapped;
		}
		else if (EVENT_TYPE_SABER == overlapped->event_type_)
		{
			if (rooms_[overlapped->room_num_].get_playing())
			{
				auto object = rooms_[overlapped->room_num_].get_object(index);
				float elapsed_time;

				if (overlapped->elapsed_time_ <= 0.001f)
					elapsed_time = 0.016f;
				else
					elapsed_time = overlapped->elapsed_time_;

				object->animate(elapsed_time, rooms_[overlapped->room_num_].get_map());
				rooms_[overlapped->room_num_].check_saber_collision_player(index);

				if (object->is_delete())
				{
					object->set_use(false);
				}
				else
				{
					using namespace std::chrono;
					add_event(index, overlapped->room_num_, EVENT_TYPE_SABER, high_resolution_clock::now() + 16ms);
				}
			}
			delete overlapped;
		}
		else if (EVENT_TYPE_RESPAWN == overlapped->event_type_)
		{
			if (rooms_[overlapped->room_num_].get_playing())
			{
				auto object = rooms_[overlapped->room_num_].get_object(index);

				if (!object->get_is_die())
				{
					PKT_PLAYER_RESPAWN pkt_rp;
					pkt_rp.PktId = PKT_ID_PLAYER_RESPAWN;
					pkt_rp.PktSize = sizeof(PKT_PLAYER_RESPAWN);
					pkt_rp.id = index;
					pkt_rp.hp = object->get_max_hp();
					pkt_rp.point = rooms_[overlapped->room_num_].get_respawn_point(index);
					pkt_rp.team = object->get_team();

					send_packet_to_room_player(overlapped->room_num_, (char*)&pkt_rp);
				}
				else
				{
					using namespace std::chrono;
					add_event(index, overlapped->room_num_, EVENT_TYPE_RESPAWN, high_resolution_clock::now() + 16ms);
				}
			}
			delete overlapped;
		}
		else if (EVENT_TYPE_BEAM_SNIPER == overlapped->event_type_)
		{
		if (rooms_[overlapped->room_num_].get_playing())
		{
			auto object = rooms_[overlapped->room_num_].get_object(index);
			float elapsed_time;

			if (overlapped->elapsed_time_ <= 0.001f)
				elapsed_time = 0.016f;
			else
				elapsed_time = overlapped->elapsed_time_;

			object->animate(elapsed_time, rooms_[overlapped->room_num_].get_map());

			float distance = 2000.0f;
			rooms_[overlapped->room_num_].check_collision_player_to_vector(index, 2000.0f, &distance);
			
			PKT_CREATE_EFFECT pkt_ce;
			pkt_ce.PktId = PKT_ID_CREATE_EFFECT;
			pkt_ce.PktSize = sizeof(PKT_CREATE_EFFECT);
			pkt_ce.EftAnitType = EFFECT_ANIMATION_TYPE_ONE;
			pkt_ce.efType = EFFECT_TYPE_BEAM_SNIPER;
			pkt_ce.xmf3Look = object->get_look();
			pkt_ce.xmf3Position = object->get_position();
			pkt_ce.fDistance = distance;

			send_packet_to_room_player(overlapped->room_num_, (char*)&pkt_ce);

			if (object->is_delete())
			{
				object->set_use(false);
			}
			else
			{
				using namespace std::chrono;
				add_event(index, overlapped->room_num_, EVENT_TYPE_BEAM_SNIPER, high_resolution_clock::now() + 16ms);
			}
		}
		delete overlapped;
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
	WSADATA wsadata;
	if (WSAStartup(MAKEWORD(2, 2), &wsadata) != 0)
	{
		std::cout << "Error - Can not load 'winsock.dll' file\n";
		return 1;
	}

	SOCKET listen_socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (listen_socket == INVALID_SOCKET)
	{
		std::cout << "Error - Invalid socket\n";
		return 1;
	}

	SOCKADDR_IN serverAddr;
	memset(&serverAddr, 0, sizeof(SOCKADDR_IN));
	serverAddr.sin_family = PF_INET;
	serverAddr.sin_port = htons(SERVERPORT);
	serverAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);

	if (::bind(listen_socket, (struct sockaddr*)&serverAddr,
		sizeof(SOCKADDR_IN)) == SOCKET_ERROR)
	{
		std::cout << "Error - Fail bind\n";
		closesocket(listen_socket);
		WSACleanup();
		return 1;
	}

	if (listen(listen_socket, 5) == SOCKET_ERROR)
	{
		std::cout << "Error - Fail listen\n";
		closesocket(listen_socket);
		WSACleanup();
		return 1;
	}

	SOCKADDR_IN client_addr;
	int addr_len = sizeof(SOCKADDR_IN);
	memset(&client_addr, 0, addr_len);
	SOCKET client_socket;
	DWORD flags;

	while (1)
	{
		client_socket = accept(listen_socket, (struct sockaddr *)&client_addr, &addr_len);
		if (client_socket == INVALID_SOCKET)
		{
			std::cout << "Error - Accept Failure\n";
			return 1;
		}

		accept_l.lock();
		int new_id = -1;
		for (int i = 0; i < MAX_USER; ++i)
			if (false == clients_[i].in_use_) {
				new_id = i;
				break;
			}
		accept_l.unlock();

		if (-1 == new_id) {
			std::cout << "MAX USER overflow\n";
			continue;
		}

		//std::cout << "플레이어 접속\n";

		clients_[new_id].socket_ = client_socket;
		clients_[new_id].prev_size_ = 0;
		clients_[new_id].in_use_ = true;
		ZeroMemory(&clients_[new_id].over_ex.overlapped_,
			sizeof(clients_[new_id].over_ex.overlapped_));
		flags = 0;

		CreateIoCompletionPort(reinterpret_cast<HANDLE>(client_socket),
			iocp_, new_id, 0);

		PKT_CHANGE_NAME pkt_cn;
		pkt_cn.PktId = PKT_ID_CHANGE_NAME;
		pkt_cn.PktSize = sizeof(PKT_CHANGE_NAME);
		lstrcpynW(pkt_cn.name, clients_[new_id].name_, MAX_NAME_LENGTH);

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

	closesocket(listen_socket);

	WSACleanup();

	return 0;
}

int Framawork::timer_process()
{
	while (true) {
		using namespace std::chrono;
		std::this_thread::sleep_for(16ms);
		while (true) {
			timer_l.lock();
			if (true == timer_queue.empty()) {
				timer_l.unlock();
				break;
			}
			Event event = timer_queue.top();

			if (event.start_time_ > std::chrono::high_resolution_clock::now()) {
				timer_l.unlock();
				break;
			}
			timer_queue.pop();
			timer_l.unlock();
			Overlapped *over_ex = new Overlapped;
			over_ex->event_type_ = event.type_;
			over_ex->elapsed_time_ = (float)(duration_cast<milliseconds>(std::chrono::high_resolution_clock::now() - event.start_time_).count()) / 1000.0f;
			over_ex->room_num_ = event.room_num_;
			PostQueuedCompletionStatus(iocp_, 1, event.object_index_, &over_ex->overlapped_);
		}
	}
	return 0;
}

void Framawork::add_event(int obj_id, int room_num, EVENT_TYPE et, std::chrono::high_resolution_clock::time_point start_time)
{
	timer_l.lock();
	timer_queue.emplace(Event{ obj_id, room_num, et, start_time });
	timer_l.unlock();
}

void Framawork::do_recv(int id)
{
	DWORD flags = 0;

	if (WSARecv(clients_[id].socket_, &clients_[id].over_ex.wsa_buffer_, 1,
		NULL, &flags, &(clients_[id].over_ex.overlapped_), 0))
	{
		if (WSAGetLastError() != WSA_IO_PENDING)
		{
			std::cout << "Recv_Error\n";
			disconnect_client(id);
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
	int room_num = search_client_in_room(clients_[id].socket_);

	if (room_num != -1)
	{
		PKT_PLAYER_OUT packet;
		packet.id = rooms_[room_num].find_player_by_socket(clients_[id].socket_);
		packet.PktId = PKT_ID_PLAYER_OUT;
		packet.PktSize = sizeof(PKT_PLAYER_OUT);
		send_packet_to_room_player(room_num, (char*)&packet);
		rooms_[room_num].disconnect_client(clients_[id].socket_);

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

	closesocket(clients_[id].socket_);
	clients_[id].in_use_ = false;
	clients_[id].in_room_ = false;
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
	int packet_id = packet[1];
	switch (packet_id)
	{
	case PKT_ID_PLAYER_INFO:
	{
		int room_num = search_client_in_room(clients_[id].socket_);
		if (room_num != -1)
		{
			int player = rooms_[room_num].find_player_by_socket(clients_[id].socket_);
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
							int Index = rooms_[room_num].add_object(OBJECT_TYPE_SABER, ((PKT_PLAYER_INFO*)packet)->WorldMatrix, player);
							using namespace std::chrono;
							add_event(Index, room_num, EVENT_TYPE_SABER, high_resolution_clock::now() + 16ms);
						}
					if (((PKT_PLAYER_INFO*)packet)->Player_Up_Animation == ANIMATION_TYPE_BEAM_SABER_2_ONE)
						if (((PKT_PLAYER_INFO*)packet)->UpAnimationPosition > 0.33f && ((PKT_PLAYER_INFO*)packet)->UpAnimationPosition < 0.34f)
						{
							int Index = rooms_[room_num].add_object(OBJECT_TYPE_SABER, ((PKT_PLAYER_INFO*)packet)->WorldMatrix, player);
							using namespace std::chrono;
							add_event(Index, room_num, EVENT_TYPE_SABER, high_resolution_clock::now() + 16ms);
						}
					if (((PKT_PLAYER_INFO*)packet)->Player_Up_Animation == ANIMATION_TYPE_BEAM_SABER_3_ONE)
						if (((PKT_PLAYER_INFO*)packet)->UpAnimationPosition > 0.51f && ((PKT_PLAYER_INFO*)packet)->UpAnimationPosition < 0.52f)
						{
							int Index = rooms_[room_num].add_object(OBJECT_TYPE_SABER, ((PKT_PLAYER_INFO*)packet)->WorldMatrix, player);
							using namespace std::chrono;
							add_event(Index, room_num, EVENT_TYPE_SABER, high_resolution_clock::now() + 16ms);
						}
				}
				send_packet_to_room_player(room_num, packet);
			}
		}
		break;
	}
	case PKT_ID_LOAD_COMPLETE:
	{
		int room_num = search_client_in_room(clients_[id].socket_);
		if (room_num != -1)
		{
			std::cout << room_num << "번방의 " << id << "번 플레이어" << "로드완료\n";
			rooms_[room_num].player_load_complete(clients_[id].socket_);
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
						pkt_rp.hp = rooms_[room_num].get_object(i)->get_max_hp();
						pkt_rp.point = rooms_[room_num].get_respawn_point(i);
						pkt_rp.team = rooms_[room_num].get_object(i)->get_team();
						send_packet_to_player(players[i].get_serverid(), (char*)&pkt_rp);

						std::cout << i << "번째 플레이어 정보 보냄\n";
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
			rooms_[room_num].add_player(id, clients_[id].socket_, 0, clients_[id].name_);
			rooms_[room_num].set_map(4);
			rooms_[room_num].set_name(reinterpret_cast<PKT_CREATE_ROOM*>(packet)->name);
			clients_[id].in_room_ = true;

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
		if (rooms_[room_num].get_is_use() && rooms_[room_num].get_num_player_in_room() < MAX_CLIENT && !rooms_[room_num].get_playing())
		{
			PKT_ROOM_IN_OK pkt_rio;
			pkt_rio.PktId = PKT_ID_ROOM_IN_OK;
			pkt_rio.PktSize = sizeof(PKT_ROOM_IN_OK);
			int player_id = rooms_[room_num].findindex();
			pkt_rio.index = player_id;
			pkt_rio.map = rooms_[room_num].get_map();
			char empty_slot = rooms_[room_num].get_empty_slot();
			pkt_rio.slot = empty_slot;
			rooms_[room_num].add_player(id, clients_[id].socket_, empty_slot, clients_[id].name_);
			send_packet_to_player(id, (char*)&pkt_rio);
			clients_[id].in_room_ = true;

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
					lstrcpynW(pkt_pin.name, clients_[player->get_serverid()].name_, MAX_NAME_LENGTH);

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
			lstrcpynW(pkt_pin.name, clients_[id].name_, MAX_NAME_LENGTH);

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

		//	std::cout << id << "번 플레이어" << room_num << "번 방 입장\n";
		}
		//else
		//	std::cout << id << "번 플레이어" << (int)reinterpret_cast<PKT_ROOM_IN*>(packet)->Room_num << "번 방에 참가 실패\n";
		break;
	}
	case PKT_ID_SHOOT:
	{
		int room_num = search_client_in_room(clients_[id].socket_);
		if (room_num != -1)
		{
			using namespace std::chrono;

			if (reinterpret_cast<PKT_SHOOT*>(packet)->Player_Weapon == WEAPON_TYPE_GM_GUN)
			{
				int index;
				auto pkt_ce = rooms_[room_num].shoot(reinterpret_cast<PKT_SHOOT*>(packet)->ID,
					reinterpret_cast<PKT_SHOOT*>(packet)->BulletWorldMatrix,
					reinterpret_cast<PKT_SHOOT*>(packet)->Player_Weapon, 600.0f, &index);
				add_event(index, room_num, EVENT_TYPE_GM_GUN, high_resolution_clock::now() + 16ms);
				delete pkt_ce;
			}
			else if (reinterpret_cast<PKT_SHOOT*>(packet)->Player_Weapon == WEAPON_TYPE_BEAM_RIFLE)
			{
				int index;
				auto pkt_ce = rooms_[room_num].shoot(reinterpret_cast<PKT_SHOOT*>(packet)->ID,
					reinterpret_cast<PKT_SHOOT*>(packet)->BulletWorldMatrix,
					reinterpret_cast<PKT_SHOOT*>(packet)->Player_Weapon, 1000.0f, &index);
				add_event(index, room_num, EVENT_TYPE_BEAM_RIFLE, high_resolution_clock::now() + 16ms);
				delete pkt_ce;
			}
			else if (reinterpret_cast<PKT_SHOOT*>(packet)->Player_Weapon == WEAPON_TYPE_BEAM_SNIPER)
			{
				int index;
				auto pkt_ce = rooms_[room_num].shoot(reinterpret_cast<PKT_SHOOT*>(packet)->ID,
					reinterpret_cast<PKT_SHOOT*>(packet)->BulletWorldMatrix,
					reinterpret_cast<PKT_SHOOT*>(packet)->Player_Weapon, 2000.0f, &index);
				add_event(index, room_num, EVENT_TYPE_BEAM_SNIPER, high_resolution_clock::now() + 16ms);
				delete pkt_ce;
			}
			else
			{
				auto pkt_co = rooms_[room_num].shoot(reinterpret_cast<PKT_SHOOT*>(packet)->ID,
					reinterpret_cast<PKT_SHOOT*>(packet)->BulletWorldMatrix,
					reinterpret_cast<PKT_SHOOT*>(packet)->Player_Weapon);

				send_packet_to_room_player(room_num, (char*)pkt_co);
				add_event(pkt_co->Object_Index, room_num, EVENT_TYPE_OBJECT_MOVE, high_resolution_clock::now() + 16ms);
				delete pkt_co;
			}
		}
		break;
	}
	case PKT_ID_LOBBY_PLAYER_INFO:
	{
		int room_num = search_client_in_room(clients_[id].socket_);
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
		int room_num = search_client_in_room(clients_[id].socket_);
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
		int room_num = search_client_in_room(clients_[id].socket_);
		if (room_num != -1)
		{
			PKT_GAME_START pkt_gs;
			pkt_gs.map = rooms_[room_num].get_map();
			pkt_gs.PktID = PKT_ID_GAME_START;
			pkt_gs.PktSize = sizeof(PKT_GAME_START);
			send_packet_to_room_player(room_num, (char*)&pkt_gs);

			rooms_[room_num].start_game();

			using namespace std::chrono;
			add_event(room_num, room_num, EVENT_TYPE_ROOM_UPDATE, high_resolution_clock::now() + 16ms);
		}
		break;
	}
	case PKT_ID_LEAVE_ROOM:
	{
		int room_num = search_client_in_room(clients_[id].socket_);
		if (room_num != -1)
		{
			PKT_PLAYER_OUT packet;
			packet.id = rooms_[room_num].find_player_by_socket(clients_[id].socket_);
			packet.PktId = PKT_ID_PLAYER_OUT;
			packet.PktSize = sizeof(PKT_PLAYER_OUT);
			send_packet_to_room_player(room_num, (char*)&packet);

			rooms_[room_num].disconnect_client(clients_[id].socket_);
			clients_[id].in_room_ = false;

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
		int room_num = search_client_in_room(clients_[id].socket_);
		if (room_num != -1)
		{
			int player = rooms_[room_num].find_player_by_socket(clients_[id].socket_);
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
		//std::wcout << id << L"번 플레이어의 이름을 " << clients_[id].name_ << L"에서 " << pkt_cn->name << L"로 변경\n";
		lstrcpynW(clients_[id].name_, pkt_cn->name, MAX_NAME_LENGTH);
		break;
	}
	case PKT_ID_MOVE_TO_MAIN_LOBBY:
	{
		int room_num = search_client_in_room(clients_[id].socket_);
		if (room_num != -1)
		{
			rooms_[room_num].disconnect_client(clients_[id].socket_);
			clients_[id].in_room_ = false;

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
	case PKT_ID_LOG_OUT:
	{
		disconnect_client(id);
		break;
	}		
	case PKT_ID_LOG_IN:
	{
		PKT_LOG_IN* pkt_li = reinterpret_cast<PKT_LOG_IN*>(packet);
#ifdef WITH_DATA_BASE
		DB_PKT_SELECT_PLAYER pkt_sp;
		pkt_sp.PktId = DB_PKT_ID_SELECT_PLAYER;
		pkt_sp.PktSize = sizeof(DB_PKT_SELECT_PLAYER);
		lstrcpynW(pkt_sp.id, pkt_li->id, MAX_NAME_LENGTH);
		lstrcpynW(pkt_sp.pass, pkt_li->pass, MAX_NAME_LENGTH);
		std::wcout << pkt_li->id << L", " << pkt_li->pass << L"\n";
		send_packet_to_player(DBSERVER_KEY, (char*)&pkt_sp);
#endif
		break;
	}
	case PKT_ID_CREATE_ACCOUT:
	{
		PKT_CREATE_ACCOUNT* packet = reinterpret_cast<PKT_CREATE_ACCOUNT*>(packet);
#ifdef WITH_DATA_BASE
		DB_PKT_CREATE_ACCOUNT pkt_ca;
		pkt_ca.PktId = DB_PKT_ID_CREATE_ACCOUNT;
		pkt_ca.PktSize = sizeof(DB_PKT_CREATE_ACCOUNT);
		lstrcpynW(pkt_ca.id, packet->id, MAX_NAME_LENGTH);
		lstrcpynW(pkt_ca.pass, packet->pass, MAX_NAME_LENGTH);
		send_packet_to_player(DBSERVER_KEY, (char*)&pkt_ca);
#endif
		break;
	}
	case DB_PKT_ID_SELECT_PLAYER_RESULT:
	{
		DB_PKT_SELECT_PLAYER_RESULT* dbpkt_spr = 
			reinterpret_cast<DB_PKT_SELECT_PLAYER_RESULT*>(packet);
		if (dbpkt_spr->result == RESULT_FAIL)
			std::cout << "로그인 실패!\n";
		break;
	}
	default:
		std::wcout << L"정의되지 않은 패킷 도착 오류!! 패킷아이디 : " << (int)packet[1] <<"\n";
		break;
	}
}

void Framawork::send_packet_to_player(int id, char* packet)
{
	char *p = packet;
	Overlapped *ov = new Overlapped;
	ov->wsa_buffer_.len = p[0];
	ov->wsa_buffer_.buf = ov->packet_buffer_;
	ov->event_type_ = EVENT_TYPE_SEND;
	memcpy(ov->packet_buffer_, p, p[0]);
	ZeroMemory(&ov->overlapped_, sizeof(ov->overlapped_));
	int error = WSASend(clients_[id].socket_, &ov->wsa_buffer_, 1, 0, 0,
		&ov->overlapped_, NULL);
	if (0 != error) {
		int err_no = WSAGetLastError();
		if (err_no != WSA_IO_PENDING)
			error_display("WSASend in send_packet_to_player()  ", err_no);
	}
}

void Framawork::send_packet_to_all_player(char* packet)
{
	for (int i = 0; i < 1000; ++i)
	{
		if (!clients_[i].in_use_) continue;
		if (clients_[i].in_room_) continue;
		if (clients_[i].is_server_) continue;
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
