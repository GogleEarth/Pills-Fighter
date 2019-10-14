#include "pch.h"
#include "Framework.h"
#include "DBServer_Protocol.h"

Framework::Framework()
{
}


Framework::~Framework()
{
	SQLCancel(hstmt_);
	SQLFreeHandle(SQL_HANDLE_STMT, hstmt_);
	SQLDisconnect(hdbc_);
	SQLFreeHandle(SQL_HANDLE_DBC, hdbc_);
	SQLFreeHandle(SQL_HANDLE_ENV, henv_);
	closesocket(listen_socket_);
	closesocket(main_socket_);
	WSACleanup();
}

bool Framework::init()
{
	hstmt_ = 0;
	SQLRETURN retcode;

	retcode = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv_);

	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
	{
		retcode = SQLSetEnvAttr(henv_, SQL_ATTR_ODBC_VERSION, (SQLPOINTER*)SQL_OV_ODBC3, 0);

		if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
		{
			retcode = SQLAllocHandle(SQL_HANDLE_DBC, henv_, &hdbc_);

			if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
			{
				SQLSetConnectAttr(hdbc_, SQL_LOGIN_TIMEOUT, (SQLPOINTER)5, 0);

				retcode = SQLConnect(hdbc_, (SQLWCHAR*)L"PillsFighter_DB", SQL_NTS, (SQLWCHAR*)NULL, 0, NULL, 0);

				if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
				{
					retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc_, &hstmt_);

					std::cout << "DB connect OK!\n";
					return true;
				}
				else
					std::cout << "DB connect Fail!\n";
			}
			else
				std::cout << "hdbc AllocHandle Fail!\n";
		}
		else
			std::cout << "henv SetEnvAttr Fail!\n";
	}
	else
		std::cout << "henv AllocHandle Fail!\n";
	
	return true;
}

bool Framework::accpet_process()
{
	WSADATA wsadata;
	if (WSAStartup(MAKEWORD(2, 2), &wsadata) != 0)
	{
		std::cout << "Error - Can not load 'winsock.dll' file\n";
		return false;
	}

	listen_socket_ = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (listen_socket_ == INVALID_SOCKET)
	{
		std::cout << "Error - Invalid socket\n";
		return false;
	}

	SOCKADDR_IN serverAddr;
	memset(&serverAddr, 0, sizeof(SOCKADDR_IN));
	serverAddr.sin_family = PF_INET;
	serverAddr.sin_port = htons(DBSERVERPORT);
	serverAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);

	if (::bind(listen_socket_, (struct sockaddr*)&serverAddr,
		sizeof(SOCKADDR_IN)) == SOCKET_ERROR)
	{
		std::cout << "Error - Fail bind\n";
		closesocket(listen_socket_);
		WSACleanup();
		return false;
	}

	if (listen(listen_socket_, 5) == SOCKET_ERROR)
	{
		std::cout << "Error - Fail listen\n";
		closesocket(listen_socket_);
		WSACleanup();
		return false;
	}

	SOCKADDR_IN client_addr;
	int addr_len = sizeof(SOCKADDR_IN);
	memset(&client_addr, 0, addr_len);

	main_socket_ = accept(listen_socket_, (struct sockaddr *)&client_addr, &addr_len);
	if (main_socket_ == INVALID_SOCKET)
	{
		std::cout << "Error - Accept Failure\n";
		return false;
	}
	recvn();

	return true;
}

