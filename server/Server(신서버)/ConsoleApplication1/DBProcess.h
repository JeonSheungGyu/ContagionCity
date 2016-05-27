#pragma once
#include "stdafx.h"
#include "User.h"
// DB 처리함수 static으로 선언

extern User users[MAX_USER];
class DBProcess
{
public:
	static void RequestState(const Overlap_ex* over, const DWORD id)
	{
		users[id].setStatus(over->status);
		users[id].setPos(over->pos);

		//패킷을전송하여야한다.
		//PacketMaker::instance().Login(id);
	}
};

