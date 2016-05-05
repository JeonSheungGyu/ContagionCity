#pragma once

#include <mutex>
#include "State.h"
#include "Monster.h"

class Die : public State < Monster >
{
private:
	static Die* dInstance;

	static std::mutex pMutex;

	Die() {}
	Die(const Die&copy) = delete;
	Die& operator=(const Die&other) = delete;

public:
	static Die* Instance();

	virtual void Enter(Monster* pMonster);

	virtual void Execute(Monster* pMonster);

	virtual void Exit(Monster* pMonster);

	virtual ~Die();
};