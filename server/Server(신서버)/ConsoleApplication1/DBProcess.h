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
		//ĳ���������� �����´�.
		users[id].setStatus(over->status);
	}
};

