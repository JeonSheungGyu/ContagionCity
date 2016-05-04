#include "stdafx.h"
#include "Sector.h"
#include "Zone.h"
#include "User.h"
#include "UserViewList.h"
#include "Monster.h"
#include "ConnectedUserManager.h"

extern User users[MAX_USER];
extern std::vector<Monster*> monsters;
void Zone::SectorUpdateOfPlayer(const unsigned id)
{
	assert(id >= 0);
	// 플레이어 이동 후 섹터 이동여부 확인

	Sector* oldSector = users[id].sector;
	Sector* newSector = getSectorWithPlayer(&users[id]); // 현재좌표 기준으로 갱신
																		    // 이전 섹터와 갱신된 섹터가 다르면,
	if (oldSector != newSector)
	{
		if (oldSector != nullptr ) oldSector->erasePlayer(id);
		newSector->insertPlayer(id);
		users[id].sector = newSector;
	}
	
	
}

void Zone::SectorUpdateOfMonster(const unsigned id)
{
	assert(id >= 0);
	
	Sector* oldSector = monsters.at(id - MAX_USER)->sector;
	Sector* newSector = getSectorWithPoint(monsters.at(id - MAX_USER)->x, monsters.at(id - MAX_USER)->y);

	if (oldSector != newSector)
	{
		if (oldSector != nullptr ) oldSector->erasePlayer(id);
		monsters.at(id - MAX_USER)->sector = newSector;
		newSector->insertPlayer(id );
	}
}