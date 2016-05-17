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
// �ش� Ŭ���̾�Ʈ���� ���� ������ ���� �� id_list ����
// �Ű����� : �����ϴ� �÷��̾� ID, �ش� Ŭ���̾�Ʈ���� �˷��� ���� ID_LIST


// ĳ���� ���� CRASH_RANGE �Ÿ���ŭ �� ����
void CombatCollision::CircleAround(const WORD id, vector<pair<WORD, int>>& InfoList, const FLOAT x, const FLOAT z)
{
	assert(id >= 0);

	try {
		auto player = &users[id];
		WORD damage = 0;
		BoundingSphere crashRange(player->getPos(), CA_RANGE);	// �÷��̾� ���� 10�ݰ� �浹üũ

		for (const auto& ID : player->getNearList())
		{
			if (ID < MAX_USER) continue;
			else {
				if (monsters.at(ID - MAX_USER)->is_alive == false) continue;
				if (crashRange.Intersects(monsters.at(ID - MAX_USER)->getCollisionSphere())) {
					// ��������� �� �ش� ���� HP minus
					/*damage = calculator(player->getState().AP, monsters.at(ID - MAX_USER)->getState().DP,
					player->getState().ElementType, monsters.at(ID - MAX_USER)->getState().ElementType);*/
					damage = player->getStatus().damage / 5;
					monsters.at(ID - MAX_USER)->minusHP(damage);

					//��Ŷ�� ������ ���� ����
					InfoList.push_back(make_pair(ID, monsters.at(ID - MAX_USER)->getStatus().hp));
					//Ÿ�ټ���
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

// �� �ݰ� (��) �ȿ� ���� ���� �߿� �÷��̾� �տ� �ִ� (����)�ִ� ����
void CombatCollision::CircleFront(const WORD id, vector<pair<WORD, int>>& InfoList, const FLOAT x, const FLOAT z)
{
	assert(id >= 0);

	try {
		auto player = &users[id];
		WORD damage = 0;
		XMFLOAT3 store;
		BoundingSphere basicRange(player->getPos(), CF_RANGE);	// �÷��̾� ���� 4�ݰ� �浹üũ

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
					if (store.x > 0) {	// ���Ͱ� �÷��̾� �տ� ����
										// ��������� �� �ش� ���� HP minus
										/*damage = calculator(player->getState().AP, monsters.at(ID - MAX_USER)->getState().DP,
										player->getState().ElementType, monsters.at(ID - MAX_USER)->getState().ElementType);*/
						damage = player->getStatus().damage / 5;

						monsters.at(ID - MAX_USER)->minusHP(damage);
						//��Ŷ�� ������ ���� ����
						InfoList.push_back(make_pair(ID, monsters.at(ID - MAX_USER)->getStatus().hp));
						//Ÿ�ټ���
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

// �÷��̾� ��ǥ�� �������� 14 �Ÿ���ŭ ������ ��ų����
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
					//��Ŷ�� ������ ���� ����
					InfoList.push_back(make_pair(ID, monsters.at(ID - MAX_USER)->getStatus().hp));
					//Ÿ�ټ���
					monsters.at(ID - MAX_USER)->getTargetProcess().setTarget(player);
				}
			}
		}
	}
	catch (exception& e) {
		printf("WizardCollision::eraser : %s", e.what());
	}
}


// x,z ��ġ�� �� �浹üũ
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
					//��Ŷ�� ������ ���� ����
					InfoList.push_back(make_pair(ID, monsters.at(ID - MAX_USER)->getStatus().hp));
					//Ÿ�ټ���
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