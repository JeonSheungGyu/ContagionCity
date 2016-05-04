#pragma once

//프로토콜

#define CS_UP    1
#define CS_DOWN  2
#define CS_LEFT  3
#define CS_RIGHT    4
#define CS_CHAT		5

#define SC_POS           1
#define SC_PUT_PLAYER    2
#define SC_REMOVE_PLAYER 3
#define SC_CHAT			4

#pragma pack (push, 1)

struct cs_packet_dir {
	BYTE size;
	BYTE type;
};

struct sc_packet_pos {
	BYTE size;
	BYTE type;
	WORD id;
	DWORD x;
	DWORD y;
};

struct sc_packet_put_player {
	BYTE size;
	BYTE type;
	WORD id;
	DWORD x;
	DWORD y;
};
struct sc_packet_remove_player {
	BYTE size;
	BYTE type;
	WORD id;
};


#pragma pack (pop)