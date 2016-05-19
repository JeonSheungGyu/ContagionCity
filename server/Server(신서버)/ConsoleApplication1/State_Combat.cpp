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
Ÿ�ٰ� ������ �ο�
DamageCalculator�� Attack Defense Type�� ���� �������� ������ش�.


*/
void Combat::Execute(Monster* pMonster)
{
	assert(pMonster && "<Combat::Enter>: trying to work to a null state");

	//DamageCalculator calculator;

	if (pMonster->getTargetProcess().getTarget() == nullptr)
		return throw std::exception("Combat::Execute monster target is nullptr\n");

	auto* player = pMonster->getTargetProcess().getTarget();


	//������ ���
	/*WORD damage = calculator(pMonster->getState().AP, player->getState().DP,
	pMonster->getState().ElementType, player->getState().ElementType);*/
	WORD damage = 1;

	player->minusHP(damage);		// �÷��̾� ���ó�� �ؾߵ� ( �ֺ� Ŭ���̾�Ʈ�鿡�� �����ϰ� )
	//pMonster->getTargetProcess().setTmpDamage(damage);
	add_timer(pMonster->getID(), OP_NPC_MOVE, 1000);
	//���� Ÿ�Կ� ���� �׼� ����
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