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
	// near List는 모두 view List에 추가되야함
	tmpList.clear();


	//tmpList에 복사한다.
	{
		// Lock 빠른 해제를 위해 스코프 지정
		
		tmpList.resize(view.size());
		transform(view.begin(), view.end(), tmpList.begin(), [](ViewData& data) {
			data.second = false;	// false 마커를 줘서 수정여부 확인
			return data;
		});
	}

	// nearList엔 있지만 tmpList(view복사본)엔 없는 것 find
	//새로이 추가되는것
	for (const auto& id : nearList)
	{
		//유저만 추가한다.
		if (id < MAX_USER) {
			//이미있는거 체크
			auto &iter = find_if(tmpList.begin(), tmpList.end(), [=](ViewData& data) {
				if (data.first == id)
				{
					data.second = true;
					return true;
				}
				return false;
			});

			// 없는거 추가
			if (iter == tmpList.end())
			{
				insertUserInTmp(id);
			}

			//플레이어일 경우 상대방 뷰 처리
			try {
				User *player = &users[id];

				if (player->getViewList().isInViewList(owner->getID())) {  // 내가 상대 viewList에 있는지
																	//움직인다고 패킷전송
																	//몬스터 방식으로 패킷전송
					//몬스터 액션에 따른 패킷 전송
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
					//상대방에게 추가하라고 패킷전송
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


	// nearList엔 있지만 tmpList엔 없는 것 찾아서 최신으로 갱신
	// second 가 false이면 추가된것도 이미있는것도 아닌것 그러므로 삭제할것들
	auto& removeIter = remove_if(tmpList.begin(), tmpList.end(), [&](ViewData& data) {
		if (data.second == false)
		{
			//제거는 아래에서 된다.
			try {
				if (users[data.first].isConnected())
					users[data.first].getViewList().eraseUser(owner->getID());	// 항상 내 view와 상대방 view 동기화시켜야됨
			}
			catch (exception& e) {
				printf("MonsterViewList::updateViewList 2 %s", e.what());
			}
			return true;
		}
		return false;
	});
	//유저 제거
	tmpList.erase(removeIter, tmpList.end());

	Lock Sync(this);
	view.swap(tmpList);	// 최신 view로 교체

}

// id가 view 안에 존재하는지 검사
bool MonsterViewList::isInViewList(const DWORD id)
{
	assert(id >= 0);
	Lock sync(this);
	auto& iter = find_if(view.begin(), view.end(), [=](const ViewData& data) {
		return data.first == id;
	});
	return iter != view.end();
}