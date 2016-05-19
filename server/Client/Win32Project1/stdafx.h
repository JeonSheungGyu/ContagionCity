#pragma once

#include <winsock2.h>
#include <vector>
#include <algorithm>
#include <DirectXMath.h>
#include <DirectXCollision.h>
#include <atomic>
#include <iostream>
#include <chrono>
#include "resource.h"

//서버 환경
#define SERVERIP			"127.0.0.1"
#define LOGIN_SERVER_PORT	6026
#define SERVERPORT			6027

#define UPDATE_TIME 100
#define BUFSIZE 1024
#define RECTSIZE 40
#define INTERVAL 4
#define MAX_USER 10
#define MAX_NPC 300000
#define VIEWRANGE RECTSIZE*8
#define PACKET_TYPE			10

//공격종류
enum
{
	CC_CircleAround = 0,
	CC_CircleFront,
	CC_Eraser,
	CC_PointCircle,
};

//ObjectHandler에서 사용할 함수를 담을 함수포인터 구조체
struct FuncProcess
{
	using FuncType = void(*)(char*);
	FuncType Func;
	FuncProcess() { Func = nullptr; }
};

