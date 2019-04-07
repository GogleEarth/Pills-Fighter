#pragma once
#include "pch.h"


#define MAX_BUFFER	13

struct PlayerInfo
{
	int Player_num;
	char Player_id[MAX_BUFFER];
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
	char Player_id[MAX_BUFFER];
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