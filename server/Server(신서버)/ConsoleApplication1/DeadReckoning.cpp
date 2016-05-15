#include "stdafx.h"
#include "DeadReckoning.h"
#include "User.h"

DeadReckoning*		DeadReckoning::dInstance = nullptr;
std::mutex				DeadReckoning::dMutex;

extern User users[MAX_USER];

DeadReckoning& DeadReckoning::Instance()
{
	std::lock_guard<std::mutex>	guard(dMutex);

	if (dInstance == nullptr)
		dInstance = new DeadReckoning;

	return *dInstance;
}


DeadReckoning::DeadReckoning()
{
	ObjectList.reserve(1000);
	preTimePoint = std::chrono::system_clock::now();
	is_process = false;
}


void DeadReckoning::setList(const WORD id)
{
	if (!is_exist[id]) {
		is_exist[id] = 1;
		ObjectList.push_back(id);
	}
	else return;
}


void DeadReckoning::Execute()
{
	// 실제 플레이하는 id 0,1 및 보스몬스터만 DeadReckoning -> 성능을 위한 사기ㅜㅜ
	//////////////////////// 고쳐야됨///////////////////////////////
	auto elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - preTimePoint);
	preTimePoint = std::chrono::system_clock::now();
	for (int i = 0; i < 1; ++i) {
		try {
			if (users[i].isConnected()) {
				if (users[i].getDeadReckoning()) {	
					users[i].ObjectDeadReckoning(elapsedTime.count());
				}
			}
		}
		catch (std::exception& e) {
			printf("DeadReckoning::Execute ", e.what());
		}
	}
	is_process = false;
}


