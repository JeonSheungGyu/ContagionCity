#pragma once
#include "Protocol.h"
#include "SyncObject.h"
#include "Sector.h"
#include "ObjectStatus.h"
#include <atomic>

// 나중에 클래스로 바꾸기... 귀차니즘
typedef struct ObjectVector
{
	XMFLOAT3		position;
	XMFLOAT3		oldPos;
	XMFLOAT3		direction;
	FLOAT			dist;

	ObjectVector() : position(0.0, 0.0, 0.0), direction(0.0, 0.0, 0.0), dist(0.0) {}
	ObjectVector(const XMFLOAT3 pos) : position(pos), direction(0.0, 0.0, 0.0), dist(0.0) {}
	void reset(){
		position = XMFLOAT3(0, 0, 0);
		//방향 초기설정 RayCast 때문에
		direction = XMFLOAT3(1, 0, 0);
		dist = 0;
	}
}ObjectVector;

const int COLLISIONSPHERE = 20;

class Object : public SyncObject
{
protected:
	//아이디
	DWORD									id;
	//장소
	WORD									stage;
	//이동
	ObjectVector							obVector;
	float									speed;
	XMFLOAT3								targetPos; // 이동타겟위치
	XMFLOAT3								regenPos;  // 리젠 위치
	//상태
	ObjectStatus							obStatus;
	//시야
	Sector									*sector;
	std::set<Sector*>						nearSectors;
	std::set<DWORD>							nearList;
	//충돌
	BoundingSphere							collisionSphere;

	std::atomic<bool>	 is_DeadReckoning;

public:
	//공격정보 ( 본인전송, 다른유저전송할때 필요, 디스패치에서 저장 )
	CombatData							combatData;

	//////

	Object() : collisionSphere(XMFLOAT3(0,0,0), COLLISIONSPHERE) {
		id = -1;
		stage = -1;
		sector = nullptr;
		speed = 40;
		nearList.clear();
		nearSectors.clear();
	}
	Object( DWORD t_id, XMFLOAT3 pos) : id(t_id), collisionSphere(pos, COLLISIONSPHERE) {
		stage = -1;
		obVector.position = pos;
		regenPos = pos;
		sector = nullptr;
		speed = 40;
		nearList.clear();
		nearSectors.clear();
	}

	void reset() {
		obVector.reset();
		obStatus.reset();
		stage = -1;
		id = -1;
		sector = nullptr;
		speed = 40;
		nearList.clear();
		nearSectors.clear();
	}
	
	// stage
	WORD					getStage()const { return stage; }
	void					setStage(const WORD sg) { stage = sg; }

	//기존의 섹터에서 오브젝트의 정보를 지운다.
	void changeStage(const WORD sg) { 
		if (sector != nullptr) {
			sector->erasePlayer(id);
			sector = nullptr;
		}
		stage = sg; 
	}
	// speed
	FLOAT					getSpeed()const { return speed; }
	void					setSpeed(const FLOAT sp) { speed = sp; }
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


	//oldPos
	const XMFLOAT3							getOldPos()const { return obVector.oldPos; }
	void									setOldPos(const XMFLOAT3 pos) { obVector.oldPos = pos; }

	// targetPos
	const XMFLOAT3&			getTargetPos() { return targetPos; }
	void					setTargetPos(const XMFLOAT3& tp) { targetPos = tp; }
	// regenPos
	const XMFLOAT3&			getRegenPos() { return regenPos; }
	void					setRegenPos(const XMFLOAT3& rp) { regenPos = rp; }

	//데드레커닝
	void					setDeadReckoning(const bool tf) { is_DeadReckoning = tf; }
	bool					getDeadReckoning()const { return is_DeadReckoning; }
	void ObjectDeadReckoning(const float elapsed);
};
