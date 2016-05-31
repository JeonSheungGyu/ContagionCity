#include "Login.h"
#include "PacketDispatcher.h"
#include <iostream>
#include <cassert>
#include <sql.h>
#include <sqlext.h>

using namespace std;

vector<Account> Login::AccountInfo(MAX_USER);
PacketProcess Login::packetDispatcher[NUM_OF_PACKET_TYPE];
HANDLE Login::hIOCP;

// DB관련 queue 및 CRITICAL_SECTION
queue<DB_QUERY> Login::DB_Queue;
CRITICAL_SECTION Login::dbCS;


enum {
	DB_EVENT = 10000
};

Login::Login()
{
	InitializeCriticalSection(&dbCS);

	WSAStartup(MAKEWORD(2, 2), &wsadata);

	hIOCP = CreateNewCompletionPort(1);

	m_workerThread = thread{ workerThread };
	m_acceptThread = thread{ acceptThread };
	m_DBThread = thread{ DB_thread };

	// DB_connection();

	InitDispatcherFunc();
}


bool Login::beforeRecv(int key, DWORD cbTransfered)
{
	assert(key >= 0 && cbTransfered > 0);
	char* recvPacket = nullptr;
	
	try{
		AccountInfo.at(key).moveEnquePtr(cbTransfered);

		//recvPacket+1 은 type

		//패킷을 RingBuffer를 통해 받아서
		//PacketDispatcher로 받는다.
		while (recvPacket = AccountInfo.at(key).getPacketFromQueue())
			packetDispatcher[*(recvPacket + 1)].Func(recvPacket, key);

		
		if (!AccountInfo.at(key).recvFromClient()) return false;
	} catch (exception& e){
		cout << "Login::beforeRecv " << endl;
	}

	return true;
}


void Login::workerThread()
{
	DWORD cbTransfered;
	DWORD key;
	OVERLAPPED_EX *over_ex;
	while (true)
	{
		auto ret = GetQueuedCompletionStatus(hIOCP, &cbTransfered, &key, reinterpret_cast<LPOVERLAPPED*>(&over_ex), INFINITE);

		if (ret == SOCKET_ERROR) err_display("getQueuedCompletionStatus", 0);

		if (cbTransfered == 0)
		{
			try{
				closesocket(AccountInfo.at(key).getSock());
				AccountInfo.at(key).is_using = false;
			} catch (exception& e){
				cout << "Login::workerThread " << endl;
			}
			continue;
		}

		if (over_ex->command == DB_EVENT){
			lc_packet_permit_login packet;
			if (over_ex->Login_Permision){
				//성공
				packet.permit_check = true;
				packet.type = LC_PERMISION_LOGIN;
				packet.size = sizeof(packet);
			} else{
				//실패
				packet.permit_check = false;
				packet.type = LC_PERMISION_LOGIN;
				packet.size = sizeof(packet);
			}
			//전송
			sendPacket(key, &packet);
		}
		else if (over_ex->is_send == false){
			beforeRecv(key, cbTransfered);
		}
		else if(over_ex->is_send == true){
			delete over_ex;
		}
	}
}


void Login::acceptThread()
{
	sockaddr_in listenAddr;
	sockaddr_in clientAddr;

	ZeroMemory(&clientAddr, sizeof(clientAddr));

	SOCKET listenSock = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);

	ZeroMemory(&listenAddr, sizeof(listenAddr));
	listenAddr.sin_family = AF_INET;
	listenAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	listenAddr.sin_port = htons(LOGIN_PORT);

	int ret = ::bind(listenSock, reinterpret_cast<SOCKADDR*>(&listenAddr), sizeof(listenAddr));

	if (ret == SOCKET_ERROR)
	{
		err_display("Bind", WSAGetLastError());
		exit(1);
	}
	ret = ::listen(listenSock, 10);
	if (ret == SOCKET_ERROR)
	{
		err_display("Listen", WSAGetLastError());
		exit(1);
	}

	while (true)
	{
		int addrSize = sizeof(SOCKADDR_IN);
		SOCKET clientSock = accept(listenSock, reinterpret_cast<SOCKADDR*>(&clientAddr), &addrSize);

		if (clientSock == INVALID_SOCKET)
		{
			err_display("Accept", WSAGetLastError());
			exit(1);
		}

		DWORD id = getnewClientID();

		try{
			AccountInfo.at(id).setSock(clientSock);
			AccountInfo.at(id).setID(id);
			cout << "Account ID: " << id << " 로그인서버 접속" << endl;

			AssociateDeviceWithCompetionPort(hIOCP, reinterpret_cast<HANDLE>(clientSock), id);

			AccountInfo.at(id).is_using = true;

			if (!AccountInfo.at(id).recvFromClient()) return;
		} catch (exception& e){
			cout << "Login::acceptThread " << endl;
		}
	}
}


