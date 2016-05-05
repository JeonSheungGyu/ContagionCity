#pragma once

#include <mutex>
#include "State.h"
#include "Monster.h"

class Chase : public State < Monster >
{
private:
	static Chase* cInstance;

	static std::mutex pMutex;

	Chase() {}
	Chase(const Chase& copy) = delete;
	Chase& operator=(const Chase& other) = delete;

public:
	static Chase* Instance();

	virtual void Enter(Monster* pMonster);

	virtual void Execute(Monster* pMonster);

	virtual void Exit(Monster* pMonster);

	virtual ~Chase();
};