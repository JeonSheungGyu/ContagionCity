#pragma once
#include "Protocol.h"

using namespace std;

typedef char* BufPointer;

class Account;

class RingBuffer
{
private:	// pointer
	BufPointer EnquePoint;	// ������ ���� ����
	BufPointer DequePoint;	// ������ ���� ����

	BufPointer EndMark;		// ������ �� ����
	BufPointer LastMark;	// ������ En �ߴ��� ��ŷ

	char	   buffer[MAX_BUF_SIZE + 1];	// ������
	char	   tmpBuffer[MAX_PACKET_SIZE];// �߸������� ������ �ӽù���

	int		   tmpHeader;	// �߸���� ������ 4����Ʈ ����

private:	// size
	int		   remain;	// ������ �ܿ� ������
	int		   usable;

private:	// flag
	bool	   bufFull;
	bool	   bufEmpty;

	WSABUF	   wsabuf; //??
	Account* const owner;	// �÷��̾�� ������ 1��. ���ۼ�����

private:
	// EnquePoint�� �������� �������� �� ��Ŀ ���� �� ó������ ���ƿ�
	void resetEnPointer()
	{
		LastMark = EnquePoint;
		EnquePoint = buffer;
	}

public:
	RingBuffer(Account* owner);
	~RingBuffer();

	// Ŭ���̾�Ʈ���� �� ��Ŷ recv
	bool recvFromClient();

	// �����ۿ� ����ִ� �����͸� �պ��� ���ʴ�� ����
	// ���� : ���� ������ �������� ��ġ
	BufPointer deQueue();

	// ������ ���� �� EnPoint ����
	void moveEnPoint(int transferred);

	// ������ ���Խ� ���� ���� ����
	// ���� : ���� ���� ������
	int getRemain()
	{
		return (LastMark == nullptr) ? (EndMark - EnquePoint) : (DequePoint - EnquePoint);
	}

	// ������ ����� ���� ������ ����
	// ���� : ��밡���� ���� ������
	int getUsable()
	{
		return (LastMark == nullptr) ? (EnquePoint - DequePoint) :
			((LastMark - DequePoint) + (EnquePoint - buffer));
	}
};