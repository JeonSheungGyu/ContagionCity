#include "stdafx.h"
#include "Object.h"
#include "User.h"
#include "Monster.h"
#include "Zone.h"


extern User users[MAX_USER];
extern std::vector<Zone*> zone;
extern std::vector<Monster*> monsters;

/*NearList*/
void Object::updateNearList()
{
	RECT rt;
	nearSectors.clear();

	// 현재 시야 사각형의 네 꼭짓점을 구한다
	rt.left = obVector.position.x - VIEWRANGE;
	rt.right = obVector.position.x + VIEWRANGE + 1;
	rt.top = obVector.position.z - VIEWRANGE;
	rt.bottom = obVector.position.z + VIEWRANGE + 1;
	
	Zone::insertNearSector(zone.at(stage)->getSectorWithPoint(rt.left, rt.top), nearSectors);
	Zone::insertNearSector(zone.at(stage)->getSectorWithPoint(rt.right, rt.top), nearSectors);
	Zone::insertNearSector(zone.at(stage)->getSectorWithPoint(rt.left, rt.bottom), nearSectors);
	Zone::insertNearSector(zone.at(stage)->getSectorWithPoint(rt.right, rt.bottom), nearSectors);

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
					if (!users[t_id].isConnected()) continue;
					if (Sector::isinView(obVector.position.x, obVector.position.z, 
						users[t_id].getObjectVec().position.x, users[t_id].getObjectVec().position.z)) {
						nearList.insert(t_id);
					}
				}
				else {//몬스터일경우
					if (Sector::isinView(obVector.position.x, obVector.position.z, 
						monsters.at(t_id - MAX_USER)->getObjectVec().position.x, monsters.at(t_id - MAX_USER)->getObjectVec().position.z)) {
						//몬스터가 살아있을 경우에만 추가
						if (monsters.at(t_id - MAX_USER)->is_alive == true)
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

void Object::ObjectDeadReckoning(const float elapsed)
{
	float NewSpeed = speed*(elapsed/1000.0);
	obVector.position.x += obVector.direction.x*NewSpeed;
	obVector.position.z += obVector.direction.z*NewSpeed;

	obVector.dist -= NewSpeed;
	//obVector.position.y += dir.y*speed;
	//cout << "obVector.dist: " << obVector.dist << endl;

	//duration이  dword라서 오차가 생길 수 있다.
	if (obVector.dist <= 1) {
		is_DeadReckoning = false;
	}	
}