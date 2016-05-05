#include "stdafx.h"
#include "Protocol.h"
#include "User.h"
#include "Monster.h"
#include "Sector.h"
#include "Zone.h"
#include "UserViewList.h"
#include "PacketMaker.h"

//��������
HANDLE hCompletionPort;
User users[MAX_USER];
std::vector<Monster*> monsters;
Zone zone;


typedef struct EVENT {
	DWORD id;
	DWORD type;
	DWORD duration; // after
	std::chrono::system_clock::time_point startTime;

	bool operator < (const EVENT& e) const { return  startTime < e.startTime; }
}EVENT;


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
unsigned int __stdcall CompletionThread(LPVOID pComPort);
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
}
void updateMonsterView(DWORD id)
{
	
	assert(id >= MAX_USER);
	Monster *monster = monsters.at(id - MAX_USER);
	//printf("monster Zone Update\n");
	zone.SectorUpdateOfMonster(id);
	//printf("monster nearList Update\n");
	monster->updateNearList();
	//printf("monster viewList Update\n");
	monster->updateViewList();
}

void ProcessPacket(User *user, unsigned char buf[]) {

	printf("Receive %d from [%d] \n", buf[1], user->getID());
	
	int x = user->getPos().x;
	int z = user->getPos().z;

	switch (buf[1])
	{
	case CS_UP: z-= RECTSIZE; break;
	case CS_DOWN: z+= RECTSIZE; break;
	case CS_LEFT: x-= RECTSIZE; break;
	case CS_RIGHT: x+= RECTSIZE; break;
	default: printf( "Unknown type packet received!\n");
		while (true);
	}
	if (z < 0) z = 0;
	if (z >= WORLDSIZE ) z = WORLDSIZE - RECTSIZE;
	if (x < 0) x = 0;
	if (x >= WORLDSIZE ) x = WORLDSIZE - RECTSIZE;


	user->setPos(XMFLOAT3(x, user->getPos().y, z));
	user->setCollisionSpherePos(user->getPos()); // �浹ü ������Ʈ
	PacketMaker::instance().MoveObject(reinterpret_cast<Object*>(user), user->getID());
	updatePlayerView(user->getID());
}


void ErrorHandling(char *message);

std::priority_queue<EVENT, std::vector<EVENT>, std::less<EVENT> > timer_queue;
CRITICAL_SECTION qCS;
std::thread *timer_thread;
std::thread *monster_thread;

void add_timer(DWORD id, DWORD type, DWORD duration) {
	
	EVENT temp;
	temp.id = id;
	temp.type = type;
	temp.duration = duration;
	temp.startTime = std::chrono::system_clock::now();

	EnterCriticalSection(&qCS);
	timer_queue.push(temp);
	LeaveCriticalSection(&qCS);
}
/*
	IOCP���� ���� �����͸� ����ó�� ���� �ʴ´�.
*/
void process_event(EVENT k) {
	
	if (k.id < MAX_USER)
		Object *object = &users[k.id];
	else {
		Monster *monster = monsters.at(k.id - MAX_USER);
		ZeroMemory(&monster->m_overlapped, sizeof(monster->m_overlapped));
		monster->m_overlapped.operation = k.type;
		PostQueuedCompletionStatus(hCompletionPort, 1, monster->getID(), (LPOVERLAPPED)&monster->m_overlapped);
	}
}
void TimerThread()
{
	std::chrono::system_clock::time_point currTime;
	std::chrono::milliseconds mill;
	do {
		Sleep(1);
		do {
			EnterCriticalSection(&qCS);
			if (!timer_queue.empty()) {
				
				EVENT k = timer_queue.top();
				LeaveCriticalSection(&qCS);

				currTime = std::chrono::system_clock::now();
				mill = std::chrono::duration_cast<std::chrono::milliseconds>(currTime - k.startTime);
				
				if (k.duration > mill.count())
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

void MonsterEventThread()
{
	std::chrono::system_clock::time_point startTime, endTime;
	while (true)
	{
		startTime = std::chrono::system_clock::now();
		std::chrono::milliseconds mill;
		for (auto& monster : monsters)
		{
			add_timer(monster->getID(), OP_NPC_MOVE, 0);
		}
		endTime = std::chrono::system_clock::now();
		mill = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
		

		if (MONSTER_DURATION > mill.count())
			Sleep(MONSTER_DURATION - mill.count());
	}
}

/*
������Ʈ ��ġ
*/
void allocateObject()
{
	monsters.reserve(MAX_NPC);
	//// [TEST] ���� ������ġ, -> ���Ŀ� �ٲ�ߵ�
	std::mt19937 gen(12);
	std::uniform_real_distribution<float> mx(0.0, WORLDSIZE-1);
	std::uniform_real_distribution<float> mz(0.0, WORLDSIZE-1);
	DWORD x = 0, z = 0;
	int count = 0;

	// monster �Ҵ�
	for (int i = MAX_USER; i < MAX_USER + MAX_NPC; ++i)
	{
		x = ((int)(mx(gen) / RECTSIZE))*RECTSIZE;
		z = ((int)(mz(gen) / RECTSIZE))*RECTSIZE;
		monsters.push_back(new Monster(i, XMFLOAT3(x,0,z)));
		updateMonsterView(i);
	}
}

void initializeGame() {
	//ť ũ��Ƽ�ü��� �ʱ�ȭ
	InitializeCriticalSection(&qCS);
	//Ÿ�̸� ������
	timer_thread = new std::thread{TimerThread};
	monster_thread = new std::thread{ MonsterEventThread };

}
void releaseGame() {
	DeleteCriticalSection(&qCS);
	timer_thread->join();
	monster_thread->join();
	delete timer_thread;
	delete monster_thread;
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
	allocateObject();
	initializeGame();

	//2. Completion Port ���� ����� �ϷḦ ����ϴ� �����带 CPU ������ŭ ����.
	for (i = 0; i<SystemInfo.dwNumberOfProcessors; i++)
		_beginthreadex(NULL, 0, CompletionThread, (LPVOID)hCompletionPort, 0, NULL);

	hServSock = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	// ������ ���鶧�� �� ��ø ����� �������� ����� �־�� �Ѵ�.
	servAddr.sin_family = AF_INET;
	servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servAddr.sin_port = htons(atoi("2738"));

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
		

		PacketMaker::instance().PutObject(reinterpret_cast<Object*>(&users[id]), users[id].getID());
		updatePlayerView(users[id].getID());


		Flags = 0;

		//4. ��ø�� �������Է�.
		WSARecv(users[id].getSession().hClntSock, // ������ �Է¼���.
			&(users[id].getSession().recv_overlap.wsabuf),  // ������ �Է� ����������.
			1,       // ������ �Է� ������ ��.
			(LPDWORD)&RecvBytes,
			(LPDWORD)&Flags,
			&(users[id].getSession().recv_overlap.overlapped), // OVERLAPPED ����ü ������.
			NULL
			);
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
		

		if (FALSE == result) {
			// ���� ó��
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
			EnterCriticalSection(&monster->cs);
			//printf("%d ����", key);
			//���� �����̱�
			monster->heartBeat();
			//���� �� ������Ʈ
			updateMonsterView(key);
			LeaveCriticalSection(&monster->cs);
			//printf("%d ��\n", key);
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