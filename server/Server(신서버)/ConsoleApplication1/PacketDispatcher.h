#pragma once
const int DISPATCHER_FUNC_TYPE = 5;

// ��Ŷó�� �Լ��� ���� �Լ������� ����ü
struct DispatcherFuncArray
{
	using FuncType = void(*)(char* buf, const unsigned short id);
	FuncType Func;
	DispatcherFuncArray() { Func = nullptr; }
};

class PacketDispatcher
{
public:

	//static void PositionUpdate(char* ptr, const unsigned short id);	// �÷��̾� ��ġ 1�ʸ��� ����

	static void ObjectMove(char* ptr, const unsigned short id);

	static void Combat(char* ptr, const unsigned short id);

	//static void StaticUpdate(char* ptr, const unsigned short id);

	//static void DummyUpdate(char* ptr, const unsigned short id);

	static void RequestLogin(char* ptr, const unsigned short id);

	//static void CharInit(char*ptr, const unsigned short id);

	//// ��Ƽ���� ó��
	//static void PartyInit(char* ptr, const unsigned short id);

	//static void PartyInvite(char* ptr, const unsigned short id);

	//static void PartyAgree(char* ptr, const unsigned short id);

	//static void PartyDelete(char* ptr, const unsigned short id);

	//static void PartyLeave(char* ptr, const unsigned short id);

	static void RequestDBupdate(char* ptr, const unsigned short id);

	//static void InitMonster(char* ptr, const unsigned short id);

	//static void CollisionObject(char* ptr, const unsigned short id);
};