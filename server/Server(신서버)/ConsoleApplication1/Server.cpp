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
#include "PartyManager.h"
#include "DeadReckoning.h"

//전역변수
HANDLE hCompletionPort;
User users[MAX_USER];
std::vector<Monster*> monsters;
std::vector<Zone*> zone;
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
	wprintf(L"에러%s\n", lpMsgBuf);
	LocalFree(lpMsgBuf);
}
unsigned int __stdcall CompletionThread(LPVOID pComPort);

//뷰처리
void updatePlayerView( DWORD id)
{
	User *user = &users[id];
	assert(id < MAX_USER);
	zone.at(user->getStage())->SectorUpdateOfPlayer(id);
	user->updateNearList();
	user->updateViewList();

	printf("시야안 오브젝트 : %d\n", user->getViewList().getView().size());
}
//패킷처리
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
	PacketDispatcher[*(buf + 1)].Func(reinterpret_cast<char *>(buf), user->getID());	// 함수포인터배열을 이용한 패킷처리
	
}

//DB처리
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

	

	retcode = SQLConnect(hdbc, (SQLWCHAR*)L"ContagionCityMS", SQL_NTS, (SQLWCHAR*)L"sa",
		SQL_NTS, (SQLWCHAR*)L"1q2w3e4r@@", SQL_NTS);
	//// TestCode
	//retcode = SQLConnect(hdbc, (SQLWCHAR*)L"TEST_DB", SQL_NTS, (SQLWCHAR*)L"",
	//	SQL_NTS, (SQLWCHAR*)L"", SQL_NTS);

	// Allocate statetment handle
	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
		printf("DB Connection success");
		retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
		// ---------------------- DB_thread loop start --------------------------//
		while (true) {
			ZeroMemory(overEx, sizeof(overEx));
			if (DB_Queue.empty())
			{
				Sleep(1);
				continue;
			}
			DB_Queue.try_pop(q);
			// Connect to data source
			
			retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);

			if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
				switch (q.type) {
				case DB_QUERY::REQUEST_STATE:
					RequestToDB::RequestState(overEx, q.ID, hstmt);
					break;
				case DB_QUERY::REQUEST_UPDATE:
					RequestToDB::RequestUpdate(overEx, q.ID, hstmt);
					overEx->operation = OP_DB_EVENT;
					overEx->db_type = DB_QUERY::REQUEST_UPDATE;
					break;
				case DB_QUERY::REQUEST_UPDATE_AND_END:
					RequestToDB::RequestUpdate(overEx, q.ID, hstmt);
					overEx->operation = OP_DB_EVENT;
					overEx->db_type = DB_QUERY::REQUEST_UPDATE_AND_END;
					break;
				}
				PostQueuedCompletionStatus(hCompletionPort, 1, q.ID, (OVERLAPPED*)overEx);
			}
			SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
		}
		// ------------------------------ thread loop end -----------------------------------
	}
	else { printf("DB Connection fail : %d", retcode); }
	SQLDisconnect(hdbc);
	SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
	SQLFreeHandle(SQL_HANDLE_ENV, henv);
	//delete overEx;
}

//Event 처리
std::priority_queue<EVENT, std::vector<EVENT>, std::less<EVENT> > timer_queue;
CRITICAL_SECTION qCS;
std::thread *timer_thread;

void add_timer(DWORD id, DWORD type, DWORD duration) {
	
	EVENT temp;
	temp.id = id;
	temp.type = type;
	temp.duration = duration;
	temp.startTime = clock();
	temp.endTime = temp.startTime + duration; //실행될 시간

	EnterCriticalSection(&qCS);
	timer_queue.push(temp);
	LeaveCriticalSection(&qCS);
}
void process_event(EVENT k) {
	if (k.id < MAX_USER) {
	}
	else {
		//몬스터이동
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
				//실행될 시간과 비교
				if (k.endTime > currTime)
					break;

				EnterCriticalSection(&qCS);
				timer_queue.pop();
				LeaveCriticalSection(&qCS);
				//printf("아이디 : %d, 타입 : %d 이벤트 처리", k.id, k.type);
				process_event(k);
			}else
				LeaveCriticalSection(&qCS);

		} while (true);
	} while (true);
}

