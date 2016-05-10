#pragma once
#pragma warning( once : 4244 ) 
#pragma warning( once : 4290 ) 

#include "Protocol.h"
#include "ObjectStatus.h"


using namespace DirectX;


class Object
{
protected:
	int id;
	//상태
	ObjectStatus objectStatus;
	// 위치벡터, 방향벡터, 거리, 상태, 속도
	XMFLOAT2 position;
	XMFLOAT2 direction;
	XMFLOAT3 targetPos;
	double dist;
	float speed;
	


public:
	std::atomic<bool> is_move = false;
	std::atomic<bool> is_using = false;
	Object();
	Object(const XMFLOAT2& pos, const XMFLOAT2& dir, const float sp)
		:position(pos), direction(dir), speed(sp), is_using(false)
	{}
	Object(const Object& copy)
		:position(copy.position), direction(copy.direction), speed(copy.speed)
	{}
	virtual ~Object(){}

	//accessor
	void setID(const int id){ this->id = id; }
	int getID()const { return id; }

	void setPos(const XMFLOAT2& pos) { position = pos; }
	XMFLOAT2 getPos()const { return position; }

	void setDir(const XMFLOAT2& dir){ direction = dir; }
	XMFLOAT2 getDir()const { return direction; }

	void setSpeed(const float speed){ this->speed = speed; }
	float getSpeed()const { return speed; }

	void setHp(const WORD hp) { objectStatus.hp = hp; }
	void minusHp(const WORD hp) { objectStatus.hp -= hp; }
	const WORD getHp()const { return objectStatus.hp; }


	ObjectStatus& getStatus(){ return objectStatus; }
	void setStatus(const ObjectStatus& state) { objectStatus = state; }

	void setDist(const double dis) { dist = dis; }
	const double getDist()const { return dist; }

	virtual void update() {}
	virtual void move() {}

	void setTargetPos(const XMFLOAT3 tar) { targetPos = tar; }

	
};