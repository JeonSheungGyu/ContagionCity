#include "PacketSender.h"
#include "Protocol.h"
#include "User.h"
#include "Server.h"

std::mutex PacketSender::sMutex;
PacketSender* PacketSender::sInstance = nullptr;

extern std::vector<User> users;

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

	send(Server::getSock(), reinterpret_cast<char*>(&packet), packet.size, 0);
}
//게임서버에 ID를 전송한다.
void PacketSender::requestLogin2(char *id)
{
	cs_packet_request_login packet;

	packet.type = CS_REQUEST_LOGIN;
	packet.size = sizeof(packet);
	strncpy(packet.id, id, ID_LEN);
	send(Server::getSock(), reinterpret_cast<char*>(&packet), packet.size, 0);
}
//
void PacketSender::DBUpdate()
{

	cs_packet_db_update packet;

	// User가 담겨 있는 컨테이너 -> concurrent_vector -> 각 공간 동기화 되어있음, 
	// x,z 초기화 짧은 텀 동기화해야될까
	packet.id = Server::getClientID();
	packet.type = CS_DB_UPDATE;
	packet.size = sizeof(packet);

	send(Server::getSock(), reinterpret_cast<char*>(&packet), packet.size, 0);
}


void PacketSender::PlayerMove()
{
	cs_packet_object_move move_packet;
	char buf[BUFSIZE];
	//현재위치와 이동방향 전달
	move_packet.type = CS_MOVE_OBJECT;
	move_packet.size = sizeof(move_packet);
	move_packet.x = users[Server::getClientID()].getPos().x;
	move_packet.y = 0;
	move_packet.z = users[Server::getClientID()].getPos().y;
	move_packet.dx = users[Server::getClientID()].getDir().x;
	move_packet.dy = 0;
	move_packet.dz = users[Server::getClientID()].getDir().y;

	send(Server::getSock(), reinterpret_cast<char*>(&move_packet), move_packet.size, 0);
}
//// 플레이어가 스킬 or 기본공격 시 호출
//// 매개변수: 플레이어가 어떤 키를 눌렀는지 action BYTE값
void PacketSender::PlayerCombat(const char combatCollision, const float x, const float y)
{
	assert(combatCollision >= 0);
	cs_packet_combat packet;

	packet.id = Server::getClientID();
	packet.combatCollision = combatCollision;
	packet.x = x;
	packet.z = y;
	packet.type = CS_COMBAT_OBJECT;
	packet.size = sizeof(packet);

	send(Server::getSock(), reinterpret_cast<char*>(&packet), packet.size, 0);
}

void PacketSender::ChangeStage(WORD stage )
{
	assert( stage < Stages::END);
	cs_packet_change_stage packet;

	packet.id = Server::getClientID();
	packet.stage = stage;
	packet.type = CS_CHANGE_STAGE;
	packet.size = sizeof(packet);

	send(Server::getSock(), reinterpret_cast<char*>(&packet), packet.size, 0);
}


//// 플레이어가 2초 이상 정지하고 있을 때 send
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