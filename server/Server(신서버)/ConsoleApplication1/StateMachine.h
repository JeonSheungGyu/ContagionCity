#pragma once
#include "State.h"

template <class entity_type>
class StateMachine
{
private:
	// 이 인스턴스를 소유하는 에이전트를 가리키는 포인터
	entity_type*	m_pOwner;
	
	State<entity_type>*	m_pCurrentState;

public:
	StateMachine(entity_type* owner) :m_pOwner(owner),
		m_pCurrentState(nullptr)
	{}

	// FSM accessor
	void SetCurrentState(State<entity_type>* s) { m_pCurrentState = s; }
	State<entity_type>* getCurrentState() const { return m_pCurrentState; }


	void Update() const
	{
		if (m_pCurrentState) m_pCurrentState->Execute(m_pOwner);
	}


	void ChangeState(State<entity_type>* pNewState)
	{
		assert(pNewState && "<StateMachine::ChangeState>: trying to change to a null state");

		// 기존상태의 Exit 메소드 호출
		m_pCurrentState->Exit(m_pOwner);

		// 상태를 새로운 상태로 변경
		m_pCurrentState = pNewState;

		// 새로운 상태의 enter 메소드를 호출
		m_pCurrentState->Enter(m_pOwner);
	}
};