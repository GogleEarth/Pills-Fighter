#pragma once
#include "Protocol.h"
//#define MAX_NAME_LENGTH 10

//#define SERVERIP	"121.169.212.228"
//#define SERVERPORT	9000
#define DBSERVERIP	"127.0.0.1"
#define DBSERVERPORT	9001
////////////////////////////////////////

enum PKT_ID
{
	PKT_ID_SELECT_PLAYER,
	PKT_ID_SELECT_PLAYER_RESULT,
	PKT_ID_UPDATE_PLAYER,
	PKT_ID_SELECT_PLAYER_RESULT,
	PKT_ID_CREATE_ACCOUNT,
	PKT_ID_CREATE_ACCOUNT_RESULT
};

enum RESULT
{
	RESULT_FAIL,
	RESULT_FAIL_ID_OVERLAP,
	RESULT_FAIL_WRONG_ID,
	RESULT_FAIL_WRONG_PASS,
	RESULT_SUCCESS
};
#pragma pack(push, 1)

/////////////////////////////////////////////////////////// Packet

typedef struct PKT_SELECT_PLAYER
{
	char		PktSize;
	char		PktId;
	wchar_t		id[MAX_NAME_LENGTH];
	wchar_t		pass[MAX_NAME_LENGTH];
}PKT_CREATE_ACCOUNT;

struct PKT_SELECT_PLAYER_RESULT
{
	char		PktSize;
	char		PktId;
	RESULT		result;
	wchar_t		name[MAX_NAME_LENGTH];
	int			win;
	int			lose;
};

struct PKT_UPDATE_PLAYER
{
	char		PktSize;
	char		PktId;
	wchar_t		id[MAX_NAME_LENGTH];
	wchar_t		name[MAX_NAME_LENGTH];
	int			win;
	int			lose;
};

struct PKT_UPDATE_PLAYER_RESULT
{
	char		PktSize;
	char		PktId;
	RESULT		result;
	wchar_t		name[MAX_NAME_LENGTH];
	int			win;
	int			lose;
};

struct PKT_CREATE_ACCOUNT_RESULT
{
	char		PktSize;
	char		PKTId;
	RESULT		result;
	wchar_t		error[MAX_NAME_LENGTH];
};
///////////////////////////////////////////////////////////

#pragma pack(pop)
