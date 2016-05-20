#pragma once
#include <thread>
#include "protocol.h"
#include <string>

using namespace std;

static atomic<bool> isBuiled = false;
//ObjectHandler에서 사용할 함수를 담을 함수포인터 구조체
struct FuncProcess
{
	using FuncType = void(*)(char*);
	FuncType Func;
	FuncProcess() { Func = nullptr; }

};

class Server
{
private:
	static WSADATA		wsa;
	static WORD			clientID;
	static char			SERVERIP[20];
	static char			UserID[20];

public:
	
	static atomic<int> is_set;
	static atomic<int> login_success;
	static SOCKET		sock;

	//함수
	static FuncProcess Dispatcher[PACKET_TYPE];
	static void DispatcherInit();

	// 패킷수신 스레드, DB/현재좌표 갱신 스레드
	static void ReceiveThread();
	static void UpdateThread();
	//스레드
	static thread* receiveThread;
	static thread* updateThread;
	// connect
	static BOOL LoginServerConnection(const string& pId, const string& pPassWord, const string& pIp);
	static void GameServerConnection();

	// accessor
	static SOCKET& getSock() { return sock; }
	static void setClientID(const WORD id) { clientID = id; }
	static WORD getClientID() { return clientID; }


};