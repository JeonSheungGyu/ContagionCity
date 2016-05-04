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

typedef struct // 소켓의버퍼정보를구조체화.
{
	OVERLAPPED overlapped;
	WSABUF wsabuf;
	unsigned char iocp_buffer[BUFSIZE];
	int operation;
} Overlap_ex;

