#include "Monster.h"

void Monster::update()
{
	move();
}

void Monster::move()
{
	if (is_move)
	{
		position.x += direction.x*speed;
		position.y += direction.y*speed;

		dist -= speed;
		/*cout << "position.x: " << position.x << endl;
		cout << "position.y: " << position.y << endl;*/
		if (dist <= 0) is_move = false;
	}
}