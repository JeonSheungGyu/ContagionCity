#include "stdafx.h"
#include "Protocol.h"
#include "User.h"
#include "Monster.h"
#include "Sector.h"
#include "Zone.h"
#include "UserViewList.h"
#include "PacketMaker.h"
#include "RequestToDB.h"
#include "DBProcess.h"
#include "CombatCollision.h"
#include "PacketDispatcher.h"
#include "DeadReckoning.h"
//��������
HANDLE hCompletionPort;
User users[MAX_USER];
std::vector<Monster*> monsters;
Zone zone;
DispatcherFuncArray PacketDispatcher[DISPATCHER_FUNC_TYPE];
CollisionFuncArray CollisionProcess[COLLISION_FUNC_TYPE];

void ErrorHandling(char *message);
void error_display(char *msg, int err_no)
{
	WCHAR *lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, err_no,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	printf("%s", msg);
	wprintf(L"����%s\n", lpMsgBuf);
	LocalFree(lpMsgBuf);
}
unsigned int __stdcall CompletionThread(LPVOID pComPort);


//��ó��
void updatePlayerView( DWORD id)
{
	User *user = &users[id];
	assert(id < MAX_USER);
	printf("user Zone Update\n");
	zone.SectorUpdateOfPlayer(id);
	printf("user nearList Update\n");
	user->updateNearList();
	printf("user viewList Update\n");
	user->updateViewList();
	printf("�þ� ������Ʈ �� :%d\n", user->getNearList().size());
}

//��Ŷó��
void SendPacket(int id, unsigned char *packet)
{

	Overlap_ex *over = new Overlap_ex;
	memset(over, 0, sizeof(Overlap_ex));
	over->operation = OP_SEND;
	over->wsabuf.buf = reinterpret_cast<CHAR *>(over->iocp_buffer);
	over->wsabuf.len = packet[0];
	memcpy(over->iocp_buffer, packet, packet[0]);

	int ret = WSASend(users[id].getSession().hClntSock, &over->wsabuf, 1, NULL, 0,
		&over->overlapped, NULL);
	if (0 != ret) {
		int error_no = WSAGetLastError();
		error_display("SendPacket:WSASend", error_no);
		while (true);
	}
}
void ProcessPacket(User *user, unsigned char buf[]) {

	printf("Receive packet from [%d] \n", user->getID());
	PacketDispatcher[*(buf + 1)].Func(reinterpret_cast<char *>(buf), user->getID());	// �Լ������͹迭�� �̿��� ��Ŷó��
	
}
//DBó��
tbb::concurrent_queue<DB_QUERY> DB_Queue;
std::thread *db_thread;
void DataBaseThread()
{
	Overlap_ex* overEx = new Overlap_ex();
	DB_QUERY q;
	SQLHENV henv;
	SQLHDBC hdbc = 0;
	SQLHSTMT hstmt = 0;
	SQLRETURN retcode;

	SQLCHAR * OutConnStr = (SQLCHAR*)malloc(255);
	SQLSMALLINT * OutConnStrLen = (SQLSMALLINT*)malloc(255);

	// Allocate enviroment handle
	retcode = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv);

	// Set the ODBC version enviroment attribute
	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
		retcode = SQLSetEnvAttr(henv, SQL_ATTR_ODBC_VERSION, (void*)SQL_OV_ODBC3, 0);
	// Allocate connection handle
	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
		retcode = SQLAllocHandle(SQL_HANDLE_DBC, henv, &hdbc);
	// set connect timeout to 5 seconds
	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
		SQLSetConnectAttr(hdbc, SQL_LOGIN_TIMEOUT, (SQLPOINTER)5, 0);

	// Connect to data source
	retcode = SQLConnect(hdbc, (SQLWCHAR*)L"ContagionCityMS", SQL_NTS, (SQLWCHAR*)L"sa",
		SQL_NTS, (SQLWCHAR*)L"1q2w3e4r@@", SQL_NTS);


	//// TestCode
	//retcode = SQLConnect(hdbc, (SQLWCHAR*)L"TEST_DB", SQL_NTS, (SQLWCHAR*)L"",
	//	SQL_NTS, (SQLWCHAR*)L"", SQL_NTS);

	// Allocate statetment handle
	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
		printf("DB Connection success");

		// ---------------------- DB_thread loop start --------------------------//
		while (true) {
			ZeroMemory(overEx, sizeof(overEx));
			if (DB_Queue.empty())
			{
				Sleep(1);
				continue;
			}
			DB_Queue.try_pop(q);
			retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);

			if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
				switch (q.type) {
				case DB_QUERY::REQUEST_STATE:
					RequestToDB::RequestState(overEx, q.ID, hstmt);
					break;
				case DB_QUERY::REQUEST_UPDATE:
					RequestToDB::RequestUpdate(overEx, q.ID, hstmt);
					break;
				}
				PostQueuedCompletionStatus(hCompletionPort, 1, q.ID, (OVERLAPPED*)overEx);
			}
		}
		// ------------------------------ thread loop end -----------------------------------
	}
	else { printf("DB Connection fail : %d", retcode); }
	SQLDisconnect(hdbc);
	SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
	SQLFreeHandle(SQL_HANDLE_ENV, henv);
	//delete overEx;
}

