#include "stdafx.h"
#include "Protocol.h"
#include "User.h"
#include "Monster.h"
#include "PacketMaker.h"
#include "CombatCollision.h"

using namespace std;

PacketMaker* PacketMaker::pInstance = nullptr;
std::mutex PacketMaker::pMutex;


extern User users[MAX_USER];
extern std::vector<Monster*> monsters;
void SendPacket(int id, unsigned char *packet);


PacketMaker& PacketMaker::instance()
{
	// 멀티스레드에서 안전한 싱글톤
	std::lock_guard<std::mutex> guard(pMutex);	
	if (pInstance == nullptr)
		pInstance = new PacketMaker;

	return *pInstance;
}


PacketMaker::~PacketMaker()
{
	delete pInstance;
	pInstance = nullptr;
}

void PacketMaker::Login(const unsigned short id)
{
	assert(id >= 0);
	sc_packet_login packet;

	try {
		Object *object = &users[id];

		packet.id = id;
		packet.type = SC_LOGIN;
		packet.size = sizeof(packet);

		packet.x = object->getPos().x;
		packet.y = object->getPos().y;
		packet.z = object->getPos().z;
		packet.speed = object->getSpeed();
		packet.lv = object->getStatus().lv;
		packet.hp = object->getStatus().hp;
		packet.ap = object->getStatus().ap;
		packet.damage = object->getStatus().damage;
		packet.defense = object->getStatus().defense;
		packet.exp = object->getStatus().exp;
		packet.request_exp = object->getStatus().requestEXP;
		//packet.ElementType = player->getEleType();

		SendPacket(object->getID(), reinterpret_cast<unsigned char *>(&packet));
		printf("Send [%d] about SC_LOGIN \n", object->getID());
	}
	catch (exception& e) {
		printf("PacketMaker::Login : %s", e.what());
	}
}


void PacketMaker::PutObject(Object *owner, const DWORD id)
{

	assert(id >= 0);
	Object *object;
	sc_packet_put_object put_packet;

	try {

	

		if (id < MAX_USER)
		{
			object = &users[id];
			put_packet.x = object->getPos().x;
			put_packet.y = object->getPos().y;
			put_packet.z = object->getPos().z;
			put_packet.speed = object->getSpeed();
			put_packet.lv = object->getStatus().lv;
			put_packet.hp = object->getStatus().hp;
			put_packet.ap = object->getStatus().ap;
			put_packet.damage = object->getStatus().damage;
			put_packet.defense = object->getStatus().defense;
			put_packet.exp = object->getStatus().exp;
			put_packet.request_exp = object->getStatus().requestEXP;
			//put_packet.ElementType = player->getEleType();
		}
		else
		{
			object = monsters.at(id - MAX_USER);
			put_packet.x = object->getPos().x;
			put_packet.y = object->getPos().y;
			put_packet.z = object->getPos().z;
			put_packet.speed = object->getSpeed();
			put_packet.lv = object->getStatus().lv;
			put_packet.hp = object->getStatus().hp;
			put_packet.ap = object->getStatus().ap;
			put_packet.damage = object->getStatus().damage;
			put_packet.defense = object->getStatus().defense;
			put_packet.exp = object->getStatus().exp;
			put_packet.request_exp = 0;
			//put_packet.ElementType = object->getEleType();
		}

		put_packet.id = object->getID();
		put_packet.size = sizeof(put_packet);
		put_packet.type = SC_PUT_OBJECT;
	

		SendPacket(owner->getID(), reinterpret_cast<unsigned char *>(&put_packet));
		printf("Send [%d] about [%d] SC_PUT_PLAYER  pos( %f, %f, %f)\n", owner->getID(),
			object->getID(), object->getPos().x, object->getPos().y, object->getPos().z);
		//중복패킷 발생가능함. 한명은 뷰를 보고 한명은 tmp를 보고 있으니까
	}
	catch (exception& e) {
		printf("PacketMaker::putObject : %s", e.what());
	}
}


void PacketMaker::RemoveObject(Object* owner, const DWORD id)
{
	assert(id >= 0);
	sc_packet_remove_object remove_packet;
	try {
		Object *object;
		if (id < MAX_USER)
			object = &users[id];
		else
			object = monsters.at(id - MAX_USER);

		
		remove_packet.id = object->getID();
		remove_packet.size = sizeof(remove_packet);
		remove_packet.type = SC_REMOVE_OBJECT;

		SendPacket(owner->getID(), reinterpret_cast<unsigned char *>(&remove_packet));
		printf("Send [%d] about [%d] SC_REMOVE_OBJECT  pos( %f, %f, %f) \n", owner->getID(),
			object->getID(), object->getPos().x, object->getPos().y, object->getPos().z);
		//중복패킷 발생가능함. 한명은 뷰를 보고 한명은 tmp를 보고 있으니까
	}
	catch (exception& e) {
		printf("PacketMaker::RemoveObject : %s", e.what());
	}
}




