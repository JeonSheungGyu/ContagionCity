#include "stdafx.h"
#include "State_Wander.h"

const int WANDER_RAGNE = RECTSIZE;
const int CRASH_RANGE = RECTSIZE*6;

using namespace DirectX;

Wander* Wander::wInstance = nullptr;
std::mutex Wander::pMutex;

extern User users[MAX_USER];

Wander* Wander::Instance()
{
	std::lock_guard<std::mutex> guard(pMutex);
	if (wInstance == nullptr)
		wInstance = new Wander;

	return wInstance;
}


void Wander::Enter(Monster* pMonster)
{
	assert(pMonster && "<Wander::Enter>: trying to work to a null state");

	if (!pMonster->getTargetProcess().TargetIsNull())
		pMonster->getTargetProcess().TargetFree();

	//if (pMonster->mMonType == 1)
	//{
	//	if (!pMonster->BossAct)
	//		return;
	//}
}

/*
몬스터 배회
랜덤값을 사용하여 배회하도록 한다.
*/
void Wander::Execute(Monster* pMonster)
{
	assert(pMonster && "<Wander::Execute>: trying to work to a null state");
	//타겟이 널이면 타겟팅을한다.
	if (pMonster->getTargetProcess().TargetIsNull()) {
		try {

			BoundingSphere crashRange(pMonster->getPos(), CRASH_RANGE);	// 몬스터기준반경 구만들기

			for (const auto& t_id : pMonster->getNearList())
			{
				if (t_id < MAX_USER) {
					if (users[t_id].isConnected() == false)continue;
					if (crashRange.Intersects(users[t_id].getCollisionSphere())) {
						// 데미지계산 후 해당 몬스터 HP minus
						/*damage = calculator(player->getState().AP, monsters.at(ID - MAX_USER)->getState().DP,
						player->getState().ElementType, monsters.at(ID - MAX_USER)->getState().ElementType);*/
						//damage = player->getState().AP / 5;

						//monsters.at(ID - MAX_USER)->minusHP(damage);
						//InfoList.push_back(make_pair(ID, monsters.at(ID - MAX_USER)->getState().hp));
						pMonster->getTargetProcess().setTarget(&users[t_id]);
						return;
					}
					else continue;
				}
			}
		}
		catch (std::exception& e) {
			printf("Wander::Execute %s", e.what());
		}
	}


	std::random_device rnd;
	std::uniform_real_distribution<float> tx(pMonster->getPos().x - WANDER_RAGNE, pMonster->getPos().x + WANDER_RAGNE);
	std::uniform_real_distribution<float> tz(pMonster->getPos().z - WANDER_RAGNE, pMonster->getPos().z + WANDER_RAGNE);

	float x = tx(rnd);	float z = tz(rnd);
	// 중간발표 맵 크기
	if (x < 0 || x>WORLDSIZE || z < 0 || z>WORLDSIZE) return;

	try {
		//if (!CollisionMap::Instance().getCollisionMap().at((int)x / NODE_HEIGHT).at((int)z / NODE_WIDTH).is_move) return;
		//cout << "x: " << x << " z: " << z << endl;
		// 방향벡터
		XMVECTOR dir = XMVector3Normalize(XMLoadFloat3(&XMFLOAT3(
			x - pMonster->getPos().x, 0, z - pMonster->getPos().z)));

		// 목표와 현재 플레이어 위치간의 거리
		float dist = sqrt(
			(x - pMonster->getPos().x)*(x - pMonster->getPos().x) +
			(z - pMonster->getPos().z)*(z - pMonster->getPos().z));

		pMonster->setDir(XMFLOAT3(XMVectorGetX(dir), 0, XMVectorGetZ(dir)));
		pMonster->setDist(dist);
		pMonster->setPos(XMFLOAT3(((DWORD)x/RECTSIZE)*RECTSIZE, 0, ((DWORD)z / RECTSIZE)*RECTSIZE));
	}
	catch (std::exception& e) {
		printf("Wander::Execute %s", e.what());
	}

	/*if (pMonster->mMonType == 1)
	{
	cout << pMonster->getPos().x << "\n" << pMonster->getPos().y << "\n" << pMonster->getPos().z << endl;
	}*/
}


void Wander::Exit(Monster* pMonster)
{
	assert(pMonster && "<Wander::Exit>: trying to work to a null state");
}


Wander::~Wander()
{
	delete wInstance;
	wInstance = nullptr;
}