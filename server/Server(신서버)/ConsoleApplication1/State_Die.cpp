#include "stdafx.h"
#include "State_Die.h"

Die* Die::dInstance = nullptr;
std::mutex Die::pMutex;

Die* Die::Instance()
{
	std::lock_guard<std::mutex> guard(pMutex);
	if (dInstance == nullptr)
		dInstance = new Die;

	return dInstance;
}


void Die::Enter(Monster* pMonster)
{
	assert(pMonster && "<Die::Enter>: trying to work to a null state");
}

/*
몬스터 사망
- 타겟 Null 설정
- 플레이어의 경험치 상승
*/
void Die::Execute(Monster* pMonster)
{
	assert(pMonster && "<Die::Execute>: trying to work to a null state");

	// 몬스터를 초기 상태로 초기화
	//pMonster->setPos(pMonster->getRegenPos());
	//pMonster->setState(pMonster->getRegenState());

	if (pMonster->getTargetProcess().getTarget() == nullptr)
		return throw std::exception("Die::Execute monster target is nullptr\n");

	//pMonster->getTargetProcess().getTarget()->plusExp(pMonster->getEXP());
	// pMonster->setAlive(true);
}

void Die::Exit(Monster* pMonster)
{
	printf("Die monster id: ", pMonster->getID());
	assert(pMonster && "<Die::Exit>: trying to work to a null state");
}


Die::~Die()
{
	delete dInstance;
	dInstance = nullptr;
}