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

	// ���� �þ� �簢���� �� �������� ���Ѵ�
	rt.left = x - VIEWRANGE;
	rt.right = x + VIEWRANGE + 1;
	rt.top = y - VIEWRANGE;
	rt.bottom = y + VIEWRANGE + 1;
	
	Zone::insertNearSector(zone.getSectorWithPoint(rt.left, rt.top), nearSectors);
	Zone::insertNearSector(zone.getSectorWithPoint(rt.right, rt.top), nearSectors);
	Zone::insertNearSector(zone.getSectorWithPoint(rt.left, rt.bottom), nearSectors);
	Zone::insertNearSector(zone.getSectorWithPoint(rt.right, rt.bottom), nearSectors);

	//printf("%d -> %d\n", id, nearSectors.size());
	// nearSector�� �ִ� ������ �� �þ� ���� �ִ� ������ nearList�� �߰�
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
				//�����ϰ��
				if (t_id < MAX_USER) {
					//������ �ƴѰ��� �н�
					if (!users[t_id].isConnected) continue;
					if (Sector::isinView(x, y, users[t_id].x, users[t_id].y)) {
						nearList.insert(t_id);
					}
				}
				else {//�����ϰ��
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