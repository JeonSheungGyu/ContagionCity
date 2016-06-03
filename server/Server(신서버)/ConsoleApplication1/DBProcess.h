#pragma once
#include "stdafx.h"
#include "User.h"
// DB ó���Լ� static���� ����

extern User users[MAX_USER];
class DBProcess
{
public:
	static void RequestState(const Overlap_ex* over, const DWORD id)
	{
		//ĳ��������
		users[id].setStatus(over->status);
		//DB ���� ����� ������� ó�� ���ӽ� �������� ����
		users[id].setStage(Stages::STAGE_VILLIGE);
		users[id].setPos(XMFLOAT3(0,0,0));
		users[id].setSpeed(300);
	}
};

