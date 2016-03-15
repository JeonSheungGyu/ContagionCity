#pragma once

//CIocp�� ��ӹ޴´�

class CServerIocp : public CIocp{
public:
	CServerIocp(VOID);
	virtual ~CServerIocp(VOID);


	//��ü�� �ʱ�ȭ�ϰ� �����ϱ� ���� �Լ�
	BOOL Begin(VOID);

	//��ü�� �����ϱ� ���� �Լ�
	VOID End(VOID);


protected:
	//CIocp ��� ���� �Լ����� �缱���Ѵ�.
	VOID OnIoRead(VOID *pObject, DWORD dwDataLength);
	VOID OnIoWrote(VOID *pObject, DWORD dwDataLength);
	VOID OnIoConnected(VOID *pObject);
	VOID OnIoDisconnected(VOID *pObject);

private:
	//������ Listen�� ����� ��ü
	//NetworkSession�� ����Ѵ�.
	CNetworkSession *m_pListen;
	CConnectedSessionManager m_oConnectedSessionManager;


};