void Framework::process_packet()
{
	char type = recv_buffer_[1];

	switch (type)
	{
	case DB_PKT_ID_SELECT_PLAYER:
	{
		DB_PKT_SELECT_PLAYER* packet = (DB_PKT_SELECT_PLAYER*)recv_buffer_;

		SQLINTEGER win, lose;
		SQLWCHAR name[MAX_NAME_LENGTH];
		SQLLEN cbname = 0, cbwin = 0, cblose = 0;

		std::wstring query = L"EXEC select_player ";
		query += packet->id;
		query += L", ";
		query += packet->pass;

		DB_PKT_SELECT_PLAYER_RESULT pkt_spr;
		pkt_spr.PktSize = sizeof(DB_PKT_SELECT_PLAYER_RESULT);
		pkt_spr.PktId = DB_PKT_ID_SELECT_PLAYER_RESULT;

		SQLSMALLINT retcode = SQLExecDirect(hstmt_, (SQLWCHAR*)query.c_str(), SQL_NTS);
		if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
		{
			retcode = SQLBindCol(hstmt_, 1, SQL_C_WCHAR, name, 10, &cbname);
			retcode = SQLBindCol(hstmt_, 2, SQL_C_LONG, &win, 10, &cbwin);
			retcode = SQLBindCol(hstmt_, 3, SQL_C_LONG, &lose, 10, &cblose);

			for (int i = 0; ; ++i)
			{
				retcode = SQLFetch(hstmt_);
				if (retcode == SQL_ERROR || retcode == SQL_SUCCESS_WITH_INFO)
					show_error(hstmt_, SQL_HANDLE_STMT, retcode);
				if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
					wprintf(L"%d : %s %d %d\n", i + 1, name, win, lose);
				else
					break;
			}

			pkt_spr.result = RESULT_SUCCESS;
			pkt_spr.win = win;
			pkt_spr.lose = lose;
			lstrcpynW(pkt_spr.name, name, MAX_NAME_LENGTH);

			if (send(main_socket_, (char*)&pkt_spr, sizeof(DB_PKT_SELECT_PLAYER_RESULT), 0) == SOCKET_ERROR)
				std::cout << "Send Error!\n";
		}
		else
		{
			show_error(hstmt_, SQL_HANDLE_STMT, retcode);

			pkt_spr.result = RESULT_FAIL;

			if (send(main_socket_, (char*)&pkt_spr, sizeof(DB_PKT_SELECT_PLAYER_RESULT), 0) == SOCKET_ERROR)
				std::cout << "Send Error!\n";
		}
		break;
	}
	case DB_PKT_ID_UPDATE_PLAYER:
	{
		DB_PKT_UPDATE_PLAYER* packet = (DB_PKT_UPDATE_PLAYER*)recv_buffer_;

		SQLINTEGER win, lose;
		SQLWCHAR name[MAX_NAME_LENGTH];
		SQLLEN cbname = 0, cbwin = 0, cblose = 0;

		std::wstring query = L"EXEC update_player ";
		query += packet->id;
		query += L", ";
		query += packet->name;
		query += L", ";
		query += packet->win;
		query += L", ";
		query += packet->lose;

		DB_PKT_UPDATE_PLAYER_RESULT pkt_spr;
		pkt_spr.PktSize = sizeof(DB_PKT_UPDATE_PLAYER_RESULT);
		pkt_spr.PktId = DB_PKT_ID_UPDATE_PLAYER_RESULT;

		SQLSMALLINT retcode = SQLExecDirect(hstmt_, (SQLWCHAR*)query.c_str(), SQL_NTS);
		if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
		{
			retcode = SQLBindCol(hstmt_, 1, SQL_C_WCHAR, name, 10, &cbname);
			retcode = SQLBindCol(hstmt_, 2, SQL_C_LONG, &win, 10, &cbwin);
			retcode = SQLBindCol(hstmt_, 3, SQL_C_LONG, &lose, 10, &cblose);

			for (int i = 0; ; ++i)
			{
				retcode = SQLFetch(hstmt_);
				if (retcode == SQL_ERROR || retcode == SQL_SUCCESS_WITH_INFO)
					show_error(hstmt_, SQL_HANDLE_STMT, retcode);
				if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
					wprintf(L"%d : %s %d %d\n", i + 1, name, win, lose);
				else
					break;
			}

			pkt_spr.result = RESULT_SUCCESS;
			pkt_spr.win = win;
			pkt_spr.lose = lose;
			lstrcpynW(pkt_spr.name, name, MAX_NAME_LENGTH);

			if (send(main_socket_, (char*)&pkt_spr, sizeof(DB_PKT_UPDATE_PLAYER_RESULT), 0) == SOCKET_ERROR)
				std::cout << "Send Error!\n";
		}
		else
		{
			show_error(hstmt_, SQL_HANDLE_STMT, retcode);

			pkt_spr.result = RESULT_FAIL;

			if (send(main_socket_, (char*)&pkt_spr, sizeof(DB_PKT_UPDATE_PLAYER_RESULT), 0) == SOCKET_ERROR)
				std::cout << "Send Error!\n";
		}
		break;
	}
	case DB_PKT_ID_CREATE_ACCOUNT:
	{
		DB_PKT_CREATE_ACCOUNT* packet = (DB_PKT_CREATE_ACCOUNT*)recv_buffer_;

		SQLINTEGER win, lose;
		SQLWCHAR name[MAX_NAME_LENGTH];
		SQLLEN cbname = 0, cbwin = 0, cblose = 0;

		std::wstring query = L"EXEC select_player ";
		query += packet->id;
		query += L", ";
		query += packet->pass;

		DB_PKT_CREATE_ACCOUNT_RESULT pkt_spr;
		pkt_spr.PktSize = sizeof(DB_PKT_CREATE_ACCOUNT_RESULT);
		pkt_spr.PktId = DB_PKT_ID_CREATE_ACCOUNT_RESULT;

		SQLSMALLINT retcode = SQLExecDirect(hstmt_, (SQLWCHAR*)query.c_str(), SQL_NTS);
		if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
		{
			retcode = SQLBindCol(hstmt_, 1, SQL_C_WCHAR, name, 10, &cbname);
			retcode = SQLBindCol(hstmt_, 2, SQL_C_LONG, &win, 10, &cbwin);
			retcode = SQLBindCol(hstmt_, 3, SQL_C_LONG, &lose, 10, &cblose);

			for (int i = 0; ; ++i)
			{
				retcode = SQLFetch(hstmt_);
				if (retcode == SQL_ERROR || retcode == SQL_SUCCESS_WITH_INFO)
					show_error(hstmt_, SQL_HANDLE_STMT, retcode);
				if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
					wprintf(L"%d : %s %d %d\n", i + 1, name, win, lose);
				else
					break;
			}

			pkt_spr.result = RESULT_FAIL_ID_OVERLAP;

			if (send(main_socket_, (char*)&pkt_spr, sizeof(DB_PKT_SELECT_PLAYER_RESULT), 0) == SOCKET_ERROR)
				std::cout << "Send Error!\n";
		}
		else
		{
			std::wstring query2 = L"EXEC create_account ";
			query2 += packet->id;
			query2 += L", ";
			query2 += packet->pass;

			SQLSMALLINT retcode = SQLExecDirect(hstmt_, (SQLWCHAR*)query2.c_str(), SQL_NTS);
			if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
			{
				retcode = SQLBindCol(hstmt_, 1, SQL_C_CHAR, name, 10, &cbname);
				retcode = SQLBindCol(hstmt_, 2, SQL_C_LONG, &win, 10, &cbwin);
				retcode = SQLBindCol(hstmt_, 3, SQL_C_LONG, &lose, 10, &cblose);

				for (int i = 0; ; ++i)
				{
					retcode = SQLFetch(hstmt_);
					if (retcode == SQL_ERROR || retcode == SQL_SUCCESS_WITH_INFO)
						show_error(hstmt_, SQL_HANDLE_STMT, retcode);
					if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
						wprintf(L"%d : %s %d %d\n", i + 1, name, win, lose);
					else
						break;
				}

				pkt_spr.result = RESULT_SUCCESS;

				if (send(main_socket_, (char*)&pkt_spr, sizeof(DB_PKT_SELECT_PLAYER_RESULT), 0) == SOCKET_ERROR)
					std::cout << "Send Error!\n";
			}
			else
			{
				show_error(hstmt_, SQL_HANDLE_STMT, retcode);
				pkt_spr.result = RESULT_FAIL;

				if (send(main_socket_, (char*)&pkt_spr, sizeof(DB_PKT_SELECT_PLAYER_RESULT), 0) == SOCKET_ERROR)
					std::cout << "Send Error!\n";
			}
		}
		break;
	}
	default:
	{
		std::cout << "Unknown Packet Type!\n";
		break;
	}
	}
}

void Framework::recvn()
{
	int rest = recv(main_socket_, recv_buffer_, MAX_BUFFER, 0);

	char *ptr = recv_buffer_;
	int packetsize = 0;

	if (prev_size_ > 0) packetsize = recv_buffer_[0];

	while (rest > 0)
	{
		if (packetsize == 0) packetsize = ptr[0];
		int required = packetsize - prev_size_;

		if (required <= rest)
		{
			memcpy(recv_buffer_ + prev_size_, ptr, required);

			process_packet();

			rest -= required;
			ptr += required;

			packetsize = 0;
			prev_size_ = 0;

			recvn();
		}
		else
		{
			memcpy(recv_buffer_ + prev_size_, ptr, rest);
			rest = 0;
			prev_size_ += rest;
		}
	}
}
