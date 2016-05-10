#pragma once
#include "RingBuffer.h"
#include "OVERLAPPED_EX.h"

class Account
{
private:
	SOCKET sock;
	RingBuffer recvQue;
	WORD accountid;
	char id[ID_LEN];
	char password[PASSWORD_LEN];

public:
	Account();
	~Account();

	OVERLAPPED_EX overlapped;

	// 링버퍼 함수를 위한 인터페이스 함수
	bool recvFromClient() { return recvQue.recvFromClient(); }
	void moveEnquePtr(const int transferred) { recvQue.moveEnPoint(transferred);}
	char* getPacketFromQueue() { return recvQue.deQueue(); }


	void setID(const WORD id) { accountid = id; }
	WORD getID()const { return accountid; }


	const SOCKET getSock()const { return sock; }
	void setSock(const SOCKET soc) { sock = soc; }

	void setPassword(char* password) { memcpy(this->password, password, sizeof(password)); }
	char* getPassword(){ return password; }

	bool is_using;
};