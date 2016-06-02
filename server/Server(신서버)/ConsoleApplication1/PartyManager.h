#pragma once
#include <atomic>
#include <WinSock2.h>
#include <vector>
#include <mutex>

using namespace std;

class PartyManager
{
private:
	typedef vector<pair<vector<WORD>, bool>> PARTY;
	PARTY PartyInfo;

	CRITICAL_SECTION pCS;

	PartyManager();
	PartyManager(const PartyManager& copy) = delete;
	PartyManager& operator=(const PartyManager& other) = delete;

	static PartyManager*	pInstance;
	static mutex			pMutex;
public:
	static PartyManager&	instance();

	void					PartyInit(const WORD player_id);
	void					EnterInParty(const WORD party_id, const WORD player_id);
	void					LeaveInParty(const WORD party_id, const WORD player_id);
	void					PartyDelete(const WORD party_id);

	// accessor
	PARTY					getPartyInfo() { return PartyInfo; }

	void					Enter();
	void					Leave();
};