#include "stdafx.h"
#include "ConnectedSessionManager.h"

CConnectedSessionManager::CConnectedSessionManager(VOID){

}

CConnectedSessionManager::~CConnectedSessionManager(VOID){

}

BOOL CConnectedSessionManager::Begin(SOCKET hListenSocket){
	CThreadSync Sync;

	//파라미터로 Listen 개체의 소켓을 받습니다.
	if (!hListenSocket)
		return FALSE;

	//최대 접속자만큼 개체를 생성합니다.
	for (DWORD i = 0; i < MAX_USER; i++){
		CConnectedSession *pConnectedSession = new CConnectedSession();
		//생성된 개체를 관리하는 m_vConnectedSession 벡터에 입력합니다.
		m_vConnectedSessions.push_back(pConnectedSession);
	}

	//생성한 개체들을 초기화하고 Accept 상태로 만들어줍니다.
	for (DWORD i = 0; i < MAX_USER; i++){
		//개체 초기화 중 실패할 경우 종료시킵니다.
		if (!m_vConnectedSessions[i]->Begin()){
			CConnectedSessionManager::End();
			return FALSE;
		}

		//개체를 Accept 상태로 변경하다 문제가 있으면 종료시킵니다.
		if (!m_vConnectedSessions[i]->Accept(hListenSocket)){
			CConnectedSessionManager::End();
			return FALSE;
		}
	}
	return TRUE;
}

//ConnectedSessionManager를 종료하는 함수

VOID CConnectedSessionManager::End(VOID){
	//동기화 개체
	CThreadSync Sync;

	//최대 접속자만큼의 개체를 삭제합니다.
	for (DWORD i = 0; i < m_vConnectedSessions.size(); i++){
		//개체 종료
		m_vConnectedSessions[i]->End();
		delete m_vConnectedSessions[i];
	}
	//관리하는 벡터의 내용을 모두 지웁니다.
	m_vConnectedSessions.clear();
}

VOID CConnectedSessionManager::WriteAll(DWORD dwProtocol, BYTE * pData, DWORD dwLength){
	CThreadSync Sync;

	for (DWORD i = 0; i < m_vConnectedSessions.size(); i++){
		//접속이 체크되어 있는 Session에 대해서 KeepAlive 패킷을 날립니다.
		if (m_vConnectedSessions[i]->GetConnected())
			m_vConnectedSessions[i]->WritePacket(dwProtocol, pData, dwLength);

	}
}