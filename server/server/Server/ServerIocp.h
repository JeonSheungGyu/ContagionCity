#pragma once

//CIocp를 상속받는다

class CServerIocp : public CIocp{
public:
	CServerIocp(VOID);
	virtual ~CServerIocp(VOID);


	//개체를 초기화하고 시작하기 위한 함수
	BOOL Begin(VOID);

	//개체를 종료하기 위한 함수
	VOID End(VOID);


protected:
	//CIocp 상속 가상 함수들을 재선언한다.
	VOID OnIoRead(VOID *pObject, DWORD dwDataLength);
	VOID OnIoWrote(VOID *pObject, DWORD dwDataLength);
	VOID OnIoConnected(VOID *pObject);
	VOID OnIoDisconnected(VOID *pObject);

private:
	//서버의 Listen을 담당할 개체
	//NetworkSession을 사용한다.
	CNetworkSession *m_pListen;
	CConnectedSessionManager m_oConnectedSessionManager;


};