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
	// ���� ����� ��Ŷ��������� �̻��̸� �޴´�

	if (remain < MAX_CS_PACKET_SIZE && LastMark == nullptr)	// �ִ� ��Ŷũ��� �ٲٱ�
	{
		// size <= 4 && ������ �� á�� �� ������ ����
		resetEnPointer();
		remain = getRemain();	// remain �ٽ� ����
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
	//owner���� recv
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


// �ϼ��� ��Ŷ�� ������ ���� �� Deque, �߷����� tmpBuffer�� ������ ����
// ****** �Ŀ� �������� �����ϰ� ��Ŷ������� �������� ������ �Լ� ��������
// �����ؾߵ�**************************************************************
BufPointer RingBuffer::deQueue()
{
	if (DequePoint == LastMark)
	{
		LastMark = nullptr;
		DequePoint = buffer;
	}
	char size = *DequePoint;	// ��Ŷ size
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



