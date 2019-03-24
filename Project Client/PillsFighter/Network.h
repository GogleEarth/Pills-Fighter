#pragma once

//#define ON_NETWORKING

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
	PKT_ID_PLAYER_IN
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

typedef enum EFFECT_TYPE
{
	EFFECT_TYPE_DEFAULT,
	EFFECT_TYPE_SPRITE_ONE,
	EFFECT_TYPE_SPRITE_LOOP
}EFFECT_TYPE;

typedef enum SCENE_NAME
{
	SCENE_NAME_COLONY
}SCENE_NAME;

typedef enum GAME_STATE
{
	GAME_STATE_START,
	GAME_STATE_END
}GAME_STATE;

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

typedef struct PKT_PLAYER_INFO
{
	int				ID;
	XMFLOAT4X4		WorldMatrix;
	BOOL			IsShooting;
	XMFLOAT4X4		BulletWorldMatrix;
	WEAPON_TYPE		Player_Weapon;
	BOOL			isChangeWeapon;
	ANIMATION_TYPE	Player_Animation;
	BOOL			isChangeAnimation;
}PKT_PLAYER_INFO;

typedef struct PKT_PLAYER_LIFE
{
	int		ID;
	DWORD	HP;
	DWORD	AMMO;
}PKT_PLAYER_LIFE;

typedef struct PKT_CREATE_OBJECT
{
	OBJECT_TYPE	Object_Type;
	XMFLOAT4X4	WorldMatrix;
	int			Object_Index;
}PKT_CREATE_OBJECT;

typedef struct PKT_DELETE_OBJECT
{
	int Object_Index;
}PKT_DELETE_OBJECT;

typedef struct PKT_TIME_INFO
{
	float ElapsedTime;
}PKT_TIME_INFO;

typedef struct PKT_UPDATE_OBJECT
{
	int Object_Index;
	XMFLOAT3 Object_Position;
}PKT_UPDATE_OBJECT;

typedef struct PKT_CREATE_EFFECT
{
	XMFLOAT3 xmf3Position;
	EFFECT_TYPE nEffectType;
}PKT_CREATE_EFFECT;

struct PKT_GAME_STATE
{
	GAME_STATE game_state;
	char num_player;
};

struct PKT_PLAYER_IN
{
	int id;
};

typedef SCENE_NAME SCENEINFO;
typedef unsigned int CLIENTID;

#define MAX_NUM_OBJECT 2000