//Event ó��
std::priority_queue<EVENT, std::vector<EVENT>, std::less<EVENT> > timer_queue;
CRITICAL_SECTION qCS;
std::thread *timer_thread;
void add_timer(DWORD id, DWORD type, DWORD duration) {
	
	EVENT temp;
	temp.id = id;
	temp.type = type;
	temp.duration = duration;
	temp.startTime = clock();
	temp.endTime = temp.startTime + duration; //����� �ð�

	EnterCriticalSection(&qCS);
	timer_queue.push(temp);
	LeaveCriticalSection(&qCS);
}
void process_event(EVENT k) {
	if (k.id < MAX_USER)
		Object *object = &users[k.id];
	else {
		Monster *monster = monsters.at(k.id - MAX_USER);
		ZeroMemory(&monster->m_overlapped, sizeof(monster->m_overlapped));
		monster->m_overlapped.operation = k.type;
		monster->m_overlapped.duration = k.duration;
		PostQueuedCompletionStatus(hCompletionPort, 1, monster->getID(), (LPOVERLAPPED)&monster->m_overlapped);
	}
}
void TimerThread()
{
	clock_t currTime;
	do {
		Sleep(1);
		do {
			EnterCriticalSection(&qCS);
			if (!timer_queue.empty()) {
				
				EVENT k = timer_queue.top();
				LeaveCriticalSection(&qCS);
			
				currTime = clock();
				//����� �ð��� ��
				if (k.endTime > currTime)
					break;

				EnterCriticalSection(&qCS);
				timer_queue.pop();
				LeaveCriticalSection(&qCS);

				//printf("���̵� : %d, Ÿ�� : %d �̺�Ʈ ó��", k.id, k.type);
				process_event(k);
			}else
				LeaveCriticalSection(&qCS);

		} while (true);
	} while (true);
}

//���� �ʱ�ȭ, ������, ������Ʈ �Ҵ�
void allocateObject()
{
	monsters.reserve(MAX_NPC);
	//// [TEST] ���� ������ġ, -> ���Ŀ� �ٲ�ߵ�
	std::mt19937 gen(12);
	std::uniform_real_distribution<float> mx(0.0, WORLDSIZE-1);
	std::uniform_real_distribution<float> mz(0.0, WORLDSIZE-1);
	DWORD x = 0, z = 0;
	int count = 0;

	// monster �Ҵ� �� ����
	for (int i = MAX_USER; i < MAX_USER + MAX_NPC; ++i)
	{
		x = ((int)(mx(gen) / RECTSIZE))*RECTSIZE;
		z = ((int)(mz(gen) / RECTSIZE))*RECTSIZE;
		monsters.push_back(new Monster(i, XMFLOAT3(x,0,z)));
		zone.SectorUpdateOfMonster(i);
		//add_timer(i, OP_NPC_MOVE, 1000);
	}
}

