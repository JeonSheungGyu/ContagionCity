// Server.cpp : �ܼ� ���� ���α׷��� ���� �������� �����մϴ�.
//

#include "stdafx.h"

DWORD	DEFAULT_GAMETIME = 0;
DWORD	DEFAULT_MAX_HP = 50;
DWORD	DEFAULT_MAX_AP = 50;


#include "Character.h"
#include "ConnectedUser.h"
#include "ConnectedUserManager.h"


int _tmain(int argc, _TCHAR* argv[])
{

	CoInitialize(NULL);

	// Winsock�� ����ϱ� ���� DLL �ε�
	WSADATA WsaData;
	WSAStartup(MAKEWORD(2, 2), &WsaData);	// 2.2 ������ �ε��մϴ�.
  
	// ������ ũ���� �Ǿ����� ó���� �� �ְ� �ϴ� MiniDump
	if (!CMiniDump::Begin())
		return 0;

	// ������ �����ϴ°�
	CGameIocp *GameIocp = new CGameIocp();
	if (GameIocp->Begin())
	{
		getchar();
		GameIocp->End();
	}

	// ����� DLL�� ��ε� ��ŵ�ϴ�.
	WSACleanup();
	CMiniDump::End();	// MiniDump�� �����ϴ�.
	CoUninitialize();

	return 0;
}

