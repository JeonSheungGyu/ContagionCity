// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently

#pragma once

#include "../CoreLib/Global.h"
#include "../CoreLib/MultiThreadSync.h"
#include "../CoreLib/MemoryPool.h"
#include "../CoreLib/CircularQueue.h"
#include "../CoreLib/NetworkSession.h"
#include "../CoreLib/PacketSession.h"
#include "../CoreLib/Iocp.h"
#include "../CoreLib/Stream.h"
#include "../CoreLib/MiniDump.h"
#include "../CoreLib/IniFile.h"
#include "../CoreLib/Log.h"


#define MAX_USER 100
#define DEFAULT_PORT 1820


typedef enum USER_STATUS{
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


DWORD DEFAULT_MAX_HP = 100;
DWORD DEFAULT_GAME_TIME = 0;



#include "TcpProtocol.h"
#include "Character.h"
#include "ConnectedUser.h"
#include "ConnectedUserManager.h"
#include "Room.h"
#include "RoomManager.h"
#include "ServerIocp.h"




// TODO: reference additional headers your program requires here
