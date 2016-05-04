#include "stdafx.h"
#include "Object.h"
#include "User.h"
#include "Monster.h"
#include "Zone.h"

extern Zone zone;
extern User users[MAX_USER];
extern std::vector<Monster*> monsters;

/*NearList*/
void Object::updateNearList()
{
	RECT rt;
	nearSectors.clear();

	// 현재 시야 사각형의 네 꼭짓점을 구한다
	rt.left = x - VIEWRANGE;
	rt.right = x + VIEWRANGE + 1;
	rt.top = y - VIEWRANGE;
	rt.bottom = y + VIEWRANGE + 1;
	
	Zone::insertNearSector(zone.getSectorWithPoint(rt.left, rt.top), nearSectors);
	Zone::insertNearSector(zone.getSectorWithPoint(rt.right, rt.top), nearSectors);
	Zone::insertNearSector(zone.getSectorWithPoint(rt.left, rt.bottom), nearSectors);
	Zone::insertNearSector(zone.getSectorWithPoint(rt.right, rt.bottom), nearSectors);

	//printf("%d -> %d\n", id, nearSectors.size());
	// nearSector에 있는 유저들 중 시야 내에 있는 유저만 nearList에 추가
	nearList.clear();
	for(auto sector : nearSectors)
	{
		EnterCriticalSection(&Sector::sCS);
		auto vec = sector->getPlayers();
		LeaveCriticalSection(&Sector::sCS);

		for (auto t_id : vec )
		{
			if (t_id == id) continue;

			try {
				//유저일경우
				if (t_id < MAX_USER) {
					//접속이 아닌경우는 패스
					if (!users[t_id].isConnected) continue;
					if (Sector::isinView(x, y, users[t_id].x, users[t_id].y)) {
						nearList.insert(t_id);
					}
				}
				else {//몬스터일경우
					if (Sector::isinView(x, y, monsters.at(t_id - MAX_USER)->x, monsters.at(t_id - MAX_USER)->y)) {
						nearList.insert(t_id);
					}
				}
			}
			catch(std::exception& e) {
				printf("Object::updateNearList %s\n", e.what());
				continue;
			}

		}
	}
}