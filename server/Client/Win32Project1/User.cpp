#pragma warning( once : 4244 ) 

#include "User.h"
void User::update()
{
	move();
}

void User::move()
{
	//���⺤�� * ���ǵ� �Ÿ���ŭ �̵�
	//�̵��� �ٵǸ� �����.
	if (is_move)
	{
		position.x += direction.x*speed;
		position.y += direction.y*speed;
		dist -= speed;
		
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