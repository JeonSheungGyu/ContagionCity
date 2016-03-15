#include "stdafx.h"
#include "ConnectedSession.h"

CConnectedSession::CConnectedSession(VOID){
	m_bConnected = FALSE;
}

CConnectedSession::~CConnectedSession(VOID){

}

BOOL CConnectedSession::Restart(SOCKET hListenSocket){
	//동기화 개체
	CThreadSync Sync;
	
	//개체를 종료합니다.
	End();

	//개체를 다시 시작하고 Accept상태로 만들어줍니다.
	//Accept를 하기 위해서는 반드시 hListendSocket 값이 필요합니다.
	//이 값은 CServerIocp의 멤버인 m_pListen의 GetSocket으로 받아옵니다.
	
	//해제한 소켓을 다시 할당하고 ListenSocket에 대한 AcceptEx함수를 실행합니다.
	return Begin() && Accept(hListenSocket);
}