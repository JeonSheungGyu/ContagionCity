#include "stdafx.h"
#include "State_Chase.h"

using namespace DirectX;

Chase* Chase::cInstance = nullptr;
std::mutex Chase::pMutex;

const int OBJECT_INTERVAL = RECTSIZE;

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
타겟을 따라가는 몬스터 State
(타겟 - 몬스터 위치)를 가지고 몬스터의 방향 거리 위치를 지정한다.
*/
void Chase::Execute(Monster* pMonster)
{
	assert(pMonster && "<Chase::Enter>: trying to work to a null state");

	XMFLOAT3 mPos = pMonster->getPos();

	if (pMonster->getTargetProcess().getTarget() == nullptr)
		return throw std::exception("Combat::Execute monster target is nullptr\n");

	XMFLOAT3 tPos = pMonster->getTargetProcess().getTarget()->getPos();

	//XMVECTOR dir = XMVector3Normalize(XMLoadFloat3(&XMFLOAT3((tPos.x) - mPos.x, 0, (tPos.z) - mPos.z)));

	//쫓는방향
	if (tPos.x > mPos.x)
		mPos.x += OBJECT_INTERVAL;
	else {
		if (tPos.x == mPos.x) {
			(tPos.z > mPos.z) ? mPos.z += OBJECT_INTERVAL : mPos.z -= OBJECT_INTERVAL;
		}
		else
			mPos.x -= OBJECT_INTERVAL;
	}
		
	

	//FLOAT dist = sqrt((tPos.x - mPos.x)*(tPos.x - mPos.x) + (tPos.z - mPos.z)*(tPos.z - mPos.z));
	//dist += OBJECT_INTERVAL;
	XMVECTOR dir = XMVector3Normalize(XMLoadFloat3(&XMFLOAT3(
		mPos.x - pMonster->getPos().x, 0, mPos.z - pMonster->getPos().z)));

	// 목표와 현재 플레이어 위치간의 거리
	float dist = sqrt(
		(mPos.x - pMonster->getPos().x)*(mPos.x - pMonster->getPos().x) +
		(mPos.z - pMonster->getPos().z)*(mPos.z - pMonster->getPos().z));

	pMonster->setDir(XMFLOAT3(XMVectorGetX(dir), 0, XMVectorGetZ(dir)));
	pMonster->setDist(dist);
	pMonster->setPos(XMFLOAT3(mPos.x, mPos.y, mPos.z));
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