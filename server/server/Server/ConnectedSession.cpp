#include "stdafx.h"
#include "ConnectedSession.h"

CConnectedSession::CConnectedSession(VOID){
	m_bConnected = FALSE;
}

CConnectedSession::~CConnectedSession(VOID){

}

BOOL CConnectedSession::Restart(SOCKET hListenSocket){
	//����ȭ ��ü
	CThreadSync Sync;
	
	//��ü�� �����մϴ�.
	End();

	//��ü�� �ٽ� �����ϰ� Accept���·� ������ݴϴ�.
	//Accept�� �ϱ� ���ؼ��� �ݵ�� hListendSocket ���� �ʿ��մϴ�.
	//�� ���� CServerIocp�� ����� m_pListen�� GetSocket���� �޾ƿɴϴ�.
	
	//������ ������ �ٽ� �Ҵ��ϰ� ListenSocket�� ���� AcceptEx�Լ��� �����մϴ�.
	return Begin() && Accept(hListenSocket);
}