//게임 초기화, 릴리즈, 오브젝트 할당
void allocateObject()
{
	monsters.reserve(MAX_NPC);
	//// [TEST] 몬스터 랜덤배치, -> 추후에 바꿔야됨
	auto myzone = zone.at(Stages::STAGE_1);
	std::random_device rnd;
	std::uniform_real_distribution<float> mx(myzone->getStartPos().x, myzone->getWidth() + myzone->getStartPos().x);
	std::uniform_real_distribution<float> mz(myzone->getStartPos().z, myzone->getHeight() + myzone->getStartPos().z);
	float x = 0, z = 0;

	//Stage1 보스 몬스터 배치
	monsters.push_back(new Monster(MAX_USER, ENEMY_STAGE1_BOSS, XMFLOAT3(-2000, 0, 1250)));
	monsters[0]->changeStage(Stages::STAGE_1);
	zone.at(monsters[0]->getStage())->SectorUpdateOfMonster(MAX_USER);

	//Stage1 몬스터 배치
	for (int i = MAX_USER+1; i < MAX_USER + MAX_NPC; ++i)
	{
		x = mx(rnd);
		z = mz(rnd);
		if ( i%2 )
			monsters.push_back(new Monster(i,ENEMY_ZOMBIE_MAN, XMFLOAT3(x,0,z)));
		else
			monsters.push_back(new Monster(i, ENEMY_ZOMBIE_WOMAN, XMFLOAT3(x, 0, z)));

		monsters[i-MAX_USER]->changeStage(Stages::STAGE_1);
		zone.at(monsters[i - MAX_USER]->getStage())->SectorUpdateOfMonster(i);
	}
}
void InitFunc()
{
	// 패킷처리 함수
	PacketDispatcher[CS_MOVE_OBJECT].Func = PacketDispatcher::ObjectMove;	// 클라이언트 갱신요청
	PacketDispatcher[CS_COMBAT_OBJECT].Func = PacketDispatcher::Combat;	
	PacketDispatcher[CS_REQUEST_LOGIN].Func = PacketDispatcher::RequestLogin;
	//데이터베이스
	PacketDispatcher[CS_DB_UPDATE].Func = PacketDispatcher::RequestDBupdate;
	//파티
	PacketDispatcher[CS_PARTY_INIT].Func = PacketDispatcher::PartyInit;
	PacketDispatcher[CS_INVITE_PARTY].Func = PacketDispatcher::PartyInvite;
	PacketDispatcher[CS_LEAVE_PARTY].Func = PacketDispatcher::PartyLeave;
	//PacketDispatcher[CS_PARTY_DELETE].Func = PacketDispatcher::PartyDelete;
	PacketDispatcher[CS_PARTY_AGREE].Func = PacketDispatcher::PartyAgree;
	//채팅
	PacketDispatcher[CS_CHAT].Func = nullptr;
	//스테이지
	PacketDispatcher[CS_CHANGE_STAGE].Func = PacketDispatcher::ChangeStage;

	CollisionProcess[CC_CircleAround].Func = CombatCollision::CircleAround;
	CollisionProcess[CC_CircleFront].Func = CombatCollision::CircleFront;
	CollisionProcess[CC_Eraser].Func = CombatCollision::Eraser;
	CollisionProcess[CC_PointCircle].Func = CombatCollision::PointCircle;
	CollisionProcess[ETC_CheckUser].Func = CombatCollision::CheckUser;
}
void initZone() {
	//Villige
	zone.push_back(new Zone(VILLIGE_POS, VILLIGE_WIDTH, VILLIGE_HEIGHT));
	//Stage1
	zone.push_back(new Zone(STAGE1_POS, STAGE1_WIDTH, STAGE1_HEIGHT));
	//Stage2
	//Stage3

}
void initializeGame() {
	//큐 크리티컬섹션 초기화
	InitializeCriticalSection(&qCS);
	//타이머 스레드
	timer_thread = new std::thread{TimerThread};
	db_thread = new std::thread{ DataBaseThread };

	InitFunc();
	initZone();
	allocateObject();
}
void releaseGame() {
	DeleteCriticalSection(&qCS);
	timer_thread->join();
	db_thread->join();
	delete timer_thread;
	delete db_thread;

	for (int i = 0; i < zone.size(); i++)
		delete zone.at(i);
}
int main(int argc, char** argv)
{
	WSADATA wsaData;
	SYSTEM_INFO SystemInfo;
	/*
	시스템 정보가 전달됨 쓰레드를 생성할때 CPU 의 개수에 따라
	쓰레드를 만들어 줘야 하기 때문에 정보를 얻어옴
	*/
	
	SOCKET hServSock;
	SOCKET hClntSock;
	SOCKADDR_IN clntAddr;
	SOCKADDR_IN servAddr;
	int RecvBytes, Flags;
	int addrLen;
	int ConnectedID;

	
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) /* Load Winsock 2.2 DLL */
		ErrorHandling("WSAStartup() error!");


	//1. Completion Port 생성.
	hCompletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);


	GetSystemInfo(&SystemInfo);


	//게임배치 및 초기화
	initializeGame();

	//2. Completion Port 에서 입출력 완료를 대기하는 쓰레드를 CPU 개수만큼 생성.
	for (int i = 0; i<SystemInfo.dwNumberOfProcessors; i++)
		_beginthreadex(NULL, 0, CompletionThread, (LPVOID)hCompletionPort, 0, NULL);

	hServSock = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	// 소켓을 만들때는 꼭 중첩 입출력 형식으로 만들어 주어야 한다.
	servAddr.sin_family = AF_INET;
	servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servAddr.sin_port = htons(GAME_PORT);

	bind(hServSock, (SOCKADDR*)&servAddr, sizeof(servAddr));
	listen(hServSock, 5);

	while (TRUE)
	{
		addrLen = sizeof(clntAddr);
		Flags = 0;
		ConnectedID = -1;

		hClntSock = accept(hServSock, (SOCKADDR*)&clntAddr, &addrLen);

		if (hClntSock == -1)
		{
			printf("Connection Fail\n");
			exit(1);
		}

		for (int i = 0; i < MAX_USER; i++) {
			if (!users[i].isConnected()) {
				users[i].reset();
				users[i].setID(i);
				users[i].setConnected(true);
				users[i].getSession().hClntSock = hClntSock;
				memcpy(&(users[i].getSession().clntAddr), &clntAddr, addrLen);
				users[i].setStage(START_STAGE);
				users[i].setPos(START_POS);
				users[i].setSpeed(300);
			
				ConnectedID = i;
				break;
			}
		}
		if (ConnectedID < 0) {
			closesocket(hClntSock);
			continue;
		}


		printf("[%d] connect \n", users[ConnectedID].getID());
		CreateIoCompletionPort((HANDLE)hClntSock, hCompletionPort, (DWORD)&users[ConnectedID], 0);

	
		WSARecv(users[ConnectedID].getSession().hClntSock, // 데이터 입력소켓.
			&(users[ConnectedID].getSession().recv_overlap.wsabuf),  // 데이터 입력 버퍼포인터.
			1,       // 데이터 입력 버퍼의 수.
			(LPDWORD)&RecvBytes,
			(LPDWORD)&Flags,
			&(users[ConnectedID].getSession().recv_overlap.overlapped), // OVERLAPPED 구조체 포인터.
			NULL);
	}

	releaseGame();
	return 0;
}

