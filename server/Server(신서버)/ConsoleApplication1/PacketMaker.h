#pragma once
#include <mutex>
#include "Object.h"
#include "User.h"
// 패킷 생성과정이 코드마다 분산되어 있으면 찾기도 힘들고 가독성 떨어짐
// 패킷 생성하는 부분 클래스로 캡슐화

class PacketMaker
{
public:
	static PacketMaker& instance();

	void Login(const unsigned short id);

	void PutObject(Object *owner, const DWORD id);

	void RemoveObject(Object *owner, const DWORD id);

	void MoveObject(Object *owner, const DWORD id);

	void MonsterAttack(Object* player, const unsigned short id);

	void MonsterChase(Object* player, const unsigned short id);

	void MonsterDie(Object* player, const unsigned short id);

	void CombatObject(Object* player, const unsigned short id);

	void PartyInfo(User* player, const unsigned int damage);
private:
	static PacketMaker* pInstance;
	static std::mutex pMutex;

	PacketMaker() {}
	PacketMaker(const PacketMaker& copy) = delete;
	PacketMaker(const PacketMaker&& copy) = delete;
	PacketMaker& operator=(const PacketMaker& copy) = delete;
	PacketMaker& operator=(const PacketMaker&& copy) = delete;
	virtual ~PacketMaker();
};