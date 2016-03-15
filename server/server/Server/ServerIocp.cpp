#include "stdafx.h"
#include "ServerIocp.h"

CServerIocp::CServerIocp(VOID){

}

CServerIocp::~CServerIocp(VOID){

}

BOOL CServerIocp::Begin(VOID){

	//��ӹ��� IOCP��ü�� �����մϴ�.
	if (!CIocp::Begin())
		return FALSE;

	//Listen�� ����� ��ü�� �����մϴ�.
	m_pListen = new CNetworkSession();

	//��ü�� �����մϴ�.
	if (!m_pListen->Begin()){
		//���������� ENd�Լ��� ȣ���ϰ� �����Ѵ�.
		CServerIocp::End();

		return FALSE;
	}

	//TCP�� ����Ѵ�.
	if (!m_pListen->TcpBind()){
		//���������� ENd�Լ��� ȣ���ϰ� �����Ѵ�.
		CServerIocp::End();

		return FALSE;
	}
	
	//��Ʈ 1820���� Listen�� �ϰ�  ����ڼ��� �ִ� 100���� �����Ѵ�.
	if (!m_pListen->Listen(DEFAULT_PORT, MAX_USER)){
		//���������� ENd�Լ��� ȣ���ϰ� �����Ѵ�.
		CServerIocp::End();

		return FALSE;
	}
	//IOCP�� Listen������ ��Ͻ��� �ش�. �׸��� Ű�δ� �ش� ��ü�� �����ͷ� �����Ѵ�.
	if (!CIocp::RegisterSocketToIocp(m_pListen->GetSocket(), reinterpret_cast<ULONG_PTR>(m_pListen))){
		//���������� ENd�Լ��� ȣ���ϰ� �����Ѵ�.
		CServerIocp::End();

		return FALSE;
	}

	//������ ����� accept���°� �˴ϴ�.
	if (!m_oConnectedSessionManager.Begin(m_pListen->GetSocket())){
		CServerIocp::End();
		return FALSE;
	}

	return TRUE;
}

VOID CServerIocp::End(VOID){
	//IOCP�� �����մϴ�.
	CIocp::End();
	m_oConnectedSessionManager.End();
	//�����ߴ� Listen�� ��ü�� �����ϰ� �������ݴϴ�.
	if (m_pListen){
		m_pListen->End();
		delete m_pListen;
	}
}


VOID CServerIocp::OnIoConnected(VOID *pObject){
	//���� ���� �Լ����� �Ѿ�� pObject�� ������ ����� ��ü�� �Ѿ���� �˴ϴ�.
	//�̰��� ���⼭ Ŭ���̾�Ʈ�� ������ CConnectedSessio���� �� ��ȯ�� ���־� �ް� �˴ϴ�.
	//�� ��ȯ�� reinterpret_cast�� ����մϴ�.
	CConnectedSession *pConnectedSession = reinterpret_cast<CConnectedSession*>(pObject);

	if (!CIocp::RegisterSocketToIocp(pConnectedSession->GetSocket(), reinterpret_cast<ULONG_PTR>(pConnectedSession)))
		return;

	//IOCP �ʱ� �ޱ⸦ �������ݴϴ�.
	if (!pConnectedSession->InitializeReadForIocp()){
		//���� �������� ��� ��ü�� ������մϴ�.
		pConnectedSession->Restart(m_pListen->GetSocket());
		return;
	}
}

VOID CServerIocp::OnIoDisconnected(VOID *pObject){
	CConnectedSession *pConnectedSession = reinterpret_cast<CConnectedSession*>(pObject);
	//������ �����Ͽ��� ������ ��ü�� ��������ݴϴ�.
	pConnectedSession->Restart(m_pListen->GetSocket());
}


VOID CServerIocp::OnIoWrote(VOID *pObject, DWORD dwDataLenth){

}

VOID CServerIocp::OnIoRead(VOID *pObject, DWORD dwDataLength){

}