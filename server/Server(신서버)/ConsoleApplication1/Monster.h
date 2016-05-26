#pragma once
#include "Object.h"
#include "TargetProcess.h"
#include "StateMachine.h"
#include "User.h"
#include <atomic>

enum ObjectType_ENEMY { ENEMY_ZOMBIE_MAN = 20, ENEMY_ZOMBIE_WOMAN, ENEMY_BOSS, };


class Monster : public Object//소켓정보를구조체화.
{
public:
	//몬스터타입
	WORD									m_MonType;
	//action
	TargetProcess<User, Monster>			m_target;
	StateMachine<Monster>*					m_pStateMachine;
	
	
	Overlap_ex								m_overlapped;
	BYTE									m_currentAction, m_preAction;
	WORD									m_EXP;
public:
	std::atomic<bool>	is_alive;
	std::atomic<bool>	is_active;
	DWORD				InActiveTime;
	//생성자
	Monster(DWORD id, XMFLOAT3 pos);
	// StateMachine accessor
	StateMachine<Monster>*					GetFSM()const { return m_pStateMachine; }
	// Target Process
	TargetProcess<User, Monster>&			getTargetProcess() { return m_target; }
	// action accessor
	void									setAction(const BYTE ac) { m_currentAction = ac; }
	BYTE									getAction()const { return m_currentAction; }
	//몬스터행동계산
	void									heartBeat();
	//유저여부
	bool									isNearUser();
	//몬스터 경험치 리턴
	WORD									getEXP() { return m_EXP; }
};
