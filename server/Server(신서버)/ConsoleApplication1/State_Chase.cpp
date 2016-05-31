#include "stdafx.h"
#include "State_Chase.h"

using namespace DirectX;

Chase* Chase::cInstance = nullptr;
std::mutex Chase::pMutex;

const int OBJECT_INTERVAL = RECTSIZE;

void add_timer(DWORD id, DWORD type, DWORD duration);

Chase* Chase::Instance()
{
	std::lock_guard<std::mutex> guard(pMutex);
	if (cInstance == nullptr)
		cInstance = new Chase;

	return cInstance;
}


void Chase::Enter(Monster* pMonster)
{
	assert(pMonster && "<Chase::Enter>: trying to work to a null state");
}

/*
Ÿ���� ���󰡴� ���� State
(Ÿ�� - ���� ��ġ)�� ������ ������ ���� �Ÿ� ��ġ�� �����Ѵ�.
Ŭ���̾�Ʈ������ �Ÿ��� ������ ������ ���͸� �����δ�.
*/
void Chase::Execute(Monster* pMonster)
{
	assert(pMonster && "<Chase::Enter>: trying to work to a null state");

	XMFLOAT3 mPos = pMonster->getPos();

	if (pMonster->getTargetProcess().getTarget() == nullptr)
		return throw std::exception("Combat::Execute monster target is nullptr\n");

	XMFLOAT3 tPos = pMonster->getTargetProcess().getTarget()->getPos();

	XMVECTOR dir = XMVector3Normalize(XMLoadFloat3(&XMFLOAT3((tPos.x) - mPos.x, 0, (tPos.z) - mPos.z)));

	tPos.x = tPos.x - XMVectorGetX(dir)*OBJECT_INTERVAL;
	tPos.z = tPos.z - XMVectorGetZ(dir)*OBJECT_INTERVAL;

	FLOAT dist = sqrt((tPos.x - mPos.x)*(tPos.x - mPos.x) + (tPos.z - mPos.z)*(tPos.z - mPos.z));
	//dist += OBJECT_INTERVAL;

	//���� ���� �̵��� ���
	pMonster->setDir(XMFLOAT3(XMVectorGetX(dir), 0, XMVectorGetZ(dir)));
	pMonster->setDist(dist);
	pMonster->setDeadReckoning(true);
	//Ŭ���̾�Ʈ ���� �̵��� ���
	pMonster->setTargetPos(XMFLOAT3(tPos.x, tPos.y, tPos.z));
	//���� ������Ʈ
	DWORD duration = (dist / pMonster->getSpeed())*1000;
	if ( duration < 1000) {
		add_timer(pMonster->getID(), OP_NPC_MOVE, duration);
	}
	else {
		add_timer(pMonster->getID(), OP_NPC_MOVE, 1000);
	}
	//pMonster->setPos(XMFLOAT3(tPos.x, tPos.y, tPos.z));
}



void Chase::Exit(Monster* pMonster)
{
	assert(pMonster && "<Chase::Enter>: trying to work to a null state");
}


Chase::~Chase()
{
	delete cInstance;
	cInstance = nullptr;
}