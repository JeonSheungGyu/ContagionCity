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

#include "ConnectedSession.h"
#include "ConnectedSessionManager.h"
#include "ServerIocp.h"


#define MAX_USER 100
#define DEFAULT_PORT 1820


// TODO: reference additional headers your program requires here
