#pragma once
#include "Object.h"

class Monster : public Object
{
public:
	Monster() {}
	Monster(const XMFLOAT2& pos, const XMFLOAT2& dir, const float sp)
		:Object(pos, dir, sp)
	{}
	Monster(const Monster& copy) 
		:Object(copy.position,copy.direction,copy.speed)
	{}

	virtual void update();
	virtual void move();
	virtual ~Monster() {}
};