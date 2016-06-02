#include "stdafx.h"
#include "Protocol.h"
#include "PartyManager.h"
#include "User.h"

mutex PartyManager::pMutex;
PartyManager* PartyManager::pInstance = nullptr;
void SendPacket(int id, unsigned char *packet);
extern User users[MAX_USER];

PartyManager& PartyManager::instance()
{
	lock_guard<mutex> guard(pMutex);

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
	catch (exception& e) {
		printf("PartyManger::PartyInit : %s", e.what());
	}
}


void PartyManager::EnterInParty(const WORD party_id, const WORD player_id)
{
	assert(player_id >= 0 && party_id >= 0);

	try {
		PartyInfo.at(party_id).first.push_back(player_id);
	}
	catch (exception& e) {
		printf("PartyManger::EnterInParty : %s", e.what());
	}
}


void PartyManager::LeaveInParty(const WORD party_id, const WORD player_id)
{
	assert(player_id >= 0 && party_id >= 0);

	try {
		auto& iter = find(PartyInfo.at(party_id).first.begin(), PartyInfo.at(party_id).first.end(), player_id);
		PartyInfo.at(party_id).first.erase(iter);

		sc_packet_party_notify packet;
		packet.newPlayer_id = player_id;
		packet.type = SC_LEAVE_PARTY;
		packet.size = sizeof(packet);

		for (auto& id : PartyInfo.at(party_id).first)
			SendPacket(id, reinterpret_cast<unsigned char *>(&packet));
	}
	catch (exception& e) {
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
	catch (exception& e) {
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