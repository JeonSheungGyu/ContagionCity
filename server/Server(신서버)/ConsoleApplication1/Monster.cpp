#include "Sector.h"
#include "StateHeader.h"
#include "PacketMaker.h"
#include "Zone.h"
#include "Monster.h"

extern Zone zone;
extern User users[MAX_USER];


Monster::Monster(DWORD id, XMFLOAT3 pos) : Object(id, pos), m_currentAction(wander), m_preAction(wander),
	is_alive(true), is_active(false), InActiveTime(0), m_pStateMachine(new StateMachine<Monster>(this)) {
	// FSM ������� �ʱ�ȭ
	m_pStateMachine->SetCurrentState(Wander::Instance());
}
void Monster::heartBeat() {
	if (is_alive == false)
	{
		printf("is_alive==false id: %d\n", id);
		return;
	}
	if (is_active == false)
	{
		printf("is_active==false id: %d\n", id);
		return;
	}
	if (sector == nullptr) return;

	//�Ÿ��� ���� �׼Ǻ�ȭ
	if (m_target.TargetIsNull()) m_target.FindTarget(this); 
	else m_target.CalculateDistWithTarget(this);

	// �׼ǿ� ���� State ��ȯ
	if (m_preAction != m_currentAction) {
		switch (m_currentAction)
		{
		case wander:
			m_pStateMachine->ChangeState(dynamic_cast<State<Monster>*>(Wander::Instance()));
			break;
		case combat:
			m_pStateMachine->ChangeState(dynamic_cast<State<Monster>*>(Combat::Instance()));
			break;
		case chase:
			m_pStateMachine->ChangeState(dynamic_cast<State<Monster>*>(Chase::Instance()));
			break;
		case die:
			m_pStateMachine->ChangeState(dynamic_cast<State<Monster>*>(Die::Instance()));
			break;
		}
		m_preAction = m_currentAction;
	}

	try {
		m_pStateMachine->Update();	// ���� FSM�� ���� �ൿ
	

		std::set<int> old_view;
		std::set<int> new_view;


		updateNearList();
		// �÷��̾�� �����ϰ� nearList ���ϴ¹������ ����
		for (auto& id : nearList)
		{
			if (id < MAX_USER)
			{
				if (Sector::isinView(this->getOldPos().x, this->getOldPos().z, users[id].getPos().x, users[id].getPos().z))
					old_view.insert(id);
				if (Sector::isinView(this->getPos().x, this->getPos().z, users[id].getPos().x, users[id].getPos().z))
					new_view.insert(id);
			}
		}
		//���ͽþ߿� ������ ������ �������� �����.
		if (new_view.size() == 0) {
			this->is_active = false;
			this->InActiveTime = clock();
			//this->obVector.position = this->regenPos;
		}
		for (auto& player_id : old_view)
		{
			size_t isDeleted = new_view.erase(player_id);

			if (isDeleted)	// old���� �ְ� new���� �ִ�.-> move_obj ȣ��
			{
				switch (m_currentAction) {
				case wander:
				case chase:
					PacketMaker::instance().MonsterChase(reinterpret_cast<Object*>(&users[player_id]), id);
					break;
				case combat:
					PacketMaker::instance().MonsterAttack(reinterpret_cast<Object*>(&users[player_id]), id);
					break;
				case die:
					PacketMaker::instance().MonsterDie(reinterpret_cast<Object*>(&users[player_id]), id);
					break;
				}
			}
			else  // old���� �ְ� new���� ����. -> remove obj ȣ��
			{
				PacketMaker::instance().RemoveObject(reinterpret_cast<Object*>(&users[player_id]), id);
			}
		}

		for (auto& new_player_id : new_view)
		{
			// old���� ���� new���� �ִ�.
			// push_object ȣ��
			PacketMaker::instance().PutObject(reinterpret_cast<Object*>(&users[new_player_id]), id);
		}


		setOldPos(this->getPos());
	}
	catch (std::exception& e) {
		printf("Monster::heart_beat : %s", e.what());
	}
}
/*
	������ �þ߷� ������ �÷��̾ �ִ��� Ž��
*/
bool Monster::isNearUser()
{
	for (auto sector : nearSectors)
	{
		EnterCriticalSection(&Sector::sCS);
		auto vec = sector->getPlayers();
		LeaveCriticalSection(&Sector::sCS);

		for (auto t_id : vec)
		{
			if (t_id == id) continue;

			try {
				//�����ϰ��
				if (t_id < MAX_USER) {
					//������ �ƴѰ��� �н�
					if (!users[t_id].isConnected()) continue;
					if (Sector::isinMonsterView(obVector.position.x, obVector.position.z,
						users[t_id].getObjectVec().position.x, users[t_id].getObjectVec().position.z)) {
						return true;
					}
				}
			}
			catch (std::exception& e) {
				printf("Object::updateNearList %s\n", e.what());
				continue;
			}

		}
	}
	return false;
}