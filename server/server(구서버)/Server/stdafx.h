// stdafx.h : ���� ��������� ���� ��������� �ʴ�
// ǥ�� �ý��� ���� ���� �� ������Ʈ ���� ���� ������ 
// ��� �ִ� ���� �����Դϴ�.
//

#pragma once

#define MAX_USER 100
#define DEFAULT_PORT 1820
#define MAX_USER 100
#define MAX_ROOM 100
#define CONFIG_FILENAME								_T("./config.ini")

#include "../CoreLib/Global.h"


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
#include "../CoreLib/Log.h"

#include "ErrorCode.h"
#include "../Packet/TcpProtocol.h"
#include "../Packet/Tcp_Structure.h"
#include "../Packet/Tcp_ReadPacket.h"
#include "../Packet/Tcp_WritePacket.h"

#include "ConnectedUser.h"
#include "ConnectedUserManager.h"
#include "GameIocp.h"


