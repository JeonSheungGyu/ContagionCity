#include "stdafx.h"
#include "Protocol.h"
#include "Monster.h"
#include "User.h"
#include "MonsterViewList.h"
#include "PacketMaker.h"

using namespace std;

const int AVAILABLE_NUMBER = 64;

extern User users[MAX_USER];

MonsterViewList::MonsterViewList(Monster *owner)
{
	assert(owner);
	this->owner = owner;
	view.reserve(AVAILABLE_NUMBER);
	tmpList.reserve(AVAILABLE_NUMBER);
}


MonsterViewList::~MonsterViewList() {}


bool MonsterViewList::insertUserInTmp(const DWORD id)
{
	assert(id >= 0);
	if (tmpList.size() >= AVAILABLE_NUMBER)
		return false;

	tmpList.push_back(make_pair(id, true));
	return true;
}


bool MonsterViewList::eraseUserInTmp(const DWORD id)
{
	assert(id >= 0);
	Lock sync(this);
	auto& iter = find_if(tmpList.begin(), tmpList.end(), [=](ViewData& data) {
		if (data.first == id)
		{
			data.second = true;
			return true;
		}
		return false;
	});

	if (iter != tmpList.end())
		tmpList.erase(iter);

	return true;
}



void MonsterViewList::updateViewList(const set<DWORD>& nearList)
{
	// near List�� ��� view List�� �߰��Ǿ���
	tmpList.clear();


	//tmpList�� �����Ѵ�.
	{
		// Lock ���� ������ ���� ������ ����
		
		tmpList.resize(view.size());
		transform(view.begin(), view.end(), tmpList.begin(), [](ViewData& data) {
			data.second = false;	// false ��Ŀ�� �༭ �������� Ȯ��
			return data;
		});
	}

	// nearList�� ������ tmpList(view���纻)�� ���� �� find
	//������ �߰��Ǵ°�
	for (const auto& id : nearList)
	{
		//������ �߰��Ѵ�.
		if (id < MAX_USER) {
			//�̹��ִ°� üũ
			auto &iter = find_if(tmpList.begin(), tmpList.end(), [=](ViewData& data) {
				if (data.first == id)
				{
					data.second = true;
					return true;
				}
				return false;
			});

			// ���°� �߰�
			if (iter == tmpList.end())
			{
				insertUserInTmp(id);
			}

			//�÷��̾��� ��� ���� �� ó��
			try {
				User *player = &users[id];

				if (player->getViewList().isInViewList(owner->getID())) {  // ���� ��� viewList�� �ִ���
																	//�����δٰ� ��Ŷ����
																	//���� ������� ��Ŷ����
					//���� �׼ǿ� ���� ��Ŷ ����
					switch (owner->getAction()) {
					case wander:
						player->getViewList().MonsterWander(owner->getID());
					case chase:
						player->getViewList().MonsterChase(owner->getID());
						break;
					case combat:
						player->getViewList().MonsterAttack(owner->getID());
						break;
					case die:
						player->getViewList().MonsterDie(owner->getID());
						break;
					}

				}
				else {
					//���濡�� �߰��϶�� ��Ŷ����
					player->getViewList().insertUser(owner->getID());
				}
			}
			catch (exception& e) {
				eraseUserInTmp(id);
				printf("MonsterViewList::updateViewList 1 %s", e.what());
				continue;
			}
		}
	}


	// nearList�� ������ tmpList�� ���� �� ã�Ƽ� �ֽ����� ����
	// second �� false�̸� �߰��Ȱ͵� �̹��ִ°͵� �ƴѰ� �׷��Ƿ� �����Ұ͵�
	auto& removeIter = remove_if(tmpList.begin(), tmpList.end(), [&](ViewData& data) {
		if (data.second == false)
		{
			//���Ŵ� �Ʒ����� �ȴ�.
			try {
				if (users[data.first].isConnected())
					users[data.first].getViewList().eraseUser(owner->getID());	// �׻� �� view�� ���� view ����ȭ���Ѿߵ�
			}
			catch (exception& e) {
				printf("MonsterViewList::updateViewList 2 %s", e.what());
			}
			return true;
		}
		return false;
	});
	//���� ����
	tmpList.erase(removeIter, tmpList.end());

	Lock Sync(this);
	view.swap(tmpList);	// �ֽ� view�� ��ü

}

// id�� view �ȿ� �����ϴ��� �˻�
bool MonsterViewList::isInViewList(const DWORD id)
{
	assert(id >= 0);
	Lock sync(this);
	auto& iter = find_if(view.begin(), view.end(), [=](const ViewData& data) {
		return data.first == id;
	});
	return iter != view.end();
}