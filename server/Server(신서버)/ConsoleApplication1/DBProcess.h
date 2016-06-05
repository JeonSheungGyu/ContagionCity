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
		//캐릭터정보만 가져온다.
		users[id].setStatus(over->status);
	}
};

