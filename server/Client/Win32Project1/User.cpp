#pragma warning( once : 4244 ) 

#include "User.h"
void User::update()
{
	move();
}

void User::move()
{
	//방향벡터 * 스피드 거리만큼 이동
	//이동이 다되면 멈춘다.
	if (is_move)
	{
		position.x += direction.x*(speed / 33);
		position.y += direction.y*(speed / 33);

		dist -= (speed / 33);
		
		if (dist <= 0) is_move = false;
	}
}


void User::plusEXP(const FLOAT exp)
{
	objectStatus.exp += exp;

	if (objectStatus.exp >= objectStatus.requestEXP){
		objectStatus.lv += 1;
		objectStatus *= 2;
		objectStatus.exp = 0;
	}
}