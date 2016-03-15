#include "stdafx.h"
#include "ServerIocp.h"



DWORD WINAPI KeepThreadCallback(LPVOID pParameter){
	CServerIocp *pOwner = (CServerIocp*)pParameter;
	pOwner->KeepThreadCallback();
	return 0;
}

VOID CServerIocp::KeepThreadCallback(VOID){
	DWORD dwKeepAlive = 0xFFFF;


	while (TRUE){
		//종료가 아니면 30초마다 KeepAlive 패킷을 SessionManager의 WriteAll 함수를 사용하여 전송한다.
		DWORD dwResult = WaitForSingleObject(m_hKeepThreadDestroyEvent, 30000);

		if (dwResult == WAIT_OBJECT_0) return;

		//패킷전송
		m_oConnectedSessionManager.WriteAll(0x3000000, (BYTE*)&dwKeepAlive, sizeof(DWORD));
	}
}


CServerIocp::CServerIocp(VOID){

}

CServerIocp::~CServerIocp(VOID){

}

BOOL CServerIocp::Begin(VOID){

	//상속받은 IOCP개체를 시작합니다.
	if (!CIocp::Begin())
		return FALSE;

	//Listen에 사용할 개체를 생성합니다.
	m_pListen = new CNetworkSession();

	//개체를 시작합니다.
	if (!m_pListen->Begin()){
		//실패했을때 ENd함수를 호출하고 종료한다.
		CServerIocp::End();

		return FALSE;
	}

	//TCP로 사용한다.
	if (!m_pListen->TcpBind()){
		//실패했을때 ENd함수를 호출하고 종료한다.
		CServerIocp::End();

		return FALSE;
	}

	//포트 1820으로 Listen을 하고  사용자수를 최대 100으로 설정한다.
	if (!m_pListen->Listen(DEFAULT_PORT, MAX_USER)){
		//실패했을때 ENd함수를 호출하고 종료한다.
		CServerIocp::End();

		return FALSE;
	}
	//IOCP에 Listen소켓을 등록시켜 준다. 그리고 키로는 해당 개체의 포인터로 설정한다.
	if (!CIocp::RegisterSocketToIocp(m_pListen->GetSocket(), reinterpret_cast<ULONG_PTR>(m_pListen))){
		//실패했을때 ENd함수를 호출하고 종료한다.
		CServerIocp::End();

		return FALSE;
	}

	//접속을 만들고 accept상태가 됩니다.
	if (!m_oConnectedSessionManager.Begin(m_pListen->GetSocket())){
		CServerIocp::End();
		return FALSE;
	}

	//KeepAlive
	m_hKeepThreadDestroyEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (!m_hKeepThreadDestroyEvent){
		CServerIocp::End();
		return FALSE;
	}

	//KeepAlive Thread
	m_hKeepThread = CreateThread(NULL, 0, ::KeepThreadCallback, this, 0, NULL);
	if (!m_hKeepThread){
		CServerIocp::End();
		return FALSE;
	}
	return TRUE;
}

VOID CServerIocp::End(VOID){
	//KeepAlive 
	//스레드 종료 및 이벤트 해제
	if (m_hKeepThread){
		SetEvent(m_hKeepThreadDestroyEvent);

		WaitForSingleObject(m_hKeepThread, INFINITE);

		CloseHandle(m_hKeepThread);
		m_hKeepThread = NULL;
	}
	if (m_hKeepThreadDestroyEvent){
		CloseHandle(m_hKeepThreadDestroyEvent);
		m_hKeepThreadDestroyEvent = NULL;
	}
	//IOCP를 종료합니다.
	CIocp::End();
	m_oConnectedSessionManager.End();
	//생성했던 Listen용 개체를 종료하고 삭제해줍니다.
	if (m_pListen){
		m_pListen->End();
		delete m_pListen;
	}
}


VOID CServerIocp::OnIoConnected(VOID *pObject){
	//여기 가상 함수에서 넘어온 pObject는 접속을 담당한 개체가 넘어오게 됩니다.
	//이것을 여기서 클라이언트를 관리할 CConnectedSessio으로 형 변환을 해주어 받게 됩니다.
	//형 변환은 reinterpret_cast를 사용합니다.
	CConnectedSession *pConnectedSession = reinterpret_cast<CConnectedSession*>(pObject);

	if (!CIocp::RegisterSocketToIocp(pConnectedSession->GetSocket(), reinterpret_cast<ULONG_PTR>(pConnectedSession)))
		return;

	//IOCP 초기 받기를 실행해줍니다.
	if (!pConnectedSession->InitializeReadForIocp()){
		//만일 실패했을 경우 개체를 재시작합니다.
		pConnectedSession->Restart(m_pListen->GetSocket());
		return;
	}

	pConnectedSession->SetConnected(TRUE);
}

VOID CServerIocp::OnIoDisconnected(VOID *pObject){
	CConnectedSession *pConnectedSession = reinterpret_cast<CConnectedSession*>(pObject);
	//접속을 종료하였기 떄문에 개체를 재시작해줍니다.
	pConnectedSession->Restart(m_pListen->GetSocket());

	pConnectedSession->SetConnected(FALSE);
}


VOID CServerIocp::OnIoWrote(VOID *pObject, DWORD dwDataLenth){

}

VOID CServerIocp::OnIoRead(VOID *pObject, DWORD dwDataLength){

}

