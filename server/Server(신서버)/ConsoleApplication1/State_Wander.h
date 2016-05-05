#pragma once

#include <mutex>
#include "State.h"
#include "Monster.h"

class Wander : public State<Monster>
{
private:
	static Wander* wInstance;

	static std::mutex pMutex;

	Wander() {}
	Wander(const Wander& copy) = delete;
	Wander& operator=(const Wander& copy) = delete;

public:
	static Wander* Instance();

	virtual void Enter(Monster* pMonster);

	virtual void Execute(Monster* pMonster);

	virtual void Exit(Monster* pMonster);

	virtual ~Wander();
};