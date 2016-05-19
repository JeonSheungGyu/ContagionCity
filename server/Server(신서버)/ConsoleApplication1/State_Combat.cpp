#include "stdafx.h"
#include "State_Combat.h"

using namespace DirectX;

Combat* Combat::cInstance = nullptr;
std::mutex Combat::pMutex;
void add_timer(DWORD id, DWORD type, DWORD duration);

Combat* Combat::Instance()
{
	std::lock_guard<std::mutex> guard(pMutex);
	if (cInstance == nullptr)
		cInstance = new Combat;

	return cInstance;
}


void Combat::Enter(Monster* pMonster)
{
	assert(pMonster && "<Combat::Enter>: trying to work to a null state");
}

/*
타겟과 몬스터의 싸움
DamageCalculator가 Attack Defense Type에 따른 데미지를 계산해준다.


*/
void Combat::Execute(Monster* pMonster)
{
	assert(pMonster && "<Combat::Enter>: trying to work to a null state");

	//DamageCalculator calculator;

	if (pMonster->getTargetProcess().getTarget() == nullptr)
		return throw std::exception("Combat::Execute monster target is nullptr\n");

	auto* player = pMonster->getTargetProcess().getTarget();


	//데미지 계산
	/*WORD damage = calculator(pMonster->getState().AP, player->getState().DP,
	pMonster->getState().ElementType, player->getState().ElementType);*/
	WORD damage = 1;

	player->minusHP(damage);		// 플레이어 사망처리 해야됨 ( 주변 클라이언트들에게 전송하게 )
	//pMonster->getTargetProcess().setTmpDamage(damage);
	add_timer(pMonster->getID(), OP_NPC_MOVE, 1000);
	//몬스터 타입에 따른 액션 설정
	//if (pMonster->mMonType == 1)
	//{
	//	cout << "MonCombat Check" << endl;
	//	uniform_int_distribution<int> aRange(0, 10);
	//	random_device aRandDevice;
	//	int aRan = aRange(aRandDevice);
	//	cout << "aRan: " << aRan << endl;
	//	if (aRan > 6)
	//		pMonster->getTargetProcess().ActionType = 1;
	//	else
	//		pMonster->getTargetProcess().ActionType = 0;
	//}
	//else pMonster->getTargetProcess().ActionType = 0;
}


void Combat::Exit(Monster* pMonster)
{
	assert(pMonster && "<Combat::Enter>: trying to work to a null state");
}


Combat::~Combat()
{
	delete cInstance;
	cInstance = nullptr;
}