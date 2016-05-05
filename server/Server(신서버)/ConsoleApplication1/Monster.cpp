#include "stdafx.h"
#include "Monster.h"

Monster::Monster(DWORD id, XMFLOAT3 pos) : m_viewList(this), Object(id, pos), m_currentAction(wander), m_preAction(wander),
	m_isAlive(true), m_pStateMachine(new StateMachine<Monster>(this)) {
	// FSM 현재상태 초기화
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

	//거리에 따른 액션변화
	if (!m_target.TargetIsNull()) m_target.CalculateDistWithTarget(this);

	// 액션에 따른 State 변환
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
	m_pStateMachine->Update();	// 현재 FSM에 따라 행동
	setCollisionSpherePos(getPos()); // 충돌체도 업데이트 (원)
}
