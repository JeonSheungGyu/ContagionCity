// Server.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"


int _tmain(int argc, _TCHAR* argv[])
{
	CoInitialize(NULL);
	//Winsock�� ����ϱ� ���� DLL �ε�
	WSADATA WsaData;
	WSAStartup(MAKEWORD(2, 2), &WsaData);

	//������ �ٿ� �Ǿ����� ó���ϴ� Dump
	//���Ӽ����� ��� �ٿ�Ǿ��� ����� ��Ȳ�� �����ϱ� �����. Dump�� ����Ͽ� �׶��� ���μ��� ������ ��Ȳ�� ������ �����ؾ� �ȴ�.
	if (!CMiniDump::Begin())
		return 0;


	//���� �����ϴ� ��
	CServerIocp ServerIocp;
	if (ServerIocp.Begin()){
		getchar();
		ServerIocp.End();
	}

	//����� DLL�� �����մϴ�.
	WSACleanup();
	CMiniDump::End();
	CoUninitialize();

	return 0;
}