void PacketMaker::MoveObject(Object* owner, const DWORD id)
{
	assert(id >= 0);
	Object *object;
	sc_packet_move_object move_packet;

	try {
		if (id < MAX_USER)
			object = &users[id];
		else
			object = monsters.at(id - MAX_USER);

		move_packet.id = object->getID();
		move_packet.size = sizeof(move_packet);
		move_packet.type = SC_MOVE_OBJECT;
		move_packet.tx = object->getPos().x;
		move_packet.ty = object->getPos().y;
		move_packet.tz = object->getPos().z;

		SendPacket(owner->getID(), reinterpret_cast<unsigned char *>(&move_packet));
		printf("Send [%d] about  [%d] SC_POS pos( %f, %f, %f) \n", owner->getID(),
			object->getID(), object->getPos().x, object->getPos().y, object->getPos().z);
	}
	catch (exception& e) {
		printf("PacketMaker::MoveObject : %s", e.what());
	}
}

//몬스터
void PacketMaker::MonsterAttack(Object* player, const unsigned short id)
{
	assert(player && id >= MAX_USER);

	sc_packet_monster_attack packet;

	try {
		auto& monster = monsters.at(id - MAX_USER);

		if (monster->getTargetProcess().getTarget() == nullptr)
			return throw exception("PacketDMaker::MonsterAttack monster target is nullptr!\n");

		packet.mon_id = id;
		packet.type = SC_MONSTER_ATTACK;
		packet.size = sizeof(sc_packet_monster_attack);
		packet.target_id = monster->getTargetProcess().getTarget()->getID();
		packet.damage = monster->getStatus().damage;
	
		SendPacket(player->getID(), reinterpret_cast<unsigned char *>(&packet));
		printf("Send [%d] about  [%d] SC_MONSTER_ATTACK \n", player->getID(), monster->getID());
	}
	catch (exception& e) {
		printf("PacketMaker::MonsterAttack : %s", e.what());
	}
}

//몬스터 쫓기
void PacketMaker::MonsterChase(Object* player, const unsigned short id)
{
	assert(player && id >= MAX_USER);

	sc_packet_monster_chase packet;

	try {
		auto& monster = monsters.at(id - MAX_USER);

		packet.id = id;
		packet.type = SC_MONSTER_CHASE;
		packet.size = sizeof(sc_packet_monster_chase);
		//위치보정을 위해 위치 전송
		packet.x = monster->getPos().x;
		packet.y = monster->getPos().y;
		packet.z = monster->getPos().z;
		//움직일 방향
		packet.dx = monster->getDir().x;
		packet.dy = monster->getDir().y;
		packet.dz = monster->getDir().z;
		packet.dist = monster->getDist();
	
		SendPacket(player->getID(), reinterpret_cast<unsigned char *>(&packet));
		printf("Send [%d] about  [%d] SC_MONSTER_CHASE pos( %f, %f, %f) \n", player->getID(),
			monster->getID(), monster->getPos().x, monster->getPos().y, monster->getPos().z);
	}
	catch (exception& e) {
		printf("PacketMaker::MonsterChase : %s", e.what());
	}
}


void PacketMaker::MonsterDie(Object* player, const unsigned short id)
{
	assert(player && id >= MAX_USER);

	sc_packet_monster_die packet;

	try {
		packet.mon_id = id;

		if (monsters.at(id - MAX_USER)->getTargetProcess().getTarget() == nullptr)
			return throw exception("PacketMaker::MonsterDie monster target is nullptr\n");

		packet.player_id = monsters.at(id - MAX_USER)->getTargetProcess().getTarget()->getID();
		packet.EXP = monsters.at(id - MAX_USER)->getEXP();
		packet.size = sizeof(packet);
		packet.type = SC_MONSTER_DIE;

		SendPacket(player->getID(), reinterpret_cast<unsigned char *>(&packet));
		monsters.at(id - MAX_USER)->getTargetProcess().setTarget(nullptr);
		monsters.at(id - MAX_USER)->is_alive = false;
		printf("Send [%d] about  [%d] SC_MONSTER_DIE \n", player->getID(), id);
	}
	catch (exception& e) {
		printf("PacketMaker::MonsterDie : %s", e.what());
	}
}

//플레이어 공격
void PacketMaker::CombatObject(Object* player, const unsigned short id)
{
	assert(player && id >= 0);

	sc_packet_combat packet;

	try {
		auto& user = users[id];

		for (int i = 0; i < user.combatData.size; i++) {
			packet.InfoList[i] = user.combatData.InfoList[i];
			printf("Send [%d] about  [%d] SC_COMBAT_OBJECT \n", player->getID(), packet.InfoList[i].first);
		}


		packet.combatCollision = user.combatData.combatCollision;
		packet.ListSize = user.combatData.size;
		packet.id = id;
		packet.type = SC_COMBAT_OBJECT;
		packet.size = sizeof(sc_packet_combat);

		if (user.combatData.combatCollision == CC_PointCircle) {
			packet.x = user.combatData.x;
			packet.z = user.combatData.z;
		}
		//user->setAction(-1);

		SendPacket(player->getID(), reinterpret_cast<unsigned char*>( &packet));
		
	}
	catch (exception& e) {
		printf("PacketMaker::ObjectCombat %s", e.what());
	}
}
