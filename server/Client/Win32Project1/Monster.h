#pragma once
#include "Object.h"

class Monster : public Object
{
protected:

	BYTE									m_MonType;
public:
	Monster() {}
	Monster(const XMFLOAT2& pos, const XMFLOAT2& dir, const float sp)
		:Object(pos, dir, sp)
	{}
	Monster(const Monster& copy) 
		:Object(copy.position,copy.direction,copy.speed)
	{}

	//ElementType
	WORD					getMonType()const { return m_MonType; }
	void					setMonType(const BYTE mt) { m_MonType = mt; }


	virtual void update();
	virtual void move();
	virtual ~Monster() {}
};