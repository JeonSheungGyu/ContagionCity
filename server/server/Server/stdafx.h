// stdafx.h : ���� ��������� ���� ��������� �ʴ�
// ǥ�� �ý��� ���� ���� �� ������Ʈ ���� ���� ������ 
// ��� �ִ� ���� �����Դϴ�.
//

#pragma once

#define MAX_USER 100
#define DEFAULT_PORT 1820
#define MAX_USER 100
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

//������ ���ӻ��¸� ��Ÿ�������� STATUS
typedef enum USER_STATUS
{
	US_NONE = 0,
	US_CHANNEL_ENTERING,
	US_CHANNEL_ENTERED,
	US_ROOM_ENTERING,
	US_ROOM_ENTERED,
	US_ROOM_LEAVING,
	US_GAME_STARTING,
	US_GAME_STARTED,
	US_GAME_ENDING
};
#include <DirectXCollision.h>


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

#include "ConnectedUser.h"
#include "ConnectedUserManager.h"
#include "ServerIocp.h"
