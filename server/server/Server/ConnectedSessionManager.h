#pragma once

//ConnectedSession�� �����ϴ� Ŭ����


class CConnectedSessionManager : public CMultiThreadSync<CConnectedSessionManager>{

public:
	CConnectedSessionManager(VOID);
	~CConnectedSessionManager(VOID);


	BOOL Begin(SOCKET hListenSocket);
	VOID End(VOID);

	//���ӵ� ��� ConnectedSession�� ��Ŷ�� �����ϴ� �Լ�
	
	VOID WriteAll(DWORD dwProtocol, BYTE *pData, DWORD dwLength);

private:
	std::vector<CConnectedSession*> m_vConnectedSessions;


};