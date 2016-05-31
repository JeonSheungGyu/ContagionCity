#include "stdafx.h"
#include "Server.h"
#include "PacketDispatcher.h"
#include "PacketSender.h"
#include "User.h"
#include "Monster.h"

using namespace std::chrono;

WSADATA Server::wsa;
SOCKET  Server::sock;
int	Server::clientID=-1;
thread*  Server::receiveThread;
thread*  Server::updateThread;
FuncProcess Server::Dispatcher[PACKET_TYPE];
atomic<int> Server::is_set = 0;
char	Server::SERVERIP[20];
char	Server::UserID[20];
atomic<int> Server::login_success = -1;

extern std::vector<User> users;
extern std::vector<Monster> monsters;


void Server::DispatcherInit()
{
	Dispatcher[SC_PUT_OBJECT].Func = PacketDispatcher::PutObject;
	Dispatcher[SC_REMOVE_OBJECT].Func = PacketDispatcher::RemoveObject;
	Dispatcher[SC_LOGIN].Func = PacketDispatcher::Login;
	Dispatcher[SC_MOVE_OBJECT].Func = PacketDispatcher::MoveObject;
	Dispatcher[SC_COMBAT_OBJECT].Func = PacketDispatcher::ObjectCombat;
	Dispatcher[SC_MONSTER_ATTACK].Func = PacketDispatcher::MonsterAttack;
	Dispatcher[SC_MONSTER_CHASE].Func = PacketDispatcher::MonsterChase;
	Dispatcher[SC_MONSTER_DIE].Func = PacketDispatcher::MonsterDie;
	Dispatcher[LC_PERMISION_LOGIN].Func = PacketDispatcher::PermisionLogin;
}


BOOL Server::LoginServerConnection(const string& pId, const string& pPassWord, const string& pIp)
{
	int retval = 0;

	static int count = 0;

	//IP, ID 저장
	strncpy_s(SERVERIP, pIp.c_str(), strlen(pIp.c_str()));
	strncpy_s(UserID, pId.c_str(), strlen(pId.c_str()));

	if (count++ == 0)
	{
		WSAStartup(MAKEWORD(2, 2), &wsa);
		sock = socket(AF_INET, SOCK_STREAM, 0);
		SOCKADDR_IN login_serveraddr;
		ZeroMemory(&login_serveraddr, sizeof(login_serveraddr));
		login_serveraddr.sin_family = AF_INET;
		login_serveraddr.sin_addr.s_addr = inet_addr(SERVERIP);
		login_serveraddr.sin_port = htons(LOGIN_PORT);
		//receiveThread = new thread{ ReceiveThread };
		retval = connect(sock, (SOCKADDR*)&login_serveraddr, sizeof(login_serveraddr));
	}

	cl_packet_request_login packet;
	strncpy_s(packet.id, pId.c_str(), strlen(pId.c_str()));
	strncpy_s(packet.password, pPassWord.c_str(), strlen(pPassWord.c_str()));
	packet.type = CL_REQUEST_LOGIN;
	packet.size = sizeof(packet);
	send(sock, reinterpret_cast<char*>(&packet), packet.size, 0);

	while (1)
	{
		if (login_success == 1)
			return true;
		else if (login_success == 0)
			return false;
	}
	cout << "LoginServer Connection End" << endl;
}


void Server::GameServerConnection()
{
	cout << "Server::GameServerConnection" << endl;
	int retval{}, sBufsize{}, sBuflen{};
	BOOL optval = TRUE;
	cout.clear();
	//cout << "\n게임서버 IP입력: "; cin >> SERVERIP;

	WSAStartup(MAKEWORD(2, 2), &wsa);

	sock = socket(AF_INET, SOCK_STREAM, 0);

	SOCKADDR_IN serveraddr;
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = inet_addr(SERVERIP);
	serveraddr.sin_port = htons(GAME_PORT);
	retval = connect(sock, (SOCKADDR*)&serveraddr, sizeof(serveraddr));
	setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (char*)&optval, sizeof(optval));
	cout << "Server::GameServerConnection End" << endl;
}


void Server::ReceiveThread()
{
	int retval{}, currentSize{};
	char buf[BUFSIZE];
	char *cPtr, *EnquePtr;
	cPtr = buf;
	EnquePtr = buf;

	while (1)
	{
		retval = recv(sock, EnquePtr, BUFSIZE, 0);

		//cout << "retval: " << retval << endl;
		if (retval == 0) break;
		if (retval == -1) continue;	// 소켓 닫고 열 때 필수적으로 처리해야됨
		currentSize += retval;
		EnquePtr += retval;
		if (currentSize < *buf) continue;	// 사이즈보다 적게 받음
		while (currentSize != 0) {	// 3월 17일 바꿈
			if (*cPtr > currentSize) break;
			Dispatcher[*(cPtr + 1)].Func(cPtr);
			currentSize -= *cPtr;

			cPtr += *cPtr;
		}
		memcpy(buf, cPtr, currentSize);
		cPtr = buf;
		EnquePtr = buf + currentSize;
	}
	cout << "receive thread exit()" << endl;
}


void Server::UpdateThread()
{
	auto& owner = users[getClientID()];
	auto oldPos = owner.getPos();
	auto start_time = std::chrono::high_resolution_clock::now();
	auto db_start_time = std::chrono::high_resolution_clock::now();
	std::chrono::duration<long long, std::milli> count;
	std::chrono::duration<long long, std::milli> db_count;
	while (1) {
		if (count.count() > UPDATE_TIME)
		{
			if (oldPos.x != owner.getPos().x || oldPos.y != owner.getPos().y) {

				oldPos = owner.getPos();
				PacketSender::instance().PlayerMove();
				start_time = std::chrono::high_resolution_clock::now();
			}
		}
		
		if (db_count.count() > DB_UPDATE_TIME)
		{
			PacketSender::instance().DBUpdate();
			db_start_time = std::chrono::high_resolution_clock::now();
			
		}

		count = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start_time);
		db_count = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - db_start_time);

		Sleep(UPDATE_TIME);
	}
}

