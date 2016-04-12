// stdafx.h : ���� ��������� ���� ��������� �ʴ�
// ǥ�� �ý��� ���� ���� �� ������Ʈ ���� ���� ������ 
// ��� �ִ� ���� �����Դϴ�.
//

#pragma once

#define DEFAULT_PORT 1820

#include "../CoreLib/Global.h"

#include "../CoreLib/CriticalSection.h"
#include "../CoreLib/MultiThreadSync.h"
#include "../CoreLib/MemoryPool.h"
#include "../CoreLib/CircularQueue.h"
#include "../CoreLib/NetworkSession.h"
#include "../CoreLib/PacketSession.h"
#include "../CoreLib/EventSelect.h"
#include "../CoreLib/Stream.h"
#include "../CoreLib/MiniDump.h"
#include "../CoreLib/IniFile.h"
#include "../CoreLib/ClientSession.h"

#include "../Packet/Protocol.h"
#include "../Packet/PT_Structure.h"
#include "../Packet/PT_ReadPacket.h"
#include "../Packet/PT_WritePacket.h"
