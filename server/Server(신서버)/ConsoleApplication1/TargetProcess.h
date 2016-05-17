#pragma once
const int	CHASE_FAIL_DIST = RECTSIZE * 6;
const int   CHASE_DIST = RECTSIZE*2;

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
	else if (dist > CHASE_DIST)
		owner->setAction(chase);
	else
		owner->setAction(combat);
}


template<class Target_type, class Owner>
void TargetProcess<Target_type, Owner>::TargetFree()
{
	target = nullptr;
}
