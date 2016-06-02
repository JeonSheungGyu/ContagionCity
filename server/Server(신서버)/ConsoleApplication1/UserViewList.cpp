#include "stdafx.h"
#include "Protocol.h"
#include "User.h"
#include "Monster.h"
#include "UserViewList.h"
#include "PacketMaker.h"

using namespace std;

const int AVAILABLE_NUMBER = 64;

extern User users[MAX_USER];
extern std::vector<Monster*> monsters;
void add_timer(DWORD id, DWORD type, DWORD duration);

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

				if (player->getViewList().isInViewList(owner->getID())) {  
	
					if (owner->getAction() == action_move)
						player->getViewList().moveObject(owner->getID());

					else if (owner->getAction() == action_combat)
						player->getViewList().combatObject(owner->getID());
				}
				else player->getViewList().insertUser(owner->getID());
			}
			catch (exception& e) {
				printf("ViewList::updateViewList 1 %s", e.what());
				eraseUserInTmp(id);
				continue;
			}
		}
		else {
			//MONSTER �� ��� �����̰� �Ѵ�.
			//��Ȱ��ȭ �����ϰ�� 
			//��Ȱ��ȭ�� ��� �ð��� 2���̻� �����°� �Ǵ�
			if (!monsters.at(id - MAX_USER)->is_active && clock() - monsters.at(id - MAX_USER)->InActiveTime  > 2500) {
				monsters.at(id - MAX_USER)->is_active = true;
				add_timer(id, OP_NPC_MOVE, 0);
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
					if (users[data.first].isConnected())
						users[data.first].getViewList().eraseUser(owner->getID());	// �׻� �� view�� ���� view ����ȭ���Ѿߵ�
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
	PacketMaker::instance().PutObject(owner, id);
}


void UserViewList::moveObject(const DWORD id)
{
	PacketMaker::instance().MoveObject(owner, id);
}


void UserViewList::removeObject(const DWORD id)
{
	PacketMaker::instance().RemoveObject(owner, id);
}

void UserViewList::combatObject(const DWORD id)
{
	PacketMaker::instance().CombatObject(owner, id);
}