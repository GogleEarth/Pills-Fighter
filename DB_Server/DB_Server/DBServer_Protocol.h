#pragma once

#define MAX_NAME_LENGTH 10
#define DBSERVERIP	"127.0.0.1"
#define DBSERVERPORT	8000
////////////////////////////////////////

enum DB_PKT_ID
{
	DB_PKT_ID_SELECT_PLAYER = 43,
	DB_PKT_ID_SELECT_PLAYER_RESULT,
	DB_PKT_ID_UPDATE_PLAYER,
	DB_PKT_ID_UPDATE_PLAYER_RESULT,
	DB_PKT_ID_CREATE_ACCOUNT,
	DB_PKT_ID_CREATE_ACCOUNT_RESULT
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

typedef struct DB_PKT_SELECT_PLAYER
{
	char		PktSize;
	char		PktId;
	wchar_t		id[MAX_NAME_LENGTH];
	wchar_t		pass[MAX_NAME_LENGTH];
}DB_PKT_CREATE_ACCOUNT;

struct DB_PKT_SELECT_PLAYER_RESULT
{
	char		PktSize;
	char		PktId;
	RESULT		result;
	wchar_t		name[MAX_NAME_LENGTH];
	int			win;
	int			lose;
};

struct DB_PKT_UPDATE_PLAYER
{
	char		PktSize;
	char		PktId;
	wchar_t		id[MAX_NAME_LENGTH];
	wchar_t		name[MAX_NAME_LENGTH];
	int			win;
	int			lose;
};

struct DB_PKT_UPDATE_PLAYER_RESULT
{
	char		PktSize;
	char		PktId;
	RESULT		result;
	wchar_t		name[MAX_NAME_LENGTH];
	int			win;
	int			lose;
};

struct DB_PKT_CREATE_ACCOUNT_RESULT
{
	char		PktSize;
	char		PktId;
	RESULT		result;
};
///////////////////////////////////////////////////////////

#pragma pack(pop)
