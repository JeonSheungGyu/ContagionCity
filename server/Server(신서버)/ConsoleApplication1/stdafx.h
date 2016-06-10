#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#include <process.h>
#include <vector>
#include <set>
#include <queue>
#include <algorithm>
#include <Windows.h>
#include <assert.h>
#include <math.h>
#include <random>
#include <chrono>
#include <thread>
#include <exception>
#include <sql.h>
#include <sqlext.h>
#include <tbb/concurrent_queue.h>
#include <DirectXCollision.h>
#include <DirectXMath.h>
#include <mutex>
#include "ObjectStatus.h"

#define BUFSIZE 1024

#define MAX_USER 10
#define MAX_NPC 100

#define RECTSIZE 40
#define INTERVAL 4
#define SECTOR_WIDTH 1000
#define SECTOR_HEIGHT 1000
#define VIEWRANGE RECTSIZE*8
#define MONSTER_VIEWRANGE RECTSIZE*8

//operation
#define OP_RECV			1
#define OP_SEND			2
#define OP_NPC_MOVE		3
#define OP_DB_EVENT		4

using namespace DirectX;

typedef struct // 소켓의버퍼정보를구조체화.
{
	OVERLAPPED overlapped;
	WSABUF wsabuf;
	unsigned char iocp_buffer[BUFSIZE];
	int operation;
	//DB_EVENT duration 정보
	DWORD duration; // after
	//DB로드정보
	DWORD db_type;
	ObjectStatus status;
	XMFLOAT3 pos;
	//DB로드성공여부
	BOOL isSuccess;

} Overlap_ex;

//DB_Queue 에들어갈 DB_QUERY
typedef struct DB_QUERY
{
	DWORD ID;
	DWORD type;
	enum DB_TYPE{
		REQUEST_STATE = 1,
		REQUEST_UPDATE,
		REQUEST_UPDATE_AND_END
	};
}DB_QUERY;

//EVENT
typedef struct EVENT {
	DWORD id;
	DWORD type;
	DWORD duration; // after
	clock_t startTime;
	clock_t endTime;

	bool operator < (const EVENT& e) const { return  e.endTime < endTime; }
}EVENT;

//장소
//Zone 1000단위로 끊어줘야 영역이 없는 곳이 안생긴다.
enum Stages { STAGE_VILLIGE = 0, STAGE_1, STAGE_2, STAGE_3, STAGE_LOGIN };
const XMFLOAT3 VILLIGE_POS = XMFLOAT3(0, 0, 0);
const int VILLIGE_WIDTH = RECTSIZE * INTERVAL * 100;
const int VILLIGE_HEIGHT = RECTSIZE * INTERVAL * 100;
const XMFLOAT3 STAGE1_POS = XMFLOAT3(-2500, 0, -2500);
const int STAGE1_WIDTH = 5000;
const int STAGE1_HEIGHT = 5000;


//유저초기위치
const XMFLOAT3 START_POS = XMFLOAT3(0, 0, 0);
const WORD START_STAGE = Stages::STAGE_VILLIGE;