#include "PacketSender.h"
#include "Protocol.h"
#include "User.h"

std::mutex PacketSender::sMutex;
PacketSender* PacketSender::sInstance = nullptr;

extern SOCKET hSocket;
extern std::vector<User> users;
extern DWORD myID;

PacketSender& PacketSender::instance()
{
	std::lock_guard<std::mutex> guard(sMutex);
	if (sInstance == nullptr)
		sInstance = new PacketSender;

	return *sInstance;
}


PacketSender::~PacketSender()
{
	delete sInstance;
	sInstance = nullptr;
}

void PacketSender::requestLogin( char *id, char *password )
{
	cl_packet_request_login packet;

	packet.type = CL_REQUEST_LOGIN;
	packet.size = sizeof(packet);
	strncpy(packet.id, id, ID_LEN);
	strncpy(packet.password, password, ID_LEN);

	send(hSocket, reinterpret_cast<char*>(&packet), packet.size, 0);
}
//
//void PacketSender::positionUpdate()
//{
//	cout << "positionUpdate ȣ�� ID: " << GameEngine::getClientID() << endl;
//	cs_packet_position_update packet;
//	auto& user = GameEngine::getUser()[GameEngine::getClientID()];
//
//	// User�� ��� �ִ� �����̳� -> concurrent_vector -> �� ���� ����ȭ �Ǿ�����, 
//	// x,z �ʱ�ȭ ª�� �� ����ȭ�ؾߵɱ�
//	packet.id = GameEngine::getClientID();
//	packet.type = CS_POSITION_UPDATE;
//	packet.x = user.getPos().x;
//	packet.y = 0;
//	packet.z = user.getPos().y;
//	packet.dx = user.getDir().x;
//	packet.dy = 0;
//	packet.dz = user.getDir().y;
//	packet.size = sizeof(packet);
//
//	//cout << "position: " << packet.x << " " << packet.y << " " << packet.z << endl;
//
//	send(GameEngine::getSock(), reinterpret_cast<char*>(&packet), packet.size, 0);
//}
//
//
void PacketSender::PlayerMove()
{
	cs_packet_object_move move_packet;
	char buf[BUFSIZE];
	//������ġ�� �̵����� ����
	move_packet.type = CS_MOVE_OBJECT;
	move_packet.size = sizeof(move_packet);
	move_packet.x = users[myID].getPos().x;
	move_packet.y = 0;
	move_packet.z = users[myID].getPos().y;
	move_packet.dx = users[myID].getDir().x;
	move_packet.dy = 0;
	move_packet.dz = users[myID].getDir().y;

	send(hSocket, reinterpret_cast<char*>(&move_packet), move_packet.size, 0);
}
//// �÷��̾ ��ų or �⺻���� �� ȣ��
//// �Ű�����: �÷��̾ � Ű�� �������� action BYTE��
void PacketSender::PlayerCombat(const char combatCollision, const float x, const float y)
{
	assert(combatCollision >= 0);
	cs_packet_combat packet;

	packet.id = myID;
	packet.combatCollision = combatCollision;
	packet.x = x;
	packet.z = y;
	packet.type = CS_COMBAT_OBJECT;
	packet.size = sizeof(packet);

	send(hSocket, reinterpret_cast<char*>(&packet), packet.size, 0);
}


//// �÷��̾ 2�� �̻� �����ϰ� ���� �� send
//void PacketSender::staticUpdate()
//{
//	cs_packet_static_update packet;
//
//	packet.id = GameEngine::getClientID();
//	packet.type = CS_STATIC_UPDATE;
//	packet.size = sizeof(cs_packet_static_update);
//
//	send(GameEngine::getSock(), reinterpret_cast<char*>(&packet), packet.size, 0);
//}