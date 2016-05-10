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
#include "ObjectStatus.h"
using namespace DirectX;


#define BUFSIZE 1024
#define MAX_USER 10
#define MAX_NPC 1000
#define RECTSIZE 40
#define INTERVAL 4
#define WORLDSIZE RECTSIZE * INTERVAL * 100
#define SECTOR_WIDTH 1000
#define SECTOR_HEIGHT 1000
#define VIEWRANGE RECTSIZE*8
#define MONSTER_DURATION 1000

#define OP_RECV  1
#define OP_SEND  2
#define OP_NPC_MOVE  3
#define OP_DB_EVENT 4

typedef struct // �����ǹ�������������üȭ.
{
	OVERLAPPED overlapped;
	WSABUF wsabuf;
	unsigned char iocp_buffer[BUFSIZE];
	int operation;

	//DB�ε�����
	DWORD db_type;
	ObjectStatus status;
	XMFLOAT3 pos;
	//DB�ε强������
	BOOL isSuccess;

} Overlap_ex;

//DB_Queue ���� DB_QUERY
typedef struct DB_QUERY
{
	DWORD ID;
	DWORD type;
	enum DB_TYPE{
		REQUEST_STATE = 1,
		REQUEST_UPDATE
	};
}DB_QUERY;

//EVENT
typedef struct EVENT {
	DWORD id;
	DWORD type;
	DWORD duration; // after
	std::chrono::system_clock::time_point startTime;

	bool operator < (const EVENT& e) const { return  startTime < e.startTime; }
}EVENT;