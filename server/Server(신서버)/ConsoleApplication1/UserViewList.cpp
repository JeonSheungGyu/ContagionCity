#include "stdafx.h"
#include "Protocol.h"
#include "User.h"
#include "Monster.h"
#include "ConnectedUserManager.h"
#include "UserViewList.h"


using namespace std;

const int AVAILABLE_NUMBER = 64;

extern User users[MAX_USER];
extern std::vector<Monster*> monsters;
void SendPacket(int id, unsigned char *packet);

UserViewList::UserViewList(User *owner)
{
	assert(owner);
	this->owner = owner;
	view.reserve(AVAILABLE_NUMBER);
	tmpList.reserve(AVAILABLE_NUMBER);
}


UserViewList::~UserViewList() {}


bool UserViewList::insertUserInTmp(const DWORD id)
{
	assert(id >= 0);
	if (tmpList.size() >= AVAILABLE_NUMBER)
		return false;

	tmpList.push_back(make_pair(id, true));
	putObject(id);
	return true;
}


bool UserViewList::eraseUserInTmp(const DWORD id)
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


bool UserViewList::insertUser(const DWORD id)
{
	Lock sync(this);
	if (view.size() >= AVAILABLE_NUMBER)
		return false;
	view.push_back(make_pair(id, true));
	putObject(id);
	return true;
}


bool UserViewList::eraseUser(const DWORD id)
{
	assert(id >= 0);
	Lock sync(this);
	auto& iter = find_if(view.begin(), view.end(), [=](ViewData& data) {
		if (data.first == id)
		{
			data.second = true;
			return true;
		}
		return false;
	});

	if (iter != view.end())
		view.erase(iter);

	removeObject(id);
	return true;
}


void UserViewList::updateViewList(const set<DWORD>& nearList)
{
	// near List는 모두 view List에 추가되야함
	tmpList.clear();


	//tmpList에 복사한다.
	{
		// Lock 빠른 해제를 위해 스코프 지정
		Lock sync(this);
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
		if (id < MAX_USER) {
			try {
				User *player = &users[id];

				if (player->viewList.isInViewList(owner->id)) {  // 내가 상대 viewList에 있는지
																 //움직인다고 패킷전송
					player->viewList.moveObject(owner->id);
				}
				else {
					//상대방에게 추가하라고 패킷전송
					player->viewList.insertUser(owner->id);
				}
			}
			catch (exception& e) {
				printf("ViewList::updateViewList 1 %s", e.what());
				eraseUserInTmp(id);
				continue;
			}
		}
	}


	// nearList엔 있지만 tmpList엔 없는 것 찾아서 최신으로 갱신
	// second 가 false이면 추가된것도 이미있는것도 아닌것 그러므로 삭제할것들
	auto& removeIter = remove_if(tmpList.begin(), tmpList.end(), [&](ViewData& data) {
		if (data.second == false)
		{
			removeObject(data.first);
			try {
				if (data.first < MAX_USER) {
					if (users[data.first].isConnected)
						users[data.first].viewList.eraseUser(owner->id);	// 항상 내 view와 상대방 view 동기화시켜야됨
				}
			}
			catch (exception& e) {
				printf("ViewList::updateViewList 2 %s", e.what());
			}
			return true;
		}
		return false;
	});
	tmpList.erase(removeIter, tmpList.end());

	Lock Sync(this);
	view.swap(tmpList);	// 최신 view로 교체
}

// id가 view 안에 존재하는지 검사
bool UserViewList::isInViewList(const DWORD id)
{
	assert(id >= 0);
	Lock sync(this);
	auto& iter = find_if(view.begin(), view.end(), [=](const ViewData& data) {
		return data.first == id;
	});
	return iter != view.end();
}


void UserViewList::putObject(const DWORD id)
{
	assert(id >= 0);
	Object *object;
	if (id < MAX_USER)
		object = &users[id];
	else
		object = monsters.at(id - MAX_USER);

	sc_packet_put_player put_packet;
	put_packet.id = object->id;
	put_packet.size = sizeof(put_packet);
	put_packet.type = SC_PUT_PLAYER;
	put_packet.x = object->x;
	put_packet.y = object->y;
	
	SendPacket(owner->id, reinterpret_cast<unsigned char *>(&put_packet));
	printf("유저 %d :  유저 %d SC_PUT_PLAYER 전송 ( %d, %d ) \n", owner->id, object->id, object->x, object->y );
	//중복패킷 발생가능함. 한명은 뷰를 보고 한명은 tmp를 보고 있으니까
}


void UserViewList::moveObject(const DWORD id)
{
	assert(id >= 0);
	Object *object;
	if (id < MAX_USER)
		object = &users[id];
	else
		object = monsters.at(id - MAX_USER);

	sc_packet_pos pos_packet;
	pos_packet.id = object->id;
	pos_packet.size = sizeof(pos_packet);
	pos_packet.type = SC_POS;
	pos_packet.x = object->x;
	pos_packet.y = object->y;

	SendPacket(owner->id, reinterpret_cast<unsigned char *>(&pos_packet));
	printf("유저 %d :  유저 %d SC_POS 전송\n", owner->id, object->id);
	//중복패킷 발생가능함. 한명은 뷰를 보고 한명은 tmp를 보고 있으니까
}


void UserViewList::removeObject(const DWORD id)
{
	assert(id >= 0);
	Object *object;
	if (id < MAX_USER)
		object = &users[id];
	else
		object = monsters.at(id - MAX_USER);

	sc_packet_remove_player remove_packet;
	remove_packet.id = object->id;
	remove_packet.size = sizeof(remove_packet);
	remove_packet.type = SC_REMOVE_PLAYER;
	
	SendPacket(owner->id, reinterpret_cast<unsigned char *>(&remove_packet));
	printf("유저 %d :  유저 %d SC_REMOVE_PLAYER 전송\n", owner->id, object->id);
	//중복패킷 발생가능함. 한명은 뷰를 보고 한명은 tmp를 보고 있으니까
}


