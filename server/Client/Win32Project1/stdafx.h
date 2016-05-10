#pragma once

#include <winsock2.h>
#include <vector>
#include <algorithm>
#include <DirectXMath.h>
#include <DirectXCollision.h>
#include <atomic>
#include "resource.h"

//서버 환경
#define SERVERIP			"127.0.0.1"
#define LOGIN_SERVER_PORT	6026
#define SERVERPORT			6027


#define BUFSIZE 1024
#define RECTSIZE 40
#define INTERVAL 4
#define MAX_USER 10
#define MAX_NPC 1000