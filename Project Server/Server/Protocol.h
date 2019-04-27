#pragma once

typedef enum PKT_ID
{
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
	PKT_ID_SCORE
}PKT_ID;

typedef enum OBJECT_TYPE
{
	OBJECT_TYPE_PLAYER,
	OBJECT_TYPE_MACHINE_BULLET,
	OBJECT_TYPE_OBSTACLE,
	OBJECT_TYPE_ITEM_HEALING,
	OBJECT_TYPE_BZK_BULLET,
	OBJECT_TYPE_BEAM_BULLET,
	OBJECT_TYPE_ITEM_AMMO
}OBJECT_TYPE;

typedef enum ROBOT_TYPE
{
	ROBOT_TYPE_GM,
	ROBOT_TYPE_GUNDAM
}ROBOT_TYPE;

typedef enum SCENE_NAME
{
	SCENE_NAME_COLONY
}SCENE_NAME;

enum GAME_STATE
{
	GAME_STATE_GAME_START,
	GAME_STATE_GAME_OVER,
	GAME_STATE_LOAD_COMPLETE
};
typedef enum EFFECT_TYPE
{
	EFFECT_TYPE_HIT_FONT,
	EFFECT_TYPE_EXPLOSION,
	EFFECT_TYPE_HIT
}EFFECT_TYPE;

enum EFFECT_ANIMATION_TYPE
{
	EFFECT_ANIMATION_TYPE_ONE,
	EFFECT_ANIMATION_TYPE_LOOP
};
enum WEAPON_TYPE
{
	WEAPON_TYPE_MACHINE_GUN,
	WEAPON_TYPE_BAZOOKA,
	WEAPON_TYPE_BEAM_RIFLE
};

enum ANIMATION_TYPE
{
	ANIMATION_TYPE_IDLE,
	ANIMATION_TYPE_WALK_FORWARD,
	ANIMATION_TYPE_WALK_RIGHT,
	ANIMATION_TYPE_WALK_LEFT,
	ANIMATION_TYPE_WALK_BAKCWARD,
	ANIMATION_TYPE_JUMP,
	ANIMATION_TYPE_LANDING,
	ANIMATION_TYPE_DASH_FORWARD,
	ANIMATION_TYPE_DASH_LEFT,
	ANIMATION_TYPE_DASH_RIGHT,
	ANIMATION_TYPE_DASH_BACKWARD,
	ANIMATION_TYPE_GM_GUN_STAND
};

enum BULLET_TYPE
{
	BULLET_TYPE_MACHINE_GUN,
	BULLET_TYPE_BAZOOKA,
	BULLET_TYPE_BEAM_RIFLE
};

typedef struct PKT_PLAYER_INFO
{
	BYTE			PktSize;
	BYTE			PktId;
	BYTE			ID;
	XMFLOAT4X4		WorldMatrix;
	BOOL			IsShooting;
	WEAPON_TYPE		Player_Weapon;
	BOOL			isChangeWeapon;
	ANIMATION_TYPE	Player_Animation;
	BOOL			isChangeAnimation;
	int				State;
}PKT_PLAYER_INFO;

typedef struct PKT_PLAYER_LIFE
{
	char		PktSize;
	char		PktId;
	int			ID;
	DWORD		HP;
	DWORD		AMMO;
}PKT_PLAYER_LIFE;

typedef struct PKT_CREATE_OBJECT
{
	char		PktSize;
	char		PktId;
	OBJECT_TYPE	Object_Type;
	XMFLOAT4X4	WorldMatrix;
	int			Object_Index;
	ROBOT_TYPE	Robot_Type;
}PKT_CREATE_OBJECT;

typedef struct PKT_DELETE_OBJECT
{
	char		PktSize;
	char		PktId;
	int			Object_Index;
}PKT_DELETE_OBJECT;

struct PKT_TIME_INFO
{
	char		PktSize;
	char		PktId;
	float		elapsedtime;
};

struct PKT_UPDATE_OBJECT
{
	char		PktSize;
	char		PktId;
	int			Object_Index;
	XMFLOAT3	Object_Position;
};

struct PKT_CREATE_EFFECT
{
	char					PktSize;
	char					PktId;
	XMFLOAT3				xmf3Position;
	EFFECT_TYPE				efType;
	EFFECT_ANIMATION_TYPE	EftAnitType;
};

struct PKT_GAME_STATE
{
	char		PktSize;
	char		PktId;
	GAME_STATE	game_state;
	char		num_player;
};

typedef struct PKT_PLAYER_IN
{
	char		PktSize;
	char		PktId;
	int			id;
}PKT_PLAYER_IN, PKT_PLAYER_OUT, PKT_CLIENTID;

struct PKT_LOBBY_PLAYER_INFO
{
	char		PktSize;
	char		PktId;
	int			id;
	char		selected_robot;
};

typedef struct PKT_GAME_START
{
	char PktSize;
	char PktID;
}PKT_GAME_START, PKT_LOAD_COMPLETE, PKT_SEND_COMPLETE;

struct PKT_SHOOT
{
	BYTE			PktSize;
	BYTE			PktId;
	BYTE			ID;
	WEAPON_TYPE		Player_Weapon;
	XMFLOAT4X4		BulletWorldMatrix;
};

struct PKT_SCORE
{
	char PktSize;
	char PktId;
	char RedScore;
	char BlueScore;
};

#define MAX_NUM_OBJECT 2000

#define SERVERIP	"127.0.0.1"
#define SERVERPORT	9000
#define LOBBYSERVERIP	"127.0.0.1"
#define LOBBYSERVERPORT	9001