#pragma once

//ConnectedSession을 관리하는 클래스


class CConnectedSessionManager : public CMultiThreadSync<CConnectedSessionManager>{

public:
	CConnectedSessionManager(VOID);
	~CConnectedSessionManager(VOID);


	BOOL Begin(SOCKET hListenSocket);
	VOID End(VOID);

private:
	std::vector<CConnectedSession*> m_vConnectedSessions;


};