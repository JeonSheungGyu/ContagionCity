#pragma once

#include "SyncObject.h"
#include "Sector.h"
#include "ObjectStatus.h"

// 나중에 클래스로 바꾸기... 귀차니즘
typedef struct ObjectVector
{
	XMFLOAT3		position;
	XMFLOAT3		direction;
	FLOAT			dist;

	ObjectVector() : position(0.0, 0.0, 0.0), direction(0.0, 0.0, 0.0), dist(0.0) {}
	ObjectVector(const XMFLOAT3 pos) : position(pos), direction(0.0, 0.0, 0.0), dist(0.0) {}
	void reset(){
		position = XMFLOAT3(0, 0, 0);
		direction = XMFLOAT3(0, 0, 0);
		dist = 0;
	}
}ObjectVector;

const int COLLISIONSPHERE = 20;

class Object : public SyncObject
{
protected:
	DWORD									id;
	ObjectVector							obVector;
	ObjectStatus							obStatus;
	Sector									*sector;
	std::set<Sector*>						nearSectors;
	std::set<DWORD>							nearList;
	BoundingSphere							collisionSphere;
public:
	Object() : collisionSphere(XMFLOAT3(0,0,0), COLLISIONSPHERE) {
		id = -1;
		sector = nullptr;
		nearList.clear();
		nearSectors.clear();
	}
	Object( DWORD t_id, XMFLOAT3 pos) : id(t_id), collisionSphere(pos, COLLISIONSPHERE) {
		obVector.position = pos;
		sector = nullptr;
		nearList.clear();
		nearSectors.clear();
	}
	void reset() {
		obVector.reset();
		id = -1;
		sector = nullptr;
		nearList.clear();
		nearSectors.clear();
	}
	// collision sphere
	const BoundingSphere&		getCollisionSphere() { return collisionSphere; }
	void						setCollisionSpherePos(const XMFLOAT3& pos) { collisionSphere.Center = pos; }
	// object vector accessor
	const ObjectVector&		getObjectVec() { return obVector; }
	const XMFLOAT3			getPos()const { return obVector.position; }
	const void				setPos(const XMFLOAT3 pos) { obVector.position = pos; }
	const XMFLOAT3			getDir()const { return obVector.direction; }
	void					setDir(const XMFLOAT3 dir) { obVector.direction = dir; }
	FLOAT					getDist()const { return obVector.dist; }
	void					setDist(const FLOAT di) { obVector.dist = di; }
	// object status accessor
	const ObjectStatus&		getStatus() { return obStatus; }
	void					setStatus(const ObjectStatus& state) { obStatus = state; }
	virtual void			minusAP(const DWORD ap){ obStatus.ap -= ap;}
	virtual void			minusHP(const DWORD hp){obStatus.hp -= hp;}
	void					plusHP(const DWORD hp) { obStatus.hp += hp; }

	// player id accessor
	DWORD					getID()const { return id; }
	void					setID(const DWORD id) { this->id = id; }

	//sector
	Sector*					getCurrentSector() { return sector; }
	void					setCurrentSector(Sector* currentSector) { this->sector = currentSector; }

	//near
	const std::set<DWORD>&		getNearList() { return nearList; }
	const std::set<Sector*>&	getNearSectors() { return nearSectors; }
	void updateNearList();
};