//입출력 완료에 따른 쓰레드의 행동 정의
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
		// 이전에 실행된 시간과 현재 시간의 차만큼 이동하는데 현재 시간계산 잘못되어 있음, 하지만 내일 출근.... 보이는건 제대로기에 무시
		if (errorCode == WAIT_TIMEOUT)
		{
			if (!DeadReckoning::Instance().is_process) {
				DeadReckoning::Instance().is_process = true;
				DeadReckoning::Instance().Execute();
			}		
			else continue;

			continue;
		}

		if (iosize == 0) //EOF 전송시.
		{
			user = reinterpret_cast<User*> (key);

			DB_QUERY q;
			q.ID = user->getID();
			q.type = DB_QUERY::REQUEST_UPDATE_AND_END;
			DB_Queue.push(q);

			continue;
		}if (OP_RECV == my_overlap->operation) {
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
			//몬스터 이동
			Monster *monster = monsters.at(key - MAX_USER);
			if (monster->getDeadReckoning())
				monster->ObjectDeadReckoning(my_overlap->duration);
			zone.at(monster->getStage())->SectorUpdateOfMonster(monster->getID());
			monster->setCollisionSpherePos(monster->getPos()); 
			//몬스터 FSM
			monster->heartBeat();

		}else if (OP_DB_EVENT == my_overlap->operation)
		{
			//DB에서 받아온 데이터 처리
			switch (my_overlap->db_type)
			{
			case DB_QUERY::REQUEST_STATE: 
				//체력과 행동풀
				my_overlap->status.hp = my_overlap->status.max_hp;
				my_overlap->status.ap = my_overlap->status.max_ap;
				DBProcess::RequestState(my_overlap, key); 
				updatePlayerView(key);
				PacketMaker::instance().Login(key);
				break;
			case DB_QUERY::REQUEST_UPDATE: break;
			case DB_QUERY::REQUEST_UPDATE_AND_END:
				//DB 업데이트 후 접속종료
				user = &users[key];

				for (auto i = 0; i < MAX_USER; ++i) {
					if (i == user->getID()) continue;
					if (users[i].isConnected()) {
						PacketMaker::instance().RemoveObject(reinterpret_cast<Object*>(&users[i]), user->getID());
					}
				}
				//파티나가기
				if (user->getPartyNum() != -1) {
					PartyManager::instance().Enter();
					PartyManager::instance().LeaveInParty(user->getPartyNum(), user->getID());
					PartyManager::instance().Leave();
				}
				//sector 제거
				user->getCurrentSector()->erasePlayer(user->getID());
				user->setConnected(false);
				closesocket(user->getSession().hClntSock);
				break;
			}
			continue;
		}else {
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