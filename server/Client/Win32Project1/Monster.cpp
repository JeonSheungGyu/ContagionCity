#include "Monster.h"

void Monster::update()
{
	move();
}

void Monster::move()
{
	if (is_move)
	{
		float NewSpeed = speed*(33 / 1000.0);
		float test = (speed / 33);

		position.x += direction.x*NewSpeed;
		position.y += direction.y*NewSpeed;

		dist -= NewSpeed;
		/*cout << "position.x: " << position.x << endl;
		cout << "position.y: " << position.y << endl;*/
		if (dist <= 0) is_move = false;
	}
}