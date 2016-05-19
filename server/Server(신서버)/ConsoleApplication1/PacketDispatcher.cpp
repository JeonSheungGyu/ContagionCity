#include "stdafx.h"
#include "Protocol.h"
#include "PacketDispatcher.h"
#include "PacketMaker.h"
#include "User.h"
#include "CombatCollision.h"
//#include "Monster.h"
//
//const int DB_UPDATE_TIME = 5;	// 5분에 한 번씩 DB UPDATE
//const int DB_UPDATE = 20001;
//



extern User users[MAX_USER];
void updatePlayerView(DWORD id);
extern CollisionFuncArray CollisionProcess[COLLISION_FUNC_TYPE];
void SendPacket(int id, unsigned char *packet);

//// 플레이어가 1초마다 보내는 위치 갱신요청 처리
//// 중간발표 끝나고 정리할때 삭제하기
//void PacketDispatcher::PositionUpdate(char* ptr, const unsigned short id)
//{
//	cs_packet_position_update rPacket;
//	memcpy(&rPacket, ptr, *ptr);
//
//	Server::getPlayers()[id]->setPos(XMFLOAT3(rPacket.x, rPacket.y, rPacket.z));
//	Server::getPlayers()[id]->setDir(XMFLOAT3(rPacket.dx, rPacket.dy, rPacket.dz));
//}
//
//
//void PacketDispatcher::StaticUpdate(char* ptr, const unsigned short id)
//{
//	cs_packet_static_update packet;
//	memcpy(&packet, ptr, *ptr);
//
//	Server::updatePlayerView(id);
//}


void PacketDispatcher::ObjectMove(char* ptr, const unsigned short id)
{
	cs_packet_object_move rPacket;
	sc_packet_move_object sPacket;
	memcpy(&rPacket, ptr, *ptr);

	try {
		auto &player = users[id];

		//if (CollisionMap::Instance().getCollisionMap().at((int)rPacket.tz / NODE_HEIGHT).at((int)rPacket.tx / NODE_WIDTH).is_move == 48)	return;
		/*if (RayCast(player->getPos(), XMFLOAT3(rPacket.dx, rPacket.dy, rPacket.dz), rPacket.dist, Server::obVector[0]))
		cout << "RayCast true" << endl;*/
		//else{

		//클라이언트로부터 수신한 유저 위치로 보정
		//player.setPos(XMFLOAT3(rPacket.x, rPacket.y, rPacket.z));

		/*	
		int x = rPacket.x;
		int z = rPacket.z;
		*/
		//위치갱신
		player.setPos(XMFLOAT3(rPacket.x, 0, rPacket.z));
		//방향설정
		player.setDir(XMFLOAT3(rPacket.dx, 0, rPacket.dz));
		player.setCollisionSpherePos(player.getPos()); // 충돌체 업데이트
		//player.setDeadReckoning(true); // 데드레커닝가동
		//PacketMaker::instance().MoveObject(reinterpret_cast<Object*>(&player), player.getID());
		updatePlayerView(player.getID());

		printf("[%d] SC_POS pos( %f, %f, %f) \n",
			player.getID(), player.getPos().x, player.getPos().y, player.getPos().z);
	}
	catch (std::exception& e) {
		printf("PacketDispatcher::ObjectMove :", e.what());
	}

}


void PacketDispatcher::Combat(char* ptr, const unsigned short id)
{
	cs_packet_combat rPacket;
	sc_packet_combat sPacket;
	memcpy(&rPacket, ptr, *ptr);
	auto &player = users[id];
	std::vector<std::pair<WORD, int>> InfoList;

	// action 충돌체크 종류
	CollisionProcess[rPacket.combatCollision].Func(id, InfoList, rPacket.x, rPacket.z);

	try {

		//데미지를 오브젝트에 저장 ( for 패킷전송 )
		player.combatData.size = InfoList.size();
		for (int i = 0; i < InfoList.size(); ++i)
			player.combatData.InfoList[i] = InfoList[i];
		player.combatData.combatCollision = rPacket.combatCollision;
		player.combatData.x = rPacket.x;
		player.combatData.z = rPacket.z;
		player.setAction(action_combat);
		
		PacketMaker::instance().CombatObject(reinterpret_cast<Object*>(&player), player.getID());
		
		//Combat을 유저들에게 알린다.
		updatePlayerView(id);

		//액션초기화
		player.setAction(-1);
	}
	catch (std::exception& e) {
		printf("PacketDispatcher::Combat %s", e.what());
	}
}

