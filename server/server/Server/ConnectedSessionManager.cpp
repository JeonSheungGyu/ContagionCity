#include "stdafx.h"
#include "ConnectedSessionManager.h"

CConnectedSessionManager::CConnectedSessionManager(VOID){

}

CConnectedSessionManager::~CConnectedSessionManager(VOID){

}

BOOL CConnectedSessionManager::Begin(SOCKET hListenSocket){
	CThreadSync Sync;

	//�Ķ���ͷ� Listen ��ü�� ������ �޽��ϴ�.
	if (!hListenSocket)
		return FALSE;

	//�ִ� �����ڸ�ŭ ��ü�� �����մϴ�.
	for (DWORD i = 0; i < MAX_USER; i++){
		CConnectedSession *pConnectedSession = new CConnectedSession();
		//������ ��ü�� �����ϴ� m_vConnectedSession ���Ϳ� �Է��մϴ�.
		m_vConnectedSessions.push_back(pConnectedSession);
	}

	//������ ��ü���� �ʱ�ȭ�ϰ� Accept ���·� ������ݴϴ�.
	for (DWORD i = 0; i < MAX_USER; i++){
		//��ü �ʱ�ȭ �� ������ ��� �����ŵ�ϴ�.
		if (!m_vConnectedSessions[i]->Begin()){
			CConnectedSessionManager::End();
			return FALSE;
		}

		//��ü�� Accept ���·� �����ϴ� ������ ������ �����ŵ�ϴ�.
		if (!m_vConnectedSessions[i]->Accept(hListenSocket)){
			CConnectedSessionManager::End();
			return FALSE;
		}
	}
	return TRUE;
}

//ConnectedSessionManager�� �����ϴ� �Լ�

VOID CConnectedSessionManager::End(VOID){
	//����ȭ ��ü
	CThreadSync Sync;

	//�ִ� �����ڸ�ŭ�� ��ü�� �����մϴ�.
	for (DWORD i = 0; i < m_vConnectedSessions.size(); i++){
		//��ü ����
		m_vConnectedSessions[i]->End();
		delete m_vConnectedSessions[i];
	}
	//�����ϴ� ������ ������ ��� ����ϴ�.
	m_vConnectedSessions.clear();
}

VOID CConnectedSessionManager::WriteAll(DWORD dwProtocol, BYTE * pData, DWORD dwLength){
	CThreadSync Sync;

	for (DWORD i = 0; i < m_vConnectedSessions.size(); i++){
		//������ üũ�Ǿ� �ִ� Session�� ���ؼ� KeepAlive ��Ŷ�� �����ϴ�.
		if (m_vConnectedSessions[i]->GetConnected())
			m_vConnectedSessions[i]->WritePacket(dwProtocol, pData, dwLength);

	}
}