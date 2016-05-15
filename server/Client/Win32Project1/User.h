#pragma once
#include "Object.h"

class User : public Object
{
public:
	User() : Object() { }
	User(const XMFLOAT2& pos, const XMFLOAT2& dir, const float sp)
		: Object(pos,dir,sp) 
	{}
	User(const User& copy)
		: Object(copy.position,copy.direction, copy.speed)
	{}


	virtual void update();
	virtual void move();

	virtual ~User() {}

	void plusEXP(const FLOAT exp);
};

