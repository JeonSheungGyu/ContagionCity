#include "RingBuffer.h"
#include "Account.h"
#include <cassert>
#include <WinSock2.h>


RingBuffer::RingBuffer(Account* owner) : owner(owner)
{
	buffer[MAX_BUF_SIZE] = 0;

	EndMark = buffer + MAX_BUF_SIZE;
	EnquePoint = buffer;
	DequePoint = buffer;

	LastMark = nullptr;

	remain = MAX_BUF_SIZE;
	usable = 0;

	bufFull = false;
	bufEmpty = false;
}


RingBuffer::~RingBuffer(){}


void RingBuffer::moveEnPoint(int transferred)
{
	assert(transferred > 0);
	remain = getRemain();
	if (transferred <= remain)
	{
		EnquePoint += transferred;
		if (EnquePoint == EndMark) resetEnPointer();
		if (EnquePoint == DequePoint) bufFull = true;

		bufEmpty = false;
	}
}


bool RingBuffer::recvFromClient()
{
	int retval = 0;
	remain = getRemain();
	// 남은 사이즈가 패킷헤더사이즈 이상이면 받는다

	if (remain < MAX_CS_PACKET_SIZE && LastMark == nullptr)	// 최대 패킷크기로 바꾸기
	{
		// size <= 4 && 끝까지 안 찼을 때 포인터 리셋
		resetEnPointer();
		remain = getRemain();	// remain 다시 얻음
	}

	if (remain < MAX_CS_PACKET_SIZE)
	{
		err_display("RingBuffer : remain < MAX_CS_PACKET_SIZE\n", 0);
		return false;
	}

	DWORD flags = 0;

	wsabuf.buf = EnquePoint;
	wsabuf.len = remain;

	::ZeroMemory(&owner->overlapped.overlapped, sizeof(OVERLAPPED));
	owner->overlapped.is_send = false;
	//owner에게 recv
	retval = WSARecv(owner->getSock(), &(wsabuf), 1,
		0, &flags, &owner->overlapped.overlapped, NULL);

	if (retval == SOCKET_ERROR)
	{
		retval = WSAGetLastError();
		if (retval != WSA_IO_PENDING)
		{
			err_display("RingBuffer: remain<sizeof(PACKET_SIZE)\n", retval);
			return false;
		}
	}
	return true;
}


// 완성된 패킷을 받을수 있을 땐 Deque, 잘렸으면 tmpBuffer에 복사후 리턴
// ****** 후에 프로토콜 정의하고 패킷헤더보다 남은공간 작으면 함수 실행으로
// 변경해야됨**************************************************************
BufPointer RingBuffer::deQueue()
{
	if (DequePoint == LastMark)
	{
		LastMark = nullptr;
		DequePoint = buffer;
	}
	char size = *DequePoint;	// 패킷 size
	usable = getUsable();
	BufPointer retPointer = DequePoint;
	if (size <= 0) return nullptr;

	if (usable >= size)	DequePoint += size;
	else return nullptr;

	if (DequePoint == EnquePoint)
		this->bufEmpty = true;

	bufFull = false;

	assert(retPointer != nullptr);
	return retPointer;
}



