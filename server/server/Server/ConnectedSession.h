#pragma once


//CPacketSesison을 상속받은 CConnectedSession

class CConnectedSession : public CPacketSession{

public:
	CConnectedSession(VOID);
	virtual ~CConnectedSession(VOID);
	
	//개체를 종료하고 다시 Accept 상태로 만들어주는 함수
	//CServerIocp에서 OnIoDisconnected가 되었을 때 개체를 종료하고 다시 초기화 할 때 사용하게 됩니다.
	BOOL Restart(SOCKET hListenSocket);

	VOID SetConnected(BOOL  bConnected)  { CThreadSync Sync; m_bConnected = bConnected; }
	BOOL GetConnected() { CThreadSync Sync; return m_bConnected; }

private:
	
	BOOL m_bConnected;

};