void InitFunc()
{
	// ��Ŷó�� �Լ�
	PacketDispatcher[CS_MOVE_OBJECT].Func = PacketDispatcher::ObjectMove;	// Ŭ���̾�Ʈ ���ſ�û
	PacketDispatcher[CS_COMBAT_OBJECT].Func = PacketDispatcher::Combat;	
	PacketDispatcher[CS_REQUEST_LOGIN].Func = PacketDispatcher::RequestLogin;

	CollisionProcess[CC_CircleAround].Func = CombatCollision::CircleAround;
	CollisionProcess[CC_CircleFront].Func = CombatCollision::CircleFront;
	CollisionProcess[CC_Eraser].Func = CombatCollision::Eraser;
	CollisionProcess[CC_PointCircle].Func = CombatCollision::PointCircle;
}
void initializeGame() {
	//ť ũ��Ƽ�ü��� �ʱ�ȭ
	InitializeCriticalSection(&qCS);
	//Ÿ�̸� ������
	timer_thread = new std::thread{TimerThread};
	db_thread = new std::thread{ DataBaseThread };

	InitFunc();
}
void releaseGame() {
	DeleteCriticalSection(&qCS);
	timer_thread->join();
	db_thread->join();
	delete timer_thread;
	delete db_thread;
}

int main(int argc, char** argv)
{
	WSADATA wsaData;

	// ������� CompletionPort�� ���޵� Handle

	SYSTEM_INFO SystemInfo;
	/*
	�ý��� ������ ���޵� �����带 �����Ҷ� CPU �� ������ ����
	�����带 ����� ��� �ϱ� ������ ������ ����
	*/
	SOCKADDR_IN servAddr;

	char buffer[BUFSIZE] = { 0,0 };

	SOCKET hServSock;
	int RecvBytes;
	int i, Flags;
	int id;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) /* Load Winsock 2.2 DLL */
		ErrorHandling("WSAStartup() error!");


	//1. Completion Port ����.
	hCompletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);


	GetSystemInfo(&SystemInfo);


	//���ӹ�ġ �� �ʱ�ȭ
	initializeGame();
	allocateObject();
	//2. Completion Port ���� ����� �ϷḦ ����ϴ� �����带 CPU ������ŭ ����.
	for (i = 0; i<SystemInfo.dwNumberOfProcessors; i++)
		_beginthreadex(NULL, 0, CompletionThread, (LPVOID)hCompletionPort, 0, NULL);

	hServSock = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	// ������ ���鶧�� �� ��ø ����� �������� ����� �־�� �Ѵ�.
	servAddr.sin_family = AF_INET;
	servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servAddr.sin_port = htons(GAME_PORT);

	bind(hServSock, (SOCKADDR*)&servAddr, sizeof(servAddr));
	listen(hServSock, 5);

	while (TRUE)
	{
		SOCKET hClntSock;
		SOCKADDR_IN clntAddr;
		int addrLen = sizeof(clntAddr);

		hClntSock = accept(hServSock, (SOCKADDR*)&clntAddr, &addrLen);

		if (hClntSock == -1)
		{
			printf("Connection Fail\n");
			exit(1);
		}

		id = -1;

		for (int i = 0; i < MAX_USER; i++) {
			if (!users[i].isConnected()) {
				users[i].reset();
				users[i].setID(i);
				users[i].setConnected(true);
				users[i].getSession().hClntSock = hClntSock;
				memcpy(&(users[i].getSession().clntAddr), &clntAddr, addrLen);
				id = i;
				break;
			}
		}
		if (id == -1) {
			closesocket(hClntSock);
			continue;
		}

		printf("connect from [%d]\n", users[id].getID());
		CreateIoCompletionPort((HANDLE)hClntSock, hCompletionPort, (DWORD)&users[id], 0);

		Flags = 0;

		//4. ��ø�� �������Է�.
		WSARecv(users[id].getSession().hClntSock, // ������ �Է¼���.
			&(users[id].getSession().recv_overlap.wsabuf),  // ������ �Է� ����������.
			1,       // ������ �Է� ������ ��.
			(LPDWORD)&RecvBytes,
			(LPDWORD)&Flags,
			&(users[id].getSession().recv_overlap.overlapped), // OVERLAPPED ����ü ������.
			NULL);
	}

	releaseGame();
	return 0;
}

//����� �Ϸῡ ���� �������� �ൿ ����
unsigned int __stdcall CompletionThread(LPVOID pComPort)

