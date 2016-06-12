#include "stdafx.h"
#include "Protocol.h"
#include "PartyManager.h"
#include "User.h"

std::mutex PartyManager::pMutex;
PartyManager* PartyManager::pInstance = nullptr;
void SendPacket(int id, unsigned char *packet);
extern User users[MAX_USER];

PartyManager& PartyManager::instance()
{
	std::lock_guard<std::mutex> guard(pMutex);

	if (pInstance == nullptr)
		pInstance = new PartyManager;

	return *pInstance;
}


PartyManager::PartyManager()
{
	PartyInfo.resize(30);
	for (auto & data : PartyInfo) {
		data.first.reserve(5);
		data.second = false;
	}

	InitializeCriticalSection(&pCS);
}


void PartyManager::PartyInit(const WORD player_id)
{
	assert(player_id >= 0);

	int pID = 0;
	try {
		for (; PartyInfo.at(pID).second != false; ++pID);

		PartyInfo.at(pID).first.push_back(player_id);
		PartyInfo.at(pID).second = true;

		users[player_id].setPartyNum(pID);


	}
	catch (std::exception& e) {
		printf("PartyManger::PartyInit : %s", e.what());
	}
}


void PartyManager::EnterInParty(const WORD party_id, const WORD player_id)
{
	assert(player_id >= 0 && party_id >= 0);

	try {
	
		PartyInfo.at(party_id).first.push_back(player_id);
		users[player_id].setPartyNum(party_id);
		
	}
	catch (std::exception& e) {
		printf("PartyManger::EnterInParty : %s", e.what());
	}
}


void PartyManager::LeaveInParty(const WORD party_id, const WORD player_id)
{
	assert(player_id >= 0 && party_id >= 0);

	try {
		//다 지우도록 패킷을 전송
		sc_packet_party_notify packet;
		packet.newPlayer_id = player_id;
		packet.type = SC_LEAVE_PARTY;
		packet.size = sizeof(packet);

		for (auto& id : PartyInfo.at(party_id).first) {
			SendPacket(id, reinterpret_cast<unsigned char *>(&packet));
			printf("[%d] SC_LEAVE_PARTY (player_id : %d) \n", id, player_id);
		}
			
		//파티에서 지운다
		auto& iter = find(PartyInfo.at(party_id).first.begin(), PartyInfo.at(party_id).first.end(), player_id);
		PartyInfo.at(party_id).first.erase(iter);
		users[player_id].setPartyNum(-1);

		//파티에 사람이 없으면 파티제거
		if (PartyInfo.at(party_id).first.size() == 0) {
			PartyInfo.at(party_id).first.clear();
			PartyInfo.at(party_id).second = false;
		}
	}
	catch (std::exception& e) {
		printf("PartyManger::LeaveInParty : %s", e.what());
	}
}


void PartyManager::PartyDelete(const WORD party_id)
{
	assert(party_id >= 0);

	sc_packet_party_delete packet;
	packet.type = SC_PARTY_DELETE;
	packet.size = sizeof(packet);

	try {
		for (auto& id : PartyInfo.at(party_id).first) {
			users[id].setPartyNum(-1);
			SendPacket(id, reinterpret_cast<unsigned char *>(&packet));
		}

		PartyInfo.at(party_id).first.clear();
		PartyInfo.at(party_id).second = false;
	}
	catch (std::exception& e) {
		printf("PartyManger::PartyDelete : %s", e.what());
	}
}


void PartyManager::Enter()
{
	EnterCriticalSection(&pCS);
}


void PartyManager::Leave()
{
	LeaveCriticalSection(&pCS);
}