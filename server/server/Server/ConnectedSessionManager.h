#pragma once

//ConnectedSession�� �����ϴ� Ŭ����


class CConnectedSessionManager : public CMultiThreadSync<CConnectedSessionManager>{

public:
	CConnectedSessionManager(VOID);
	~CConnectedSessionManager(VOID);


	BOOL Begin(SOCKET hListenSocket);
	VOID End(VOID);

private:
	std::vector<CConnectedSession*> m_vConnectedSessions;


};