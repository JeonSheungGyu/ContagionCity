#pragma once
#include "stdafx.h"

class PartyManager
{
private:
	typedef std::vector<std::pair<std::vector<WORD>, bool>> PARTY;
	PARTY PartyInfo;

	CRITICAL_SECTION pCS;

	PartyManager();
	PartyManager(const PartyManager& copy) = delete;
	PartyManager& operator=(const PartyManager& other) = delete;

	static PartyManager*	pInstance;
	static std::mutex			pMutex;
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