#pragma once
#include "Protocol.h"

using namespace std;

typedef char* BufPointer;

class Account;

class RingBuffer
{
private:	// pointer
	BufPointer EnquePoint;	// 데이터 삽입 지점
	BufPointer DequePoint;	// 데이터 추출 지점

	BufPointer EndMark;		// 버퍼의 끝 지점
	BufPointer LastMark;	// 어디까지 En 했는지 마킹

	char	   buffer[MAX_BUF_SIZE + 1];	// 링버퍼
	char	   tmpBuffer[MAX_PACKET_SIZE];// 잘린데이터 보관할 임시버퍼

	int		   tmpHeader;	// 잘린헤더 저장할 4바이트 버퍼

private:	// size
	int		   remain;	// 버퍼의 잔여 사이즈
	int		   usable;

private:	// flag
	bool	   bufFull;
	bool	   bufEmpty;

	WSABUF	   wsabuf; //??
	Account* const owner;	// 플레이어마다 링버퍼 1개. 버퍼소유주

private:
	// EnquePoint가 마지막에 도달했을 때 마커 생성 후 처음으로 돌아옴
	void resetEnPointer()
	{
		LastMark = EnquePoint;
		EnquePoint = buffer;
	}

public:
	RingBuffer(Account* owner);
	~RingBuffer();

	// 클라이언트에게 온 패킷 recv
	bool recvFromClient();

	// 링버퍼에 들어있는 데이터를 앞부터 차례대로 꺼냄
	// 리턴 : 이제 꺼내쓸 데이터의 위치
	BufPointer deQueue();

	// 데이터 삽입 시 EnPoint 전진
	void moveEnPoint(int transferred);

	// 데이터 삽입시 남은 공간 리턴
	// 리턴 : 남은 공간 사이즈
	int getRemain()
	{
		return (LastMark == nullptr) ? (EndMark - EnquePoint) : (DequePoint - EnquePoint);
	}

	// 데이터 추출시 남은 데이터 리턴
	// 리턴 : 사용가능한 공간 사이즈
	int getUsable()
	{
		return (LastMark == nullptr) ? (EnquePoint - DequePoint) :
			((LastMark - DequePoint) + (EnquePoint - buffer));
	}
};