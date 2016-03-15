#pragma once


//CPacketSesison�� ��ӹ��� CConnectedSession

class CConnectedSession : public CPacketSession{

public:
	CConnectedSession(VOID);
	virtual ~CConnectedSession(VOID);
	
	//��ü�� �����ϰ� �ٽ� Accept ���·� ������ִ� �Լ�
	//CServerIocp���� OnIoDisconnected�� �Ǿ��� �� ��ü�� �����ϰ� �ٽ� �ʱ�ȭ �� �� ����ϰ� �˴ϴ�.
	BOOL Restart(SOCKET hListenSocket);

	VOID SetConnected(BOOL  bConnected)  { CThreadSync Sync; m_bConnected = bConnected; }
	BOOL GetConnected() { CThreadSync Sync; return m_bConnected; }

private:
	
	BOOL m_bConnected;

};