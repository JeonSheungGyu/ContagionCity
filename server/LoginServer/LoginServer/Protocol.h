#pragma once

#include <WinSock2.h>

#define LOGIN_PORT	9011

#define MAX_BUF_SIZE	512

// 클라이언트 및 서버와 주고받을 패킷사이즈는 항상 같음
#define STATIC_SEND_PACKET_SIZE	16
#define STATIC_RECV_PACKET_SIZE	16

#define NUM_OF_PACKET_TYPE	10

#define MAX_USER	100

#define MAX_CS_PACKET_SIZE  48
#define MAX_SC_PACKET_SIZE	48
#define MAX_PACKET_SIZE		255
// 계정정보
#define ID_LEN			20
#define PASSWORD_LEN	20

// Client && DBServer -> Login
#define CL_REQUEST_LOGIN	0
#define DL_PERMISION_LOGIN	1

// Login -> DBServer
#define LD_REQUEST_LOGIN	0

// Login -> Client
#define LC_PERMISION_LOGIN	8


#pragma pack(push,1)

// DB_Server -> Login_Server
struct dl_packet_permit_login
{
	BYTE size;
	BYTE type;
	BYTE check;
	WORD login_id;
};

// Login_Server -> DB_Server
struct ld_packet_request_login
{
	BYTE size;
	BYTE type;
	char id[ID_LEN];
	char password[PASSWORD_LEN];
};

// Client -> Login_Server
struct cl_packet_request_login
{
	BYTE size;
	BYTE type;
	char id[ID_LEN];
	char password[PASSWORD_LEN];
};

// Login_Server -> Client
struct lc_packet_permit_login
{
	BYTE size;
	BYTE type;
	BYTE permit_check;
};
#pragma pack(pop)


void err_display(char *msg, int err_no);