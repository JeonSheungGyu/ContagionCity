#pragma once

#include <winsock2.h>
#include <vector>
#include <algorithm>
#include <DirectXMath.h>
#include <DirectXCollision.h>
#include <atomic>
#include <iostream>
#include <chrono>
#include <thread>
#include "resource.h"

#define UPDATE_TIME 100
#define DB_UPDATE_TIME 60*1000
#define BUFSIZE 1024
#define RECTSIZE 40
#define INTERVAL 4
#define MAX_USER 10
#define MAX_NPC 10000
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