// DB_Queue에 들어온 요청이 있으면 실행 없으면 Sleep
void Login::DB_thread()
{
	// workerThread로 보낼 OVERLAPPED_EX
	OVERLAPPED_EX* overEx = new OVERLAPPED_EX;

	SQLHENV henv;
	SQLHDBC hdbc;
	SQLHSTMT hstmt = 0;
	SQLRETURN retcode;

	SQLCHAR * OutConnStr = (SQLCHAR*)malloc(255);
	SQLSMALLINT * OutConnStrLen = (SQLSMALLINT*)malloc(255);

	// Allocate enviroment handle
	retcode = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv);

	// Set the ODBC version enviroment attribute
	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO){
		retcode = SQLSetEnvAttr(henv, SQL_ATTR_ODBC_VERSION, (void*)SQL_OV_ODBC3, 0);

		// Allocate connection handle
		if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO){
			retcode = SQLAllocHandle(SQL_HANDLE_DBC, henv, &hdbc);

			// Set login timeout to 5 seconds
			if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO){
				SQLSetConnectAttr(hdbc, SQL_LOGIN_TIMEOUT, (SQLPOINTER)5, 0);

				// Connect to data source
				retcode = SQLConnect(hdbc, (SQLWCHAR*)L"ContagionCityMS", SQL_NTS, (SQLWCHAR*)L"sa",
					SQL_NTS, (SQLWCHAR*)L"1q2w3e4r@@", SQL_NTS);

				// Allocate statement handle
				if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO){
					cout << "DB connection success" << endl;

					// --------------- DB_thread loop start ------------------
					while (true){
						EnterCriticalSection(&dbCS);
						if (DB_Queue.empty())
						{
							LeaveCriticalSection(&dbCS);
							Sleep(1);
							continue;
						}
						DB_QUERY q = DB_Queue.front();
						DB_Queue.pop();
						LeaveCriticalSection(&dbCS);

						retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);

						if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO){
							// q.type에 넘어온 DB 요청에 따라 처리
							if (q.type == ID_PW_CHECK){
								WORD accountID = q.accountid;

								char checkName[20];
								char checkPw[20];

								SQLINTEGER cbCheckName = SQL_NTS;
								SQLINTEGER cbCheckPw = SQL_NTS;

								int PermisionOfLogin = 0;

								SQLINTEGER cbCheckLogin = SQL_NTS;

								// 저장프로시저에 사용할 파라미터 바인드
								auto r = SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_CHAR, sizeof(checkName), 
									0, checkName, sizeof(checkName), &cbCheckName);

								r = SQLBindParameter(hstmt, 2, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_CHAR, sizeof(checkPw), 
									0, checkPw, sizeof(checkPw), &cbCheckPw);

								sprintf_s(checkName, sizeof(checkName), "%s", q.id);
								sprintf_s(checkPw, sizeof(checkPw), "%s", q.pw);

								// 저장프로시저 호출
								retcode = SQLExecDirect(hstmt, (SQLWCHAR*)L"exec CheckUser ?, ?", SQL_NTS);

								if (retcode == SQL_SUCCESS || SQL_SUCCESS_WITH_INFO){
									retcode = SQLBindCol(hstmt, 1, SQL_INTEGER, &PermisionOfLogin, sizeof(PermisionOfLogin), &cbCheckLogin);

									
									// 데이터 가져오기
									retcode = SQLFetch(hstmt);
									ZeroMemory(overEx, sizeof(overEx));
									overEx->command = DB_EVENT;

									//로그인 가능여부
									if (PermisionOfLogin) overEx->Login_Permision = true;
									else overEx->Login_Permision = false;
									cout <<"로그인여부 : " << overEx->Login_Permision << endl;
									PostQueuedCompletionStatus(Login::hIOCP, 1, accountID, (OVERLAPPED*)overEx);
								}
							}
						}
						else
							printf("connection fail");

						if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO){
							SQLCancel(hstmt);
							SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
						}
					}
					// ------------------- thread loop end -------------------
				}
				SQLDisconnect(hdbc);
			}
			SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
		}
	}
	SQLFreeHandle(SQL_HANDLE_ENV, henv);
}


int Login::getnewClientID()
{
	for (size_t i = 0; i < AccountInfo.size(); ++i)
		if (AccountInfo[i].is_using == false) return i;
}


void Login::sendPacket(int id, void* packet)
{
	assert(id >= 0 && packet);
	int packetSize = reinterpret_cast<unsigned char*>(packet)[0];

	OVERLAPPED_EX *send_over = new OVERLAPPED_EX;
	ZeroMemory(send_over, sizeof(OVERLAPPED_EX));
	send_over->wsabuf.buf = send_over->packetBuf;
	send_over->wsabuf.len = (unsigned long)((char*)packet)[0];

	memcpy(send_over->packetBuf, packet, send_over->wsabuf.len);
	send_over->is_send = true;
	unsigned long sendsize = send_over->wsabuf.len;
	WSASend(AccountInfo[id].getSock(), &send_over->wsabuf, 1, &sendsize, NULL, &send_over->overlapped, NULL);
}


HANDLE Login::CreateNewCompletionPort(DWORD dwNumberOfConcurrentTreads)
{
	assert(dwNumberOfConcurrentTreads >= 0);
	return (CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, dwNumberOfConcurrentTreads));
}


BOOL Login::AssociateDeviceWithCompetionPort(HANDLE hCompletionPort, HANDLE hDevice, DWORD dwCompletionKey)
{
	HANDLE h = CreateIoCompletionPort(hDevice, hCompletionPort, dwCompletionKey, 0);
	return (h == hCompletionPort);
}


Login::~Login()
{
	m_acceptThread.join();
	m_workerThread.join();
	m_DBThread.join();
	DeleteCriticalSection(&dbCS);
}


void Login::InitDispatcherFunc()
{
	packetDispatcher[CL_REQUEST_LOGIN].Func = PacketDispatcher_Client::RequestLogin;
	packetDispatcher[DL_PERMISION_LOGIN].Func = PacketDispatcher_DB_Server::PermisionLogin;

}


void Login::AccountUsableCheck(const WORD accountid, char* id, char* password)
{
	DB_QUERY q;
	q.accountid = accountid;
	q.type = ID_PW_CHECK;
	strcpy_s(q.id, 20, id);
	strcpy_s(q.pw, 20, password);

	
}