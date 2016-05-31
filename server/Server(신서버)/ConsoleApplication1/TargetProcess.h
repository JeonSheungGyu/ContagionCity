#pragma once

//CHASE_FAIL_DIST와 WANDER의 TARGET 반경이 일치하지 않아도 된다.
const int	CHASE_FAIL_DIST = RECTSIZE * 5; // 쫓기반경
const int   ATTACK_DIST = RECTSIZE*2; // 공격반경
const int	CRASH_RANGE = RECTSIZE * 5; // 탐색반경

enum
{
	wander = 0,
	combat,
	chase,
	die
};


template<class Target_type, class Owner>
class TargetProcess
{
protected:
	Target_type* target;
public:
	TargetProcess();
	// accessor
	Target_type* getTarget() { return target; }
	void setTarget(Target_type* tar) { target = tar; }

	void FindTarget(Owner* owner);
	// target과의 거리 계산  ( 현재 target의 위치에 따라 공격 or 추격 FSM 선택 )
	void CalculateDistWithTarget(Owner* owner);
	// 현재 Target이 지정되어 있는지 검사
	BOOL TargetIsNull();
	void TargetFree();
};

// TargetProcess 정의

template<class Target_type, class Owner>
TargetProcess<Target_type, Owner>::TargetProcess() : target(nullptr)
{}


template<class Target_type, class Owner>
BOOL TargetProcess<Target_type, Owner>::TargetIsNull()
{
	return target == nullptr;
}


template<class Target_type, class Owner>
void TargetProcess<Target_type, Owner>::CalculateDistWithTarget(Owner* owner)
{
	assert(owner &&" TargetProcess owner is nullptr! ");
	if (!target->isConnected()) {
		owner->getTargetProcess().setTarget(nullptr);
		owner->setAction(wander);
		return;
	}
		
	//만약 피가 없으면 해당 객체는 죽는다.
	if (owner->getStatus().hp <= 0) {
		owner->setAction(die);
		return;
	}

	FLOAT dist = sqrt((owner->getPos().x - target->getPos().x) * (owner->getPos().x - target->getPos().x) +
		(owner->getPos().z - target->getPos().z) * (owner->getPos().z - target->getPos().z));

	//printf("[%d] -> [%d] dist : %f", owner->getID(), target->getID(), dist);

	if (dist > CHASE_FAIL_DIST)
		owner->setAction(wander);
	else if (dist > ATTACK_DIST)
		owner->setAction(chase);
	else if ( !owner->getDeadReckoning() )
		owner->setAction(combat);
}

template<class Target_type, class Owner>
void TargetProcess<Target_type, Owner>::FindTarget(Owner* owner)
{
	assert(owner &&" TargetProcess owner is nullptr! ");

	try {

		BoundingSphere crashRange(owner->getPos(), CRASH_RANGE);	// 몬스터기준반경 구만들기

		for (const auto& t_id : owner->getNearList())
		{
			if (t_id < MAX_USER) {
				if (users[t_id].isConnected() == false)continue;
				if (crashRange.Intersects(users[t_id].getCollisionSphere())) {
					// 데미지계산 후 해당 몬스터 HP minus
					/*damage = calculator(player->getState().AP, monsters.at(ID - MAX_USER)->getState().DP,
					player->getState().ElementType, monsters.at(ID - MAX_USER)->getState().ElementType);*/
					//damage = player->getState().AP / 5;

					//monsters.at(ID - MAX_USER)->minusHP(damage);
					//InfoList.push_back(make_pair(ID, monsters.at(ID - MAX_USER)->getState().hp));
					owner->getTargetProcess().setTarget(&users[t_id]);
				}
				else continue;
			}
		}
	}
	catch (std::exception& e) {
		printf("Wander::Execute %s", e.what());
	}
	
}


template<class Target_type, class Owner>
void TargetProcess<Target_type, Owner>::TargetFree()
{
	target = nullptr;
}
