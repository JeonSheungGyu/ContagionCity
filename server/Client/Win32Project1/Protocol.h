#pragma once
#include "stdafx.h"
#define LOGIN_PORT	9011
#define GAME_PORT   9012

// Login <-> Client
#define CL_REQUEST_LOGIN		0
#define LC_PERMISION_LOGIN		1


//클라이언트->서버
#define CS_MOVE_OBJECT			0
#define CS_COMBAT_OBJECT		1
#define CS_CHAT					2
#define CS_REQUEST_LOGIN		3
#define CS_DB_UPDATE			4

//서버->클라이언트
#define SC_LOGIN				0
#define SC_MOVE_OBJECT          1
#define SC_PUT_OBJECT			2
#define SC_REMOVE_OBJECT		3
#define SC_CHAT					4

#define SC_MONSTER_CHASE		5
#define SC_MONSTER_ATTACK		6
#define SC_MONSTER_DIE			7

#define SC_COMBAT_OBJECT		8

// Login_Server -> Client
#define ID_LEN					20
#define PASSWORD_LEN			20


#pragma pack (push, 1)
// Server -> Client
struct sc_packet_login
{
	BYTE size;
	BYTE type;
	WORD id;
	FLOAT x;
	FLOAT y;
	FLOAT z;
	FLOAT speed;
	WORD  lv;
	WORD hp;
	WORD ap;
	WORD damage;
	WORD defense;
	WORD exp;
	WORD request_exp;
	BYTE ElementType;
};
struct sc_packet_put_object
{
	BYTE size;
	BYTE type;
	WORD id;
	FLOAT x;
	FLOAT y;
	FLOAT z;
	FLOAT speed;
	WORD lv;
	int hp;
	int ap;
	WORD damage;
	WORD defense;
	WORD exp;
	WORD request_exp;
	BYTE ElementType;
};
struct sc_packet_remove_object
{
	BYTE size;
	BYTE type;
	WORD id;
};
struct sc_packet_move_object
{
	BYTE size;
	BYTE type;
	WORD id;
	BYTE tf;
	FLOAT tx;
	FLOAT ty;
	FLOAT tz;
};


struct sc_packet_monster_attack
{
	BYTE size;
	BYTE type;
	WORD mon_id;
	WORD target_id;
	WORD damage;
	FLOAT x;
	FLOAT y;
	FLOAT z;
};

struct sc_packet_monster_chase
{
	BYTE size;
	BYTE type;
	WORD id;
	FLOAT tx;
	FLOAT ty;
	FLOAT tz;
	FLOAT x;
	FLOAT y;
	FLOAT z;
	FLOAT dist;
};

struct sc_packet_monster_die
{
	BYTE size;
	BYTE type;
	WORD mon_id;
	WORD player_id;
	WORD EXP;
};

struct sc_packet_combat
{
	BYTE size;
	BYTE type;
	//시전자
	WORD id;
	BYTE combatCollision;
	std::pair<WORD, int> InfoList[10];
	BYTE  ListSize;
	FLOAT x;
	FLOAT z;
};

//Client -> Server

struct cs_packet_object_move
{
	BYTE size;
	BYTE type;
	WORD id;
	FLOAT tx;
	FLOAT ty;
	FLOAT tz;
	FLOAT x;
	FLOAT y;
	FLOAT z;
	FLOAT dx;
	FLOAT dy;
	FLOAT dz;
	FLOAT dist;
	BYTE is_whirl;
};


struct cs_packet_combat
{
	BYTE size;
	BYTE type;
	//시전자
	WORD id;
	BYTE combatCollision;
	FLOAT x;
	FLOAT z;
};

struct cs_packet_request_login
{
	BYTE size;
	BYTE type;
	char id[ID_LEN];
};

struct cs_packet_db_update
{
	BYTE size;
	BYTE type;
	WORD id;
};
//Client <-> LoginServer
struct cl_packet_request_login
{
	BYTE size;
	BYTE type;
	char id[ID_LEN];
	char password[PASSWORD_LEN];
};

struct lc_packet_permit_login
{
	BYTE size;
	BYTE type;
	BYTE permit_check;
};


//기타

struct CombatData
{
	BYTE size;
	BYTE type;
	//누가공격을 시전하였는지
	WORD id;
	BYTE combatCollision;
	std::pair<WORD, int> InfoList[10];
	BYTE  ListSize;
	FLOAT x;
	FLOAT z;
};
#pragma pack (pop)