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

	// ���� �þ� �簢���� �� �������� ���Ѵ�
	rt.left = obVector.position.x - VIEWRANGE;
	rt.right = obVector.position.x + VIEWRANGE + 1;
	rt.top = obVector.position.z - VIEWRANGE;
	rt.bottom = obVector.position.z + VIEWRANGE + 1;
	
	Zone::insertNearSector(zone.at(stage)->getSectorWithPoint(rt.left, rt.top), nearSectors);
	Zone::insertNearSector(zone.at(stage)->getSectorWithPoint(rt.right, rt.top), nearSectors);
	Zone::insertNearSector(zone.at(stage)->getSectorWithPoint(rt.left, rt.bottom), nearSectors);
	Zone::insertNearSector(zone.at(stage)->getSectorWithPoint(rt.right, rt.bottom), nearSectors);

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
					if (!users[t_id].isConnected()) continue;
					if (Sector::isinView(obVector.position.x, obVector.position.z, 
						users[t_id].getObjectVec().position.x, users[t_id].getObjectVec().position.z)) {
						nearList.insert(t_id);
					}
				}
				else {//�����ϰ��
					if (Sector::isinView(obVector.position.x, obVector.position.z, 
						monsters.at(t_id - MAX_USER)->getObjectVec().position.x, monsters.at(t_id - MAX_USER)->getObjectVec().position.z)) {
						//���Ͱ� ������� ��쿡�� �߰�
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

	//duration��  dword�� ������ ���� �� �ִ�.
	if (obVector.dist <= 1) {
		is_DeadReckoning = false;
	}	
}