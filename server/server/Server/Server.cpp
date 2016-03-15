// Server.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"


int _tmain(int argc, _TCHAR* argv[])
{
	CoInitialize(NULL);
	//Winsock을 사용하기 위한 DLL 로드
	WSADATA WsaData;
	WSAStartup(MAKEWORD(2, 2), &WsaData);

	//서버가 다운 되었을때 처리하는 Dump
	//게임서버의 경우 다운되었을 경우의 상황을 가정하기 힘들다. Dump를 사용하여 그때의 프로세스 스레드 상황과 스택을 저장해야 된다.
	if (!CMiniDump::Begin())
		return 0;


	//서버 시작하는 곳
	CServerIocp ServerIocp;
	if (ServerIocp.Begin()){
		getchar();
		ServerIocp.End();
	}

	//종료시 DLL은 제거합니다.
	WSACleanup();
	CMiniDump::End();
	CoUninitialize();

	return 0;
}

