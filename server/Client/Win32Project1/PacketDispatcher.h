#pragma once

#include "Protocol.h"
class PacketDispatcher
{
public:
	//�α��� ( ���λ�� )
	static void PermisionLogin(char* buf);
	static void Login(char* buf);


	static void PutObject(char* buf);
	static void RemoveObject(char* buf);
	static void MoveObject(char* buf);
	static void ObjectCombat(char* buf);

	static void MonsterAttack(char* buf);
	static void MonsterChase(char* buf);
	static void MonsterDie(char* buf);

	
};