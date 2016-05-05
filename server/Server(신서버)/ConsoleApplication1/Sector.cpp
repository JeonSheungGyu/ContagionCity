#include "stdafx.h"
#include "UserViewList.h"
#include "User.h"
#include "Sector.h"


const int NUMBER_OF_PLAYER = 10;
CRITICAL_SECTION Sector::sCS;

Sector::Sector()
{
	inPlayers.reserve(NUMBER_OF_PLAYER);

	InitializeCriticalSection(&sCS);
}


Sector::~Sector()
{
	DeleteCriticalSection(&sCS);
}


bool Sector::insertPlayer(const DWORD id)
{
	assert(id >= 0);
	EnterCriticalSection(&sCS);
	auto iter = find(inPlayers.begin(), inPlayers.end(), id);

	if (iter != inPlayers.end())//안에 이미 있음
		return false;
	inPlayers.push_back(id);
	LeaveCriticalSection(&sCS);
	return true;
}


bool Sector::erasePlayer(const DWORD id)
{
	assert(id >= 0);
	EnterCriticalSection(&sCS);
	auto iter = find(inPlayers.begin(), inPlayers.end(), id);

	if (iter == inPlayers.end())//안에 없음
		return false;
	inPlayers.erase(iter);
	LeaveCriticalSection(&sCS);
	return true;
}


bool Sector::isinView(float x1, float y1, float x2, float y2)
{
	//assert(pos1.x >= 0 && pos1.y >= 0 && pos1.z >= 0 && pos2.x >= 0 && pos2.y >= 0 && pos2.z >= 0);
	int x = x1 - x2;
	int y = y1 - y2;

	return abs(x) < VIEWRANGE && abs(y) < VIEWRANGE;
}
