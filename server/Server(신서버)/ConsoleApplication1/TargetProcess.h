#pragma once

//CHASE_FAIL_DIST�� WANDER�� TARGET �ݰ��� ��ġ���� �ʾƵ� �ȴ�.
const int	CHASE_FAIL_DIST = RECTSIZE * 5; // �ѱ�ݰ�
const int   ATTACK_DIST = RECTSIZE*2; // ���ݹݰ�
const int	CRASH_RANGE = RECTSIZE * 5; // Ž���ݰ�

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
	// target���� �Ÿ� ���  ( ���� target�� ��ġ�� ���� ���� or �߰� FSM ���� )
	void CalculateDistWithTarget(Owner* owner);
	// ���� Target�� �����Ǿ� �ִ��� �˻�
	BOOL TargetIsNull();
	void TargetFree();
};

// TargetProcess ����

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
		
	//���� �ǰ� ������ �ش� ��ü�� �״´�.
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

		BoundingSphere crashRange(owner->getPos(), CRASH_RANGE);	// ���ͱ��عݰ� �������

		for (const auto& t_id : owner->getNearList())
		{
			if (t_id < MAX_USER) {
				if (users[t_id].isConnected() == false)continue;
				if (crashRange.Intersects(users[t_id].getCollisionSphere())) {
					// ��������� �� �ش� ���� HP minus
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
