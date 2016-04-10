// stdafx.h : 자주 사용하지만 자주 변경되지는 않는
// 표준 시스템 포함 파일 및 프로젝트 관련 포함 파일이 
// 들어 있는 포함 파일입니다.
//

#pragma once

#define MAX_USER 100
#define DEFAULT_PORT 1820

#include "../CoreLib/Global.h"

typedef struct _USER
{
	WCHAR szUserID[32];
	WCHAR szUserName[32];
	DWORD dwAge;
	BYTE cSex;
	WCHAR szAddress[32];
} USER;

typedef struct _COMPUTER
{
	WCHAR szComputerName[32];
	WCHAR szIPAddress[32];
	BYTE cCPUType;
	DWORD dwRam;
	DWORD dwHDD;
} COMPUTER;

typedef struct _PROGRAM
{
	WCHAR szProgramName[32];
	DWORD dwCost;
	WCHAR szComment[32];
} PROGRAM;

#include "../CoreLib/CriticalSection.h"
#include "../CoreLib/MultiThreadSync.h"
#include "../CoreLib/MemoryPool.h"
#include "../CoreLib/CircularQueue.h"
#include "../CoreLib/NetworkSession.h"
#include "../CoreLib/PacketSession.h"
#include "../CoreLib/Iocp.h"
#include "../CoreLib/Stream.h"
#include "../CoreLib/MiniDump.h"
#include "../CoreLib/IniFile.h"

#include "Protocol.h"
#include "../Packet/PT_Structure.h"
#include "../Packet/PT_ReadPacket.h"
#include "../Packet/PT_WritePacket.h"

#include "ConnectedSession.h"
#include "ConnectedSessionManager.h"
#include "ServerIocp.h"