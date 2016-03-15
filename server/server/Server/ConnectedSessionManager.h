#pragma once

//ConnectedSession을 관리하는 클래스


class CConnectedSessionManager : public CMultiThreadSync<CConnectedSessionManager>{

public:
	CConnectedSessionManager(VOID);
	~CConnectedSessionManager(VOID);


	BOOL Begin(SOCKET hListenSocket);
	VOID End(VOID);

	//접속된 모든 ConnectedSession에 패킷을 전송하는 함수
	
	VOID WriteAll(DWORD dwProtocol, BYTE *pData, DWORD dwLength);

private:
	std::vector<CConnectedSession*> m_vConnectedSessions;


};