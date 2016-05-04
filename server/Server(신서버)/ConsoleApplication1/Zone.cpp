#include "stdafx.h"
#include "Sector.h"
#include "Zone.h"
#include "User.h"
#include "UserViewList.h"
#include "Monster.h"

extern User users[MAX_USER];
extern std::vector<Monster*> monsters;
void Zone::SectorUpdateOfPlayer(const unsigned id)
{
	assert(id >= 0);
	// �÷��̾� �̵� �� ���� �̵����� Ȯ��

	Sector* oldSector = users[id].getCurrentSector();
	Sector* newSector = getSectorWithPlayer(&users[id]); // ������ǥ �������� ����
																		    // ���� ���Ϳ� ���ŵ� ���Ͱ� �ٸ���,
	if (oldSector != newSector)
	{
		if (oldSector != nullptr ) oldSector->erasePlayer(id);
		newSector->insertPlayer(id);
		users[id].setCurrentSector( newSector);
	}
	
	
}

void Zone::SectorUpdateOfMonster(const unsigned id)
{
	assert(id >= 0);
	
	Sector* oldSector = monsters.at(id - MAX_USER)->getCurrentSector();
	Sector* newSector = getSectorWithPoint(monsters.at(id - MAX_USER)->getPos().x, monsters.at(id - MAX_USER)->getPos().z);

	if (oldSector != newSector)
	{
		if (oldSector != nullptr ) oldSector->erasePlayer(id);
		monsters.at(id - MAX_USER)->setCurrentSector(newSector);
		newSector->insertPlayer(id );
	}
}