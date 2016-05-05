#include "stdafx.h"
#include "Monster.h"

Monster::Monster(DWORD id, XMFLOAT3 pos) : m_viewList(this), Object(id, pos), m_currentAction(wander), m_preAction(wander),
	m_isAlive(true), m_pStateMachine(new StateMachine<Monster>(this)) {
	// FSM ������� �ʱ�ȭ
	m_pStateMachine->SetCurrentState(Wander::Instance());
}

void Monster::updateViewList() {
	m_viewList.updateViewList(nearList);
}

void Monster::heartBeat() {
	if (m_isAlive == false)
	{
		printf("is_alive==false id: %d", id);
		return;
	}
	if (sector == nullptr) return;

	//�Ÿ��� ���� �׼Ǻ�ȭ
	if (!m_target.TargetIsNull()) m_target.CalculateDistWithTarget(this);

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
	m_pStateMachine->Update();	// ���� FSM�� ���� �ൿ
	setCollisionSpherePos(getPos()); // �浹ü�� ������Ʈ (��)
}
