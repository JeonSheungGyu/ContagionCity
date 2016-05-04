#pragma once


#include "Object.h"
#include "MonsterViewList.h"

#define UP 1
#define DOWN 2
#define LEFT 3
#define RIGHT 4

class Monster : public Object//소켓정보를구조체화.
{
public:
	MonsterViewList viewList;
	Overlap_ex overlapped;

	Monster(DWORD tx, DWORD ty) : viewList(this) {
		x = tx, y = ty;
	}

	void updateViewList() {
		viewList.updateViewList(nearList);
	}

	void move() {
		int dir = rand() % 4 + 1;
		switch (dir)
		{
		case UP: y -= RECTSIZE; break;
		case DOWN: y += RECTSIZE; break;
		case LEFT: x -= RECTSIZE; break;
		case RIGHT: x += RECTSIZE; break;
		default: printf("Unknown type packet received!\n");
			while (true);
		}

		if (y < 0) y = 0;
		if (y >= WORLDSIZE) y = WORLDSIZE - RECTSIZE;
		if (x < 0) x = 0;
		if (x >= WORLDSIZE) x = WORLDSIZE - RECTSIZE;
	}
};
