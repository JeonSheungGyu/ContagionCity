#pragma once
const int DISPATCHER_FUNC_TYPE = 5;

// 패킷처리 함수를 담을 함수포인터 구조체
struct DispatcherFuncArray
{
	using FuncType = void(*)(char* buf, const unsigned short id);
	FuncType Func;
	DispatcherFuncArray() { Func = nullptr; }
};

class PacketDispatcher
{
public:

	//static void PositionUpdate(char* ptr, const unsigned short id);	// 플레이어 위치 1초마다 갱신

	static void ObjectMove(char* ptr, const unsigned short id);

	static void Combat(char* ptr, const unsigned short id);

	//static void StaticUpdate(char* ptr, const unsigned short id);

	//static void DummyUpdate(char* ptr, const unsigned short id);

	static void RequestLogin(char* ptr, const unsigned short id);

	//static void CharInit(char*ptr, const unsigned short id);

	//// 파티관련 처리
	//static void PartyInit(char* ptr, const unsigned short id);

	//static void PartyInvite(char* ptr, const unsigned short id);

	//static void PartyAgree(char* ptr, const unsigned short id);

	//static void PartyDelete(char* ptr, const unsigned short id);

	//static void PartyLeave(char* ptr, const unsigned short id);

	static void RequestDBupdate(char* ptr, const unsigned short id);

	//static void InitMonster(char* ptr, const unsigned short id);

	//static void CollisionObject(char* ptr, const unsigned short id);
};