//
//void PacketDispatcher::DummyUpdate(char* ptr, const unsigned short id)
//{
//	cs_packet_dummy_update rPacket;
//	sc_packet_object_move sPacket;
//
//	memcpy(reinterpret_cast<char*>(&rPacket), ptr, *ptr);
//
//	try {
//		//if (!CollisionMap::Instance().getCollisionMap().at((int)rPacket.x / 10).at((int)rPacket.z / 10).is_move) return;
//
//		sPacket.id = id;
//		sPacket.tf = 1;
//		sPacket.tx = rPacket.x;
//		sPacket.ty = rPacket.y;
//		sPacket.tz = rPacket.z;
//		sPacket.type = SC_OBJECT_MOVE;
//		sPacket.size = sizeof(sPacket);
//
//		Server::getPlayers().at(id)->setPos(XMFLOAT3(rPacket.x, rPacket.y, rPacket.z));
//		Server::getPlayers().at(id)->setTargetPos(XMFLOAT3(rPacket.x, rPacket.y, rPacket.z));
//		Server::getPlayers().at(id)->setMoveType(sPacket.tf);
//		Server::getPlayers().at(id)->setAction(action_move);
//		Server::getPlayers().at(id)->setWhirl(false);
//
//		Server::updatePlayerView(id);
//	}
//	catch (exception& e) {
//		cout << "PacketDispatcher::DummyUpdate " << e.what() << endl;
//	}
//}


//void PacketDispatcher::RequestLogin(char* ptr, const unsigned short id)
//{
//
//	cs_packet_request_login rPacket;
//
//	memcpy(reinterpret_cast<char*>(&rPacket), ptr, *ptr);
//
//	PacketMaker::instance().Login(rPacket.id);
//}
//
//// 파티관련 처리
//void PacketDispatcher::PartyInit(char* ptr, const unsigned short id)
//{
//	cs_packet_party_init rPacket;
//	sc_packet_party_init sPacket;
//
//	memcpy(reinterpret_cast<char*>(&rPacket), ptr, *ptr);
//
//	PartyManager::instance().Enter();
//	PartyManager::instance().PartyInit(rPacket.id);
//	PartyManager::instance().Leave();
//
//	try {
//		sPacket.party_id = Server::getPlayers().at(rPacket.id)->getPartyNum();
//		sPacket.type = SC_PARTY_INIT;
//		sPacket.size = sizeof(sPacket);
//	}
//	catch (exception& e) {
//		cout << "PacketDispatcher::PartyInit " << e.what() << endl;
//	}
//}
//
//
//void PacketDispatcher::PartyInvite(char* ptr, const unsigned short id)
//{
//	cs_packet_party_invite rPacket;
//	sc_packet_party_invite sPacket;
//
//	memcpy(reinterpret_cast<char*>(&rPacket), ptr, *ptr);
//
//	sPacket.id = rPacket.target_id;
//	sPacket.type = SC_INVITE_PARTY;
//	sPacket.party_id = rPacket.party_id;
//	sPacket.size = sizeof(sPacket);
//
//	Server::sendPacket(sPacket.id, &sPacket);
//}
//
//
//void PacketDispatcher::PartyAgree(char* ptr, const unsigned short id)
//{
//	cs_packet_inivte_agree rPacket;
//	sc_packet_party_notify sPacket;
//
//	memcpy(reinterpret_cast<char*>(&rPacket), ptr, *ptr);
//
//	PartyManager::instance().Enter();
//	PartyManager::instance().EnterInParty(rPacket.party_id, rPacket.id);
//	PartyManager::instance().Leave();
//
//	sPacket.newPlayer_id = rPacket.id;
//	sPacket.type = SC_PARTY_NEWPLAYER;
//	sPacket.size = sizeof(sPacket);
//
//	for (auto& id : PartyManager::instance().getPartyInfo()[rPacket.party_id].first)
//		Server::sendPacket(id, &sPacket);
//}
//
//
//void PacketDispatcher::PartyDelete(char* ptr, const unsigned short id)
//{
//	cs_packet_party_delete rPacket;
//
//	memcpy(reinterpret_cast<char*>(&rPacket), ptr, *ptr);
//
//	PartyManager::instance().Enter();
//	PartyManager::instance().PartyDelete(rPacket.party_id);
//	PartyManager::instance().Leave();
//}
//
//
//void PacketDispatcher::PartyLeave(char* ptr, const unsigned short id)
//{
//	cs_packet_party_leave packet;
//
//	memcpy(reinterpret_cast<char*>(&packet), ptr, *ptr);
//
//	PartyManager::instance().Enter();
//	PartyManager::instance().LeaveInParty(packet.party_id, packet.id);
//	PartyManager::instance().Leave();
//}


// 클라이언트 -> 서버로 DB 업데이트 요청
//void PacketDispatcher::RequestDBupdate(char* ptr, const unsigned short id)
//{
//	cs_packet_db_update packet;
//
//	memcpy(reinterpret_cast<char*>(&packet), ptr, *ptr);
//
//	try {
//		auto& player = Server::getPlayers().at(packet.id);
//		auto checkTime = duration_cast<minutes>(high_resolution_clock::now() - player->dbTime);
//
//		if (checkTime.count() >= DB_UPDATE_TIME) {
//			DB_QUERY q;
//			q.ID = id;
//			q.type = DB_UPDATE;
//			EnterCriticalSection(&Server::dbCS);
//			Server::getDBQ().push(q);
//			LeaveCriticalSection(&Server::dbCS);
//			player->dbTime = high_resolution_clock::now();
//		}
//	}
//	catch (exception& e) {
//		cout << "PacketDispatcher::RequestDBupdate " << e.what() << endl;
//	}
//}
//
//
//void PacketDispatcher::CharInit(char*ptr, const unsigned short id)
//{
//
//	cout << "PacketDispatcher::CharInit" << endl;
//	cs_packet_char_init packet;
//	memcpy(reinterpret_cast<char*>(&packet), ptr, *ptr);
//
//	PacketMaker::instance().Login(id);
//}
//
//
//void PacketDispatcher::InitMonster(char*ptr, const unsigned short id)
//{
//	int check = 0;
//	if (check++ == 0)
//	{
//		cs_packet_monster_init packet;
//		memcpy(reinterpret_cast<char*>(&packet), ptr, *ptr);
//
//		Server::getMonsters().at(packet.mId - MAX_USER)->SettingMonster(XMFLOAT3(packet.x, packet.y, packet.z), packet.MonType);
//
//		if (packet.mId - MAX_USER == (NUM_OF_NPC - 1))
//		{
//			cout << "hb start" << endl;
//			Server::hbThread = new thread{ Server::heartBeatThread };
//		}
//		//cout << "MonType: " << (int)packet.MonType << endl;
//		//cout << "x: " << packet.x << " y:" << packet.y << " z:" << packet.z << endl;
//
//	}
//}
//
//
//void PacketDispatcher::CollisionObject(char*ptr, const unsigned short id)
//{
//	cs_packet_is_collsion packet;
//
//	memcpy(reinterpret_cast<char*>(&packet), ptr, *ptr);
//
//	auto& player = Server::getPlayers().at(id);
//
//	player->setPos(XMFLOAT3(packet.x, packet.y, packet.z));
//	player->setTargetPos(XMFLOAT3(packet.x, packet.y, packet.z));
//	player->setPrePos(XMFLOAT3(packet.x, packet.y, packet.z));
//	player->setDeadReckoning(false);
//}