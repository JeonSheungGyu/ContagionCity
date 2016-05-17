#include "stdafx.h"
#include "User.h"
#include "Monster.h"
#include "CombatCollision.h"

using namespace std;

const int CA_RANGE = 80;
const int CF_RANGE = 40;
const int E_DIST = 80;
const int PC_RANGE = 40;

extern User users[MAX_USER];
extern std::vector<Monster*> monsters;
// 해당 클라이언트에게 보낼 데미지 갱신 및 id_list 갱신
// 매개변수 : 공격하는 플레이어 ID, 해당 클라이언트에게 알려준 몬스터 ID_LIST


// 캐릭터 주위 CRASH_RANGE 거리만큼 원 공격
void CombatCollision::CircleAround(const WORD id, vector<pair<WORD, int>>& InfoList, const FLOAT x, const FLOAT z)
{
	assert(id >= 0);

	try {
		auto player = &users[id];
		WORD damage = 0;
		BoundingSphere crashRange(player->getPos(), CA_RANGE);	// 플레이어 기준 10반경 충돌체크

		for (const auto& ID : player->getNearList())
		{
			if (ID < MAX_USER) continue;
			else {
				if (monsters.at(ID - MAX_USER)->is_alive == false) continue;
				if (crashRange.Intersects(monsters.at(ID - MAX_USER)->getCollisionSphere())) {
					// 데미지계산 후 해당 몬스터 HP minus
					/*damage = calculator(player->getState().AP, monsters.at(ID - MAX_USER)->getState().DP,
					player->getState().ElementType, monsters.at(ID - MAX_USER)->getState().ElementType);*/
					damage = player->getStatus().damage / 5;
					monsters.at(ID - MAX_USER)->minusHP(damage);

					//패킷에 전송할 몬스터 정보
					InfoList.push_back(make_pair(ID, monsters.at(ID - MAX_USER)->getStatus().hp));
					//타겟설정
					monsters.at(ID - MAX_USER)->getTargetProcess().setTarget(player);
				}
				else continue;
			}
		}
	}
	catch (exception& e) {
		printf("CombatCollision::crash : %s", e.what());
	}

}

// 검 반경 (원) 안에 들어온 몬스터 중에 플레이어 앞에 있는 (내적)있는 몬스터
void CombatCollision::CircleFront(const WORD id, vector<pair<WORD, int>>& InfoList, const FLOAT x, const FLOAT z)
{
	assert(id >= 0);

	try {
		auto player = &users[id];
		WORD damage = 0;
		XMFLOAT3 store;
		BoundingSphere basicRange(player->getPos(), CF_RANGE);	// 플레이어 기준 4반경 충돌체크

		for (const auto& ID : player->getNearList())
		{
			if (ID < MAX_USER) continue;
			else {
				if (monsters.at(ID - MAX_USER)->is_alive == false) continue;
				if (basicRange.Intersects(monsters.at(ID - MAX_USER)->getCollisionSphere())) {
					XMFLOAT3 tmp(monsters.at(ID - MAX_USER)->getPos().x - player->getPos().x, monsters.at(ID - MAX_USER)->getPos().y - player->getPos().y,
						monsters.at(ID - MAX_USER)->getPos().z - player->getPos().z);

					XMVECTOR inner = XMVector3Dot(XMLoadFloat3(&player->getDir()), XMLoadFloat3(&tmp));
					XMStoreFloat3(&store, inner);
					if (store.x > 0) {	// 몬스터가 플레이어 앞에 있음
										// 데미지계산 후 해당 몬스터 HP minus
										/*damage = calculator(player->getState().AP, monsters.at(ID - MAX_USER)->getState().DP,
										player->getState().ElementType, monsters.at(ID - MAX_USER)->getState().ElementType);*/
						damage = player->getStatus().damage / 5;

						monsters.at(ID - MAX_USER)->minusHP(damage);
						//패킷에 전송할 몬스터 정보
						InfoList.push_back(make_pair(ID, monsters.at(ID - MAX_USER)->getStatus().hp));
						//타겟설정
						monsters.at(ID - MAX_USER)->getTargetProcess().setTarget(player);
					}
				}
				else continue;
			}
		}
	}
	catch (exception& e) {
		printf("CombatCollision::basic : %s", e.what());
	}
}

// 플레이어 좌표를 시작으로 14 거리만큼 일직선 스킬공격
void CombatCollision::Eraser(const WORD id, vector<pair<WORD, int>>& InfoList, const FLOAT x, const FLOAT z)
{
	assert(id >= 0);

	try {
		auto player = &users[id];
		float dist = E_DIST;
		WORD damage = 0;

		for (const auto& ID : player->getNearList())
		{
			if (ID < MAX_USER) continue;
			else {
				if (RayCast(player->getPos(), player->getDir(), dist, monsters.at(ID - MAX_USER)->getCollisionSphere())) {
					//damage = calculator(player->getState().AP, monsters.at(ID - MAX_USER)->getState().DP,
						//player->getState().ElementType, monsters.at(ID - MAX_USER)->getState().ElementType);
					damage = player->getStatus().damage / 5;

					monsters.at(ID - MAX_USER)->minusHP(damage);
					//패킷에 전송할 몬스터 정보
					InfoList.push_back(make_pair(ID, monsters.at(ID - MAX_USER)->getStatus().hp));
					//타겟설정
					monsters.at(ID - MAX_USER)->getTargetProcess().setTarget(player);
				}
			}
		}
	}
	catch (exception& e) {
		printf("WizardCollision::eraser : %s", e.what());
	}
}


// x,z 위치에 원 충돌체크
void CombatCollision::PointCircle(const WORD id, vector<pair<WORD, int>>& InfoList, const FLOAT x, const FLOAT z)
{
	assert(id >= 0);

	try {
		auto player = &users[id];
		WORD damage = 0;
		BoundingSphere holeRange(XMFLOAT3(x, 0, z), PC_RANGE);

		for (const auto& ID : player->getNearList())
		{
			if (ID < MAX_USER) continue;
			else {
				if (holeRange.Intersects(monsters.at(ID - MAX_USER)->getCollisionSphere())) {
					/*damage = calculator(player->getState().AP, monsters.at(ID - MAX_USER)->getState().DP,
						player->getState().ElementType, monsters.at(ID - MAX_USER)->getState().ElementType);*/

					damage = player->getStatus().damage / 5;
					monsters.at(ID - MAX_USER)->minusHP(damage);
					//패킷에 전송할 몬스터 정보
					InfoList.push_back(make_pair(ID, monsters.at(ID - MAX_USER)->getStatus().hp));
					//타겟설정
					monsters.at(ID - MAX_USER)->getTargetProcess().setTarget(player);
				}
			}
		}
	}
	catch (exception& e) {
		printf("WizardCollision::wormHole : %s", e.what());
	}
}
// ~WizardCollision