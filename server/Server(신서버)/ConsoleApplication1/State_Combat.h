#pragma once

#include <mutex>
#include "State.h"
#include "Monster.h"

class Combat : public State<Monster>
{
private:
	static Combat* cInstance;

	static std::mutex pMutex;

	Combat() {}
	Combat(const Combat& copy) = delete;
	Combat& operator=(const Combat& copy) = delete;

public:
	static Combat* Instance();

	virtual void Enter(Monster* pMonster);

	virtual void Execute(Monster* pMonster);

	virtual void Exit(Monster* pMonster);

	virtual ~Combat();
};