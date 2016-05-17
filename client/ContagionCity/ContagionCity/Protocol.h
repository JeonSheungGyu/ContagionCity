#pragma once
#define LOGIN_PORT	9011
#define GAME_PORT   9012

// Login <-> Client
#define CL_REQUEST_LOGIN		0
#define LC_PERMISION_LOGIN		1

// Login_Server -> Client
#define ID_LEN					20
#define PASSWORD_LEN			20


#pragma pack (push, 1)


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
#pragma pack (pop)