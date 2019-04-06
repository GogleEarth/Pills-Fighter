﻿// 시작을 위한 팁: 
//   1. [솔루션 탐색기] 창을 사용하여 파일을 추가/관리합니다.
//   2. [팀 탐색기] 창을 사용하여 소스 제어에 연결합니다.
//   3. [출력] 창을 사용하여 빌드 출력 및 기타 메시지를 확인합니다.
//   4. [오류 목록] 창을 사용하여 오류를 봅니다.
//   5. [프로젝트] > [새 항목 추가]로 이동하여 새 코드 파일을 만들거나, [프로젝트] > [기존 항목 추가]로 이동하여 기존 코드 파일을 프로젝트에 추가합니다.
//   6. 나중에 이 프로젝트를 다시 열려면 [파일] > [열기] > [프로젝트]로 이동하고 .sln 파일을 선택합니다.

#pragma once

#pragma comment(lib, "ws2_32")
#include <WinSock2.h>
#include <iostream>
#include <vector>
#include <thread>
#include <mutex>

#define MAX_USER	100
#define SERVERIP	"127.0.0.1"
#define SERVERPORT	9001
#define FLAG		FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM
#define LANG		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT)
#define WS22		MAKEWORD(2,2)
#define MAX_BUFFER	1024

// TODO: 여기에 미리 컴파일하려는 헤더 추가

struct PlayerInfo
{
	int Player_num;
	char* player_id;
	SOCKET client_sock;
	bool in_room = false;
};

typedef struct RoomInfo
{
	char room_num;
	char Player_num;
}RoomInfo, 
RoobyPacketCreateRoom, 
RoobyPacketDestroyRoom,
RoobyPacketRoomInfo;

enum RoobyPacketType
{
	RoobyPacketTypeLogIn,
	RoobyPacketTypeLogOut,
	RoobyPacketTypeCreateRoom,
	RoobyPacketTypeDestroyRoom,
	RoobyPacketTypeInvitePlayer,
	RoobyPacketTypeRoomIn,
	RoobyPacketTypeRoomOut,
	RoobyPacketTypeRoomInfo
};

typedef struct RoobyPacketLogIn
{
	int Player_num;
	char* Player_id;
}RoobyPacketLogIn,
RoobyPacketLogOut;

struct RoobyPacketInvitePlayer
{
	int fromPlayer_num;
	int toPlayer_num;
};

typedef struct RoobyPacketRoomIn
{
	int Player_num;
	char Room_num;
}RoobyPacketRoomIn, 
RoobyPacketRoomOut;