{
	HANDLE hCompletionPort = (HANDLE)pComPort;
	DWORD iosize;
	DWORD key;
	User *user;
	Overlap_ex *my_overlap;
	DWORD flags;
	while (1)
	{
		BOOL result = GetQueuedCompletionStatus(hCompletionPort,
			&iosize, (LPDWORD)&key, reinterpret_cast<LPOVERLAPPED *>(&my_overlap), INFINITE);

		WORD errorCode = GetLastError();

		if (result == SOCKET_ERROR) error_display("getQueuedCompletionStatus", 0);

		// DeadReckoning
		// ������ ����� �ð��� ���� �ð��� ����ŭ �̵��ϴµ� ���� �ð���� �߸��Ǿ� ����, ������ ���� ���.... ���̴°� ����α⿡ ����
		if (errorCode == WAIT_TIMEOUT)
		{
			if (!DeadReckoning::Instance().is_process) {
				DeadReckoning::Instance().is_process = true;
				DeadReckoning::Instance().Execute();
			}		
			else continue;

			continue;
		}

		if (iosize == 0) //EOF ���۽�.
		{
			user = reinterpret_cast<User*> (key);
			for (auto i = 0; i < MAX_USER; ++i) {
				if (i == user->getID()) continue;
				if (users[i].isConnected()) {
					PacketMaker::instance().RemoveObject(reinterpret_cast<Object*>(&users[i]), user->getID());
				}
			}
			//sector ����
			user->getCurrentSector()->erasePlayer(user->getID());
			user->setConnected(false);
			closesocket(user->getSession().hClntSock);
			continue;
		}

		if (OP_RECV == my_overlap->operation) {
			user = reinterpret_cast<User*> (key);
			unsigned char *buf_ptr = user->getSession().recv_overlap.iocp_buffer;
			int remained = iosize;
			while (0 < remained) {
				if (0 == user->getSession().packet_size)
					user->getSession().packet_size = buf_ptr[0];
				int required = user->getSession().packet_size
					- user->getSession().previous_size;
				if (remained >= required) {
					memcpy(
						user->getSession().packet_buff + user->getSession().previous_size,
						buf_ptr, required);
					ProcessPacket(user, user->getSession().packet_buff);
					buf_ptr += required;
					remained -= required;
					user->getSession().packet_size = 0;
					user->getSession().previous_size = 0;
				}
				else {
					memcpy(user->getSession().packet_buff
						+ user->getSession().previous_size,
						buf_ptr, remained);
					buf_ptr += remained;
					user->getSession().previous_size += remained;
					remained = 0;
				}
			}
			DWORD flags = 0;
			WSARecv(user->getSession().hClntSock,
				&user->getSession().recv_overlap.wsabuf, 1, NULL, &flags,
				&user->getSession().recv_overlap.overlapped, NULL);
			
		}
		else if (OP_SEND == my_overlap->operation) {
			delete my_overlap;
		}
		else if ( OP_NPC_MOVE == my_overlap->operation) {
			Monster *monster = monsters.at(key - MAX_USER);
			//�ν����尡 �ϳ��� ���͸� ó���� ��찡 �����..... concurrency control�� �̰� �ƴѰ����� ������ ����.
			//EnterCriticalSection(&monster->cs);
			//printf("%d ����", key);
			//���� ��ġ ������Ʈ
			//��ġ �̵��Ұ� ������ �̵�
			if (monster->getDeadReckoning())
				monster->ObjectDeadReckoning(my_overlap->duration);
			zone.SectorUpdateOfMonster(monster->getID());
			monster->setCollisionSpherePos(monster->getPos()); // �浹ü�� ������Ʈ (��)
			//���� FSM
			monster->heartBeat();
			//LeaveCriticalSection(&monster->cs);
			//printf("%d ��\n", key);
		}else if (OP_DB_EVENT == my_overlap->operation)
		{
			//DB���� �޾ƿ� ������ ó��
			switch (my_overlap->db_type)
			{
			case DB_QUERY::REQUEST_STATE: DBProcess::RequestState(my_overlap, key); break;
			case DB_QUERY::REQUEST_UPDATE: break;
			}
			continue;
		}
		else {
			printf("operation : %d ", my_overlap->operation);
			printf( "Unknown IOCP event!\n");
			exit(-1);
		}
	}
}

void ErrorHandling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}