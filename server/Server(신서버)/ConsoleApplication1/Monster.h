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

	Monster(DWORD id, XMFLOAT3 pos) : viewList(this), Object(id, pos) {
	
	}

	void updateViewList() {
		viewList.updateViewList(nearList);
	}

	void move() {
		int dir = rand() % 4 + 1;
		switch (dir)
		{
		case UP:    obVector.position.z -= RECTSIZE; break;
		case DOWN:  obVector.position.z += RECTSIZE; break;
		case LEFT:  obVector.position.x -= RECTSIZE; break;
		case RIGHT:  obVector.position.x += RECTSIZE; break;
		default: printf("Unknown type packet received!\n");
			while (true);
		}

		if (obVector.position.z < 0)  obVector.position.z = 0;
		if (obVector.position.z >= WORLDSIZE)  obVector.position.z = WORLDSIZE - RECTSIZE;
		if (obVector.position.x < 0)  obVector.position.x = 0;
		if (obVector.position.x >= WORLDSIZE)  obVector.position.x = WORLDSIZE - RECTSIZE;
	}
};
