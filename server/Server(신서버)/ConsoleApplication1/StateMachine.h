#pragma once
#include "State.h"

template <class entity_type>
class StateMachine
{
private:
	// �� �ν��Ͻ��� �����ϴ� ������Ʈ�� ����Ű�� ������
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

		// ���������� Exit �޼ҵ� ȣ��
		m_pCurrentState->Exit(m_pOwner);

		// ���¸� ���ο� ���·� ����
		m_pCurrentState = pNewState;

		// ���ο� ������ enter �޼ҵ带 ȣ��
		m_pCurrentState->Enter(m_pOwner);
	}
};