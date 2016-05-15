#pragma once
#include "Object.h"
#include "TargetProcess.h"
#include "StateMachine.h"
#include "User.h"
#include <atomic>

class Monster : public Object//��������������üȭ.
{
public:
	//action
	TargetProcess<User, Monster>			m_target;
	StateMachine<Monster>*					m_pStateMachine;
	

	Overlap_ex								m_overlapped;
	BYTE									m_currentAction, m_preAction;
	WORD									m_EXP;
public:
	std::atomic<bool> is_alive;


	//������
	Monster(DWORD id, XMFLOAT3 pos);
	// StateMachine accessor
	StateMachine<Monster>*					GetFSM()const { return m_pStateMachine; }
	// Target Process
	TargetProcess<User, Monster>&			getTargetProcess() { return m_target; }
	// action accessor
	void									setAction(const BYTE ac) { m_currentAction = ac; }
	BYTE									getAction()const { return m_currentAction; }
	//�����ൿ���
	void									heartBeat();
	// monster vector accessor
	
	WORD									getEXP() { return m_EXP; }
};
