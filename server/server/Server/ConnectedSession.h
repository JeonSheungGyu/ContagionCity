#pragma once


//CPacketSesison�� ��ӹ��� CConnectedSession

class CConnectedSession : public CPacketSession{

public:
	CConnectedSession(VOID);
	virtual ~CConnectedSession(VOID);
	
	//��ü�� �����ϰ� �ٽ� Accept ���·� ������ִ� �Լ�
	//CServerIocp���� OnIoDisconnected�� �Ǿ��� �� ��ü�� �����ϰ� �ٽ� �ʱ�ȭ �� �� ����ϰ� �˴ϴ�.

	BOOL Restart(SOCKET hListenSocket);

};