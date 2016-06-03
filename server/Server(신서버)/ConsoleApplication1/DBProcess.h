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
		//캐릭터정보
		users[id].setStatus(over->status);
		//DB 저장 내용과 상과없이 처음 접속시 마을에서 시작
		users[id].setStage(Stages::STAGE_VILLIGE);
		users[id].setPos(XMFLOAT3(0,0,0));
		users[id].setSpeed(300);
	}
};

