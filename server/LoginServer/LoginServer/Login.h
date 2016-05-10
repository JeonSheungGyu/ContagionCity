#pragma once
#include <thread>
#include <iostream>
#include <vector>
#include <queue>
#include "Account.h"

using namespace std;

// 패킷처리 함수를 담을 함수포인터 구조체
struct PacketProcess
{
	using FuncType = void(*)(char* buf, const unsigned short id);
	FuncType Func;
	PacketProcess(){ Func = nullptr; }
};

enum DB_TYPE
{
	ID_PW_CHECK
};

// DB 정보를 담을 구조체
struct DB_QUERY
{
	DB_TYPE type;
	WORD accountid;
	char id[20];
	char pw[20];
};


class Login
{
private:
	WSAData wsadata;
	thread m_acceptThread;
	thread m_workerThread;
	thread m_DBThread;
	static HANDLE hIOCP;

	static vector<Account> AccountInfo;
	static queue<DB_QUERY> DB_Queue;
public:
	Login();
	~Login();

	// 패킷 처리부분
	static PacketProcess packetDispatcher[NUM_OF_PACKET_TYPE];

	static bool beforeRecv(int key, DWORD cbTransfered);

	static void workerThread();

	static void acceptThread();

	static void DB_thread();

	// id 이용 패킷전송
	static void sendPacket(int id, void*packet);

	static int getnewClientID();

	// Account vector accessor
	static vector<Account>& getAccount() { return AccountInfo; }

	static HANDLE CreateNewCompletionPort(DWORD dwNumberOfConcurrentThreads);

	static BOOL AssociateDeviceWithCompetionPort(
		HANDLE hCompletionPort, HANDLE hDevice, DWORD dwCompletionPort);

	void InitDispatcherFunc();

	void AccountUsableCheck(const WORD acountid,char* id,char* password);

	static queue<DB_QUERY>& getDBQ(){ return DB_Queue; }



	static CRITICAL_SECTION dbCS;
};