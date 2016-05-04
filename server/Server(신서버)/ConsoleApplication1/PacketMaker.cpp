#include "stdafx.h"
#include "Protocol.h"
#include "User.h"
#include "Monster.h"
#include "PacketMaker.h"


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


void PacketMaker::PutObject(Object *owner, const DWORD id)
{
	assert(id >= 0);
	sc_packet_put_object packet;

	try {
		assert(id >= 0);
		Object *object;
		if (id < MAX_USER)
			object = &users[id];
		else
			object = monsters.at(id - MAX_USER);

		sc_packet_put_object put_packet;
		put_packet.id = object->getID();
		put_packet.size = sizeof(put_packet);
		put_packet.type = SC_PUT_OBJECT;
		put_packet.x = object->getPos().x;
		put_packet.y = object->getPos().y;
		put_packet.z = object->getPos().z;
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
	sc_packet_pos pos_packet;

	try {
		if (id < MAX_USER)
			object = &users[id];
		else
			object = monsters.at(id - MAX_USER);

		pos_packet.id = object->getID();
		pos_packet.size = sizeof(pos_packet);
		pos_packet.type = SC_POS;
		pos_packet.x = object->getPos().x;
		pos_packet.y = object->getPos().y;
		pos_packet.z = object->getPos().z;

		SendPacket(owner->getID(), reinterpret_cast<unsigned char *>(&pos_packet));
		printf("Send [%d] about  [%d] SC_POS pos( %f, %f, %f) \n", owner->getID(),
			object->getID(), object->getPos().x, object->getPos().y, object->getPos().z);
		//중복패킷 발생가능함. 한명은 뷰를 보고 한명은 tmp를 보고 있으니까
	}
	catch (exception& e) {
		printf("PacketMaker::MoveObject : %s", e.what());
	}
}

