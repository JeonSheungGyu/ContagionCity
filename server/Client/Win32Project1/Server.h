#pragma once
#include <thread>
#include "protocol.h"
#include <string>

using namespace std;

static atomic<bool> isBuiled = false;
//ObjectHandler���� ����� �Լ��� ���� �Լ������� ����ü
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

	//�Լ�
	static FuncProcess Dispatcher[PACKET_TYPE];
	static void DispatcherInit();

	// ��Ŷ���� ������, DB/������ǥ ���� ������
	static void ReceiveThread();
	static void UpdateThread();
	//������
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