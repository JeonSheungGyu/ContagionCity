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

//���� ȯ��
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

//��������
enum
{
	CC_CircleAround = 0,
	CC_CircleFront,
	CC_Eraser,
	CC_PointCircle,
};

//ObjectHandler���� ����� �Լ��� ���� �Լ������� ����ü
struct FuncProcess
{
	using FuncType = void(*)(char*);
	FuncType Func;
	FuncProcess() { Func = nullptr; }
};

