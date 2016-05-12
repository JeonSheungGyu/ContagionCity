#include "PacketSender.h"
#include "Protocol.h"

std::mutex PacketSender::sMutex;
PacketSender* PacketSender::sInstance = nullptr;

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

void PacketSender::requestLogin( SOCKET sock, char *id, char *password )
{
	cl_packet_request_login packet;

	packet.type = CL_REQUEST_LOGIN;
	packet.size = sizeof(packet);
	strncpy(packet.id, id, ID_LEN);
	strncpy(packet.password, password, ID_LEN);

	send(sock, reinterpret_cast<char*>(&packet), packet.size, 0);
}
//
//void PacketSender::positionUpdate()
//{
//	cout << "positionUpdate 호출 ID: " << GameEngine::getClientID() << endl;
//	cs_packet_position_update packet;
//	auto& user = GameEngine::getUser()[GameEngine::getClientID()];
//
//	// User가 담겨 있는 컨테이너 -> concurrent_vector -> 각 공간 동기화 되어있음, 
//	// x,z 초기화 짧은 텀 동기화해야될까
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
//void PacketSender::playerMove(const float tx, const float ty)
//{
//	cout << "playerMove::Client ID:" << GameEngine::getClientID() << endl;
//	cs_packet_object_move packet;
//	auto& user = GameEngine::getUser()[GameEngine::getClientID()];
//
//	packet.id = GameEngine::getClientID();
//	packet.tx = tx;
//	packet.ty = 0;
//	packet.tz = ty;
//
//	packet.type = CS_OBJECT_MOVE;
//	packet.size = sizeof(packet);
//
//	send(GameEngine::getSock(), reinterpret_cast<char*>(&packet), packet.size, 0);
//}
//
//
//// 플레이어가 스킬 or 기본공격 시 호출
//// 매개변수: 플레이어가 어떤 키를 눌렀는지 action BYTE값
//void PacketSender::playerCombat(const char action, const float x, const float y)
//{
//	cout << "Client playerCombat 호출" << endl;
//	assert(action >= 0);
//	cs_packet_combat packet;
//
//	packet.id = GameEngine::getClientID();
//	packet.action = action;
//	packet.x = x;
//	packet.z = y;
//	packet.type = CS_COMBAT;
//	packet.size = sizeof(packet);
//
//	send(GameEngine::getSock(), reinterpret_cast<char*>(&packet), packet.size, 0);
//}
//
//
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