#pragma once
#include "Object.h"
#include "MonsterViewList.h"
#include "TargetProcess.h"
#include "StateHeader.h"
#include "StateMachine.h"
#include "User.h"
#include <atomic>

class Monster : public Object//��������������üȭ.
{
public:
	//action
	TargetProcess<User, Monster>			m_target;
	StateMachine<Monster>*					m_pStateMachine;
	
	//viewlist
	MonsterViewList							m_viewList;
	Overlap_ex								m_overlapped;
	bool									m_isAlive;
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
	//�丮��Ʈ ������Ʈ
	void updateViewList();
	//�����ൿ���
	void heartBeat();

	WORD									getEXP() { return m_EXP; }
};
