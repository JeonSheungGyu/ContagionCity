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
	// near List�� ��� view List�� �߰��Ǿ���
	tmpList.clear();


	//tmpList�� �����Ѵ�.
	{
		// Lock ���� ������ ���� ������ ����
		Lock sync(this);
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
		if (id < MAX_USER) {
			try {
				User *player = &users[id];

				if (player->viewList.isInViewList(owner->id)) {  // ���� ��� viewList�� �ִ���
																 //�����δٰ� ��Ŷ����
					player->viewList.moveObject(owner->id);
				}
				else {
					//���濡�� �߰��϶�� ��Ŷ����
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


	// nearList�� ������ tmpList�� ���� �� ã�Ƽ� �ֽ����� ����
	// second �� false�̸� �߰��Ȱ͵� �̹��ִ°͵� �ƴѰ� �׷��Ƿ� �����Ұ͵�
	auto& removeIter = remove_if(tmpList.begin(), tmpList.end(), [&](ViewData& data) {
		if (data.second == false)
		{
			removeObject(data.first);
			try {
				if (data.first < MAX_USER) {
					if (users[data.first].isConnected)
						users[data.first].viewList.eraseUser(owner->id);	// �׻� �� view�� ���� view ����ȭ���Ѿߵ�
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
	view.swap(tmpList);	// �ֽ� view�� ��ü
}

// id�� view �ȿ� �����ϴ��� �˻�
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
	printf("���� %d :  ���� %d SC_PUT_PLAYER ���� ( %d, %d ) \n", owner->id, object->id, object->x, object->y );
	//�ߺ���Ŷ �߻�������. �Ѹ��� �並 ���� �Ѹ��� tmp�� ���� �����ϱ�
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
	printf("���� %d :  ���� %d SC_POS ����\n", owner->id, object->id);
	//�ߺ���Ŷ �߻�������. �Ѹ��� �並 ���� �Ѹ��� tmp�� ���� �����ϱ�
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
	printf("���� %d :  ���� %d SC_REMOVE_PLAYER ����\n", owner->id, object->id);
	//�ߺ���Ŷ �߻�������. �Ѹ��� �並 ���� �Ѹ��� tmp�� ���� �����ϱ�
}


