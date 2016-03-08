#include "stdafx.h"
#include "CriticalSection.h"
#include "MultiThreadSync.h"
#include "CircularQueue.h"
#include "NetworkSession.h"

DWORD WINAPI ReliableUdpThreadCallback(LPVOID parameter)
{
	CNetworkSession *Owner = (CNetworkSession*)parameter;
	Owner->ReliableUdpThreadCallback();

	return 0;
}

CNetworkSession::CNetworkSession(VOID)
{
	//Overlapped 구조체 
	memset(&mAcceptOverlapped, 0, sizeof(mAcceptOverlapped));	//Accpet
	memset(&mReadOverlapped, 0, sizeof(mReadOverlapped));		//Read
	memset(&mWriteOverlapped, 0, sizeof(mWriteOverlapped));		//Write

	//buffer
	memset(mReadBuffer, 0, sizeof(mReadBuffer));
	//UDP 주소
	memset(&mUdpRemoteInfo, 0, sizeof(mUdpRemoteInfo));

	//UDP Event 
	mSocket = NULL;
	mReliableUdpThreadHandle = NULL;
	mReliableUdpThreadStartupEvent = NULL;
	mReliableUdpThreadDestroyEvent = NULL;
	mReliableUdpThreadWakeUpEvent = NULL;
	mReliableUdpWriteCompleteEvent = NULL;

	mIsReliableUdpSending = FALSE;

	//Overlapped IO_TYPE
	mAcceptOverlapped.IoType = IO_ACCEPT;
	mReadOverlapped.IoType = IO_READ;
	mWriteOverlapped.IoType = IO_WRITE;

	//Object = this
	mAcceptOverlapped.Object = this;
	mReadOverlapped.Object = this;
	mWriteOverlapped.Object = this;
}

CNetworkSession::~CNetworkSession(VOID)
{

}

//UDP CallBack
VOID CNetworkSession::ReliableUdpThreadCallback(VOID)
{
	DWORD				EventID = 0;
	//Event Destroy, WakeUp
	HANDLE				ThreadEvents[2] = { mReliableUdpThreadDestroyEvent, mReliableUdpThreadWakeUpEvent };

	CHAR				RemoteAddress[32] = { 0, };
	USHORT				RemotePort = 0;
	BYTE				Data[MAX_BUFFER_LENGTH] = { 0, };
	DWORD				DataLength = 0;
	VOID				*Object = NULL;

	while (TRUE)
	{
		//시작을 알리는 Event
		SetEvent(mReliableUdpThreadStartupEvent);


		//Destroy, WakeUp Event
		EventID = WaitForMultipleObjects(2, ThreadEvents, FALSE, INFINITE);
		
		switch (EventID)
		{
			//Destroy Event
		case WAIT_OBJECT_0:
			return;
			//WakeUp
		case WAIT_OBJECT_0 + 1:
		NEXT_DATA :
			// UDP 전용 큐에서 데이터를 가져와 전송한다.
			if (mReliableWriteQueue.Pop(&Object, Data, DataLength, RemoteAddress, RemotePort))
			{
				// 데이터가 있을 경우
				// 실제 Write를 해 주고 WaitForSingleObject를 해 준다.
				// 받았을때 그 SetEvent를 해 주면 풀린다.
			RETRY:
				//UDP 데이터 전송
				if (!WriteTo2(RemoteAddress, RemotePort, Data, DataLength))
					return;

				DWORD Result = WaitForSingleObject(mReliableUdpWriteCompleteEvent, 10);

				// 받았을 경우 다음 데이터 처리
				if (Result == WAIT_OBJECT_0)
					goto NEXT_DATA;
				else
					goto RETRY;
			}
			else
				mIsReliableUdpSending = FALSE;

				  break;
		}
	}
}

//데이터 초기화
BOOL CNetworkSession::Begin(VOID)
{
	CThreadSync Sync;

	if (mSocket)
		return FALSE;

	//버퍼 초기화
	memset(mReadBuffer, 0, sizeof(mReadBuffer));
	memset(&mUdpRemoteInfo, 0, sizeof(mUdpRemoteInfo));

	//Event
	mSocket = NULL;
	mReliableUdpThreadHandle = NULL;
	mReliableUdpThreadStartupEvent = NULL;
	mReliableUdpThreadDestroyEvent = NULL;
	mReliableUdpThreadWakeUpEvent = NULL;
	mReliableUdpWriteCompleteEvent = NULL;

	mIsReliableUdpSending = FALSE;

	return TRUE;
}

//리소스 해제
BOOL CNetworkSession::End(VOID)
{
	CThreadSync Sync;

	if (!mSocket)
		return FALSE;

	closesocket(mSocket);

	mSocket = NULL;

	//UDP 스레드가 존재하면
	if (mReliableUdpThreadHandle)
	{
		//스레드 종료
		SetEvent(mReliableUdpThreadDestroyEvent);
		//스레드 종료 대기
		WaitForSingleObject(mReliableUdpThreadHandle, INFINITE);
		//스레드 해제
		CloseHandle(mReliableUdpThreadHandle);
	}

	//이벤트 해제
	if (mReliableUdpThreadDestroyEvent)
		CloseHandle(mReliableUdpThreadDestroyEvent);

	if (mReliableUdpThreadStartupEvent)
		CloseHandle(mReliableUdpThreadStartupEvent);

	if (mReliableUdpThreadWakeUpEvent)
		CloseHandle(mReliableUdpThreadWakeUpEvent);

	if (mReliableUdpWriteCompleteEvent)
		CloseHandle(mReliableUdpWriteCompleteEvent);

	//Queue 해제
	mReliableWriteQueue.End();

	return TRUE;
}

//TCP Listen
BOOL CNetworkSession::Listen(USHORT port, INT backLog)
{
	CThreadSync Sync;

	if (port <= 0 || backLog <= 0)
		return FALSE;

	if (!mSocket)
		return FALSE;

	SOCKADDR_IN ListenSocketInfo;

	ListenSocketInfo.sin_family = AF_INET;
	ListenSocketInfo.sin_port = htons(port);
	ListenSocketInfo.sin_addr.S_un.S_addr = htonl(INADDR_ANY);

	//bind
	if (bind(mSocket, (struct sockaddr*) &ListenSocketInfo, sizeof(SOCKADDR_IN)) == SOCKET_ERROR)
	{
		End();

		return FALSE;
	}
	//listen, backLog 는 대기큐의 개수이다.
	if (listen(mSocket, backLog) == SOCKET_ERROR)
	{
		End();

		return FALSE;
	}

	//LINGER 설정
	LINGER Linger;
	//데이터가 남아있어도 종료한다.
	Linger.l_onoff = 1;
	Linger.l_linger = 0;

	//소켓 옵션설정
	if (setsockopt(mSocket, SOL_SOCKET, SO_LINGER, (char*)&Linger, sizeof(LINGER)) == SOCKET_ERROR)
	{
		End();

		return FALSE;
	}

	return TRUE;
}
//TCP Accept
BOOL CNetworkSession::Accept(SOCKET listenSocket)
{
	CThreadSync Sync;

	if (!listenSocket)
		return FALSE;

	if (mSocket)
		return FALSE;

	//Socket 생성
	mSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);

	if (mSocket == INVALID_SOCKET)
	{
		End();

		return FALSE;
	}

	//BOOL NoDelay = TRUE;
	//setsockopt(mSocket, IPPROTO_TCP, TCP_NODELAY, (const char FAR *)&NoDelay, sizeof(NoDelay));

	//미리 만들어진 소켓을 가지고 접속을 받는 AcceptEx 이다.
	//많은 수의 접속이 몰려올 때 병목현상을 막을 수 있다.
	//AcceptEx당 하나의 접속을 받을 수 있다.
	//Accept가 되면 IOCP로 IO_ACCEP가 전달된다.
	if (!AcceptEx(listenSocket,
		mSocket,
		mReadBuffer,
		0,
		sizeof(sockaddr_in) + 16,
		sizeof(sockaddr_in) + 16,
		NULL,
		&mAcceptOverlapped.Overlapped))
	{
		if (WSAGetLastError() != ERROR_IO_PENDING && WSAGetLastError() != WSAEWOULDBLOCK)
		{
			End();

			return FALSE;
		}
	}

	return TRUE;
}

//IOCP를 사용하여 데이터를 받는다.
BOOL CNetworkSession::InitializeReadForIocp(VOID)
{
	CThreadSync Sync;

	if (!mSocket)
		return FALSE;

	WSABUF	WsaBuf;
	DWORD	ReadBytes = 0;
	DWORD	ReadFlag = 0;

	WsaBuf.buf = (CHAR*)mReadBuffer;
	WsaBuf.len = MAX_BUFFER_LENGTH;

	INT		ReturnValue = WSARecv(mSocket,
		&WsaBuf,
		1,
		&ReadBytes,
		&ReadFlag,
		&mReadOverlapped.Overlapped,
		NULL);

	if (ReturnValue == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING && WSAGetLastError() != WSAEWOULDBLOCK)
	{
		End();

		return FALSE;
	}

	return TRUE;
}

//IOCP를 통해 데이터 도착알림을 받게 되면 버퍼에서 data로 복사한다.
BOOL CNetworkSession::ReadForIocp(BYTE *data, DWORD &dataLength)
{
	CThreadSync Sync;

	if (!mSocket)
		return FALSE;

	if (!data || dataLength <= 0)
		return FALSE;

	memcpy(data, mReadBuffer, dataLength);

	return TRUE;
}

//EventSelect Read
BOOL CNetworkSession::ReadForEventSelect(BYTE *data, DWORD &dataLength)
{
	CThreadSync Sync;

	if (!mSocket)
		return FALSE;

	if (!data)
		return FALSE;

	if (!mSocket)
		return FALSE;

	WSABUF	WsaBuf;
	DWORD	ReadBytes = 0;
	DWORD	ReadFlag = 0;

	WsaBuf.buf = (CHAR*)mReadBuffer;
	WsaBuf.len = MAX_BUFFER_LENGTH;

	INT		ReturnValue = WSARecv(mSocket,
		&WsaBuf,
		1,
		&ReadBytes,
		&ReadFlag,
		&mReadOverlapped.Overlapped,
		NULL);

	if (ReturnValue == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING && WSAGetLastError() != WSAEWOULDBLOCK)
	{
		End();

		return FALSE;
	}
	//알림이 온상태로 데이터를 받기 때문에 바로 복사한다.
	memcpy(data, mReadBuffer, ReadBytes);
	dataLength = ReadBytes;

	return TRUE;
}

//IOCP, EventSelect 의 Write는 동일하다. 데이터 복사가 없음.
BOOL CNetworkSession::Write(BYTE *data, DWORD dataLength)
{
	CThreadSync Sync;

	if (!mSocket)
		return FALSE;

	if (!data || dataLength <= 0)
		return FALSE;

	WSABUF	WsaBuf;
	DWORD	WriteBytes = 0;
	DWORD	WriteFlag = 0;

	WsaBuf.buf = (CHAR*)data;
	WsaBuf.len = dataLength;

	INT		ReturnValue = WSASend(mSocket,
		&WsaBuf,
		1,
		&WriteBytes,
		WriteFlag,
		&mWriteOverlapped.Overlapped,
		NULL);
	
	if (ReturnValue == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING && WSAGetLastError() != WSAEWOULDBLOCK)
	{
		End();

		return FALSE;
	}

	return TRUE;
}

//TCP 접속연결
BOOL CNetworkSession::Connect(LPSTR address, USHORT port)
{
	CThreadSync Sync;

	if (!address || port <= 0)
		return FALSE;

	if (!mSocket)
		return FALSE;

	SOCKADDR_IN RemoteAddressInfo;

	RemoteAddressInfo.sin_family = AF_INET;
	RemoteAddressInfo.sin_port = htons(port);
	RemoteAddressInfo.sin_addr.S_un.S_addr = inet_addr(address);

	if (WSAConnect(mSocket, (LPSOCKADDR)&RemoteAddressInfo, sizeof(SOCKADDR_IN), NULL, NULL, NULL, NULL) == SOCKET_ERROR)
	{
		if (WSAGetLastError() != WSAEWOULDBLOCK)
		{
			End();

			return FALSE;
		}
	}

	return TRUE;
}
//UDP 바인드
BOOL CNetworkSession::UdpBind(USHORT port)
{
	CThreadSync Sync;

	if (mSocket)
		return FALSE;

	SOCKADDR_IN RemoteAddressInfo;

	RemoteAddressInfo.sin_family = AF_INET;
	RemoteAddressInfo.sin_port = htons(port);
	RemoteAddressInfo.sin_addr.S_un.S_addr = htonl(INADDR_ANY);

	mSocket = WSASocket(AF_INET, SOCK_DGRAM, IPPROTO_UDP, NULL, 0, WSA_FLAG_OVERLAPPED);

	if (mSocket == INVALID_SOCKET)
		return FALSE;

	if (bind(mSocket, (struct sockaddr*) &RemoteAddressInfo, sizeof(SOCKADDR_IN)) == SOCKET_ERROR)
	{
		End();

		return FALSE;
	}
	
	//ReliableUDP 스레드 및 이벤트 생성
	// mReliableUdpThreadDestroyEvent
	mReliableUdpThreadDestroyEvent = CreateEvent(0, FALSE, FALSE, 0);
	if (mReliableUdpThreadDestroyEvent == NULL)
	{
		End();

		return FALSE;
	}
	//

	// mReliableUdpThreadStartupEvent
	mReliableUdpThreadStartupEvent = CreateEvent(0, FALSE, FALSE, 0);
	if (mReliableUdpThreadStartupEvent == NULL)
	{
		End();

		return FALSE;
	}
	//

	// mReliableUdpThreadWakeUpEvent
	mReliableUdpThreadWakeUpEvent = CreateEvent(0, FALSE, FALSE, 0);
	if (mReliableUdpThreadWakeUpEvent == NULL)
	{
		End();

		return FALSE;
	}
	//

	// mReliableUdpWriteCompleteEvent
	mReliableUdpWriteCompleteEvent = CreateEvent(0, FALSE, FALSE, 0);
	if (mReliableUdpWriteCompleteEvent == NULL)
	{
		End();

		return FALSE;
	}
	//

	//ReliableWriteQueue 초기화
	if (!mReliableWriteQueue.Begin())
	{
		End();

		return FALSE;
	}

	//스레드 가동
	DWORD ReliableUdpThreadID = 0;
	mReliableUdpThreadHandle = CreateThread(NULL, 0, ::ReliableUdpThreadCallback, this, 0, &ReliableUdpThreadID);

	WaitForSingleObject(mReliableUdpThreadStartupEvent, INFINITE);

	return TRUE;
}

//TCPBind
BOOL CNetworkSession::TcpBind(VOID)
{
	CThreadSync Sync;

	if (mSocket)
		return FALSE;

	mSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);

	if (mSocket == INVALID_SOCKET)
		return FALSE;

	//BOOL NoDelay = TRUE;
	//setsockopt(mSocket, IPPROTO_TCP, TCP_NODELAY, (const char FAR *)&NoDelay, sizeof(NoDelay));
	return TRUE;
}



BOOL CNetworkSession::GetLocalIP(WCHAR* pIP)
{
	CThreadSync Sync;

	if (!mSocket)
		return FALSE;

	//SOCKADDR_IN addr;
	//ZeroMemory(&addr, sizeof(addr));

	//int addrLength = sizeof(addr);
	//if(getsockname(mSocket, (sockaddr*)&addr, &addrLength ) != SOCKET_ERROR)
	//{
	//	if(MultiByteToWideChar(CP_ACP, 0, inet_ntoa(addr.sin_addr), 32, pIP, 32) > 0)
	//		return TRUE;
	//}

	CHAR	Name[256] = { 0, };

	gethostname(Name, sizeof(Name));

	PHOSTENT host = gethostbyname(Name);
	if (host)
	{
		if (MultiByteToWideChar(CP_ACP, 0, inet_ntoa(*(struct in_addr*)*host->h_addr_list), -1, pIP, 32) > 0)
			return TRUE;
	}

	return FALSE;
}

USHORT CNetworkSession::GetLocalPort(VOID)
{
	CThreadSync Sync;

	if (!mSocket)
		return 0;

	SOCKADDR_IN Addr;
	ZeroMemory(&Addr, sizeof(Addr));

	INT AddrLength = sizeof(Addr);
	if (getsockname(mSocket, (sockaddr*)&Addr, &AddrLength) != SOCKET_ERROR)
		return ntohs(Addr.sin_port);

	return 0;
}
//UDP IOCP Read
BOOL CNetworkSession::InitializeReadFromForIocp(VOID)
{
	CThreadSync Sync;

	if (!mSocket)
		return FALSE;

	WSABUF		WsaBuf;
	DWORD		ReadBytes = 0;
	DWORD		ReadFlag = 0;
	INT			RemoteAddressInfoSize = sizeof(mUdpRemoteInfo);

	WsaBuf.buf = (CHAR*)mReadBuffer;
	WsaBuf.len = MAX_BUFFER_LENGTH;

	INT		ReturnValue = WSARecvFrom(mSocket,
		&WsaBuf,
		1,
		&ReadBytes,
		&ReadFlag,
		(SOCKADDR*)&mUdpRemoteInfo,
		&RemoteAddressInfoSize,
		&mReadOverlapped.Overlapped,
		NULL);

	if (ReturnValue == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING && WSAGetLastError() != WSAEWOULDBLOCK)
	{
		End();

		return FALSE;
	}

	return TRUE;
}

//UDP 데이터 복사
BOOL CNetworkSession::ReadFromForIocp(LPSTR remoteAddress, USHORT &remotePort, BYTE *data, DWORD &dataLength)
{
	CThreadSync Sync;

	if (!mSocket)
		return FALSE;

	if (!data || dataLength <= 0)
		return FALSE;

	memcpy(data, mReadBuffer, dataLength);

	//memcpy(remoteAddress, inet_ntoa(mUdpRemoteInfo.sin_addr), 32);
	strcpy(remoteAddress, inet_ntoa(mUdpRemoteInfo.sin_addr));
	remotePort = ntohs(mUdpRemoteInfo.sin_port);

	USHORT Ack = 0;
	//ACK 체크
	memcpy(&Ack, mReadBuffer, sizeof(USHORT));

	if (Ack == 9999)
	{
		//전송이 완료되었다고 알린다.
		SetEvent(mReliableUdpWriteCompleteEvent);

		return FALSE;
	}
	else
	{
		//데이터를 수신하면 ACK를 보낸다.
		Ack = 9999;
		WriteTo2(remoteAddress, remotePort, (BYTE*)&Ack, sizeof(USHORT));
	}

	return TRUE;
}
//UDP EventSelect Read
BOOL CNetworkSession::ReadFromForEventSelect(LPSTR remoteAddress, USHORT &remotePort, BYTE *data, DWORD &dataLength)
{
	CThreadSync Sync;

	if (!mSocket)
		return FALSE;

	if (!data)
		return FALSE;

	if (!mSocket)
		return FALSE;

	WSABUF		WsaBuf;
	DWORD		ReadBytes = 0;
	DWORD		ReadFlag = 0;
	INT			RemoteAddressInfoSize = sizeof(mUdpRemoteInfo);

	WsaBuf.buf = (CHAR*)mReadBuffer;
	WsaBuf.len = MAX_BUFFER_LENGTH;

	INT		ReturnValue = WSARecvFrom(mSocket,
		&WsaBuf,
		1,
		&ReadBytes,
		&ReadFlag,
		(SOCKADDR*)&mUdpRemoteInfo,
		&RemoteAddressInfoSize,
		&mReadOverlapped.Overlapped,
		NULL);

	if (ReturnValue == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING && WSAGetLastError() != WSAEWOULDBLOCK)
	{
		End();

		return FALSE;
	}

	memcpy(data, mReadBuffer, ReadBytes);
	dataLength = ReadBytes;

	//memcpy(remoteAddress, inet_ntoa(mUdpRemoteInfo.sin_addr), 32);
	strcpy(remoteAddress, inet_ntoa(mUdpRemoteInfo.sin_addr));
	remotePort = ntohs(mUdpRemoteInfo.sin_port);


	//데이터를 수신하였으면 바로 ACK 전송
	USHORT Ack = 0;
	memcpy(&Ack, mReadBuffer, sizeof(USHORT));

	if (Ack == 9999)
	{
		SetEvent(mReliableUdpWriteCompleteEvent);

		return FALSE;
	}
	else
	{
		Ack = 9999;
		WriteTo2(remoteAddress, remotePort, (BYTE*)&Ack, sizeof(USHORT));
	}

	return TRUE;
}

//UDP 전용 큐에 데이터와 주소를 저장한다.
BOOL CNetworkSession::WriteTo(LPCSTR remoteAddress, USHORT remotePort, BYTE *data, DWORD dataLength)
{
	CThreadSync Sync;

	if (!mSocket)
		return FALSE;

	if (!remoteAddress || remotePort <= 0 || !data || dataLength <= 0)
		return FALSE;

	if (!mReliableWriteQueue.Push(this, data, dataLength, remoteAddress, remotePort))
		return FALSE;

	if (!mIsReliableUdpSending)
	{
		mIsReliableUdpSending = TRUE;
		SetEvent(mReliableUdpThreadWakeUpEvent);
	}

	return TRUE;
}

//UDP 데이터 전송
BOOL CNetworkSession::WriteTo2(LPSTR remoteAddress, USHORT remotePort, BYTE *data, DWORD dataLength)
{
	CThreadSync Sync;

	if (!mSocket)
		return FALSE;

	if (!remoteAddress || remotePort <= 0 || !data || dataLength <= 0)
		return FALSE;

	WSABUF		WsaBuf;
	DWORD		WriteBytes = 0;
	DWORD		WriteFlag = 0;

	SOCKADDR_IN	RemoteAddressInfo;
	INT			RemoteAddressInfoSize = sizeof(RemoteAddressInfo);

	WsaBuf.buf = (CHAR*)data;
	WsaBuf.len = dataLength;

	RemoteAddressInfo.sin_family = AF_INET;
	RemoteAddressInfo.sin_addr.S_un.S_addr = inet_addr(remoteAddress);
	RemoteAddressInfo.sin_port = htons(remotePort);

	INT		ReturnValue = WSASendTo(mSocket,
		&WsaBuf,
		1,
		&WriteBytes,
		WriteFlag,
		(SOCKADDR*)&RemoteAddressInfo,
		RemoteAddressInfoSize,
		&mWriteOverlapped.Overlapped,
		NULL);

	if (ReturnValue == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING && WSAGetLastError() != WSAEWOULDBLOCK)
	{
		End();

		return FALSE;
	}

	return TRUE;
}

BOOL CNetworkSession::GetRemoteAddressAfterAccept(LPTSTR remoteAddress, USHORT &remotePort)
{
	CThreadSync Sync;

	if (!remoteAddress)
		return FALSE;

	sockaddr_in		*Local = NULL;
	INT				LocalLength = 0;

	sockaddr_in		*Remote = NULL;
	INT				RemoteLength = 0;

	GetAcceptExSockaddrs(mReadBuffer,
		0,
		sizeof(sockaddr_in) + 16,
		sizeof(sockaddr_in) + 16,
		(sockaddr **)&Local,
		&LocalLength,
		(sockaddr **)&Remote,
		&RemoteLength);

	//_tcscpy(remoteAddress, (LPTSTR)(inet_ntoa(Remote->sin_addr)));
	CHAR	TempRemoteAddress[32] = { 0, };
	strcpy(TempRemoteAddress, inet_ntoa(Remote->sin_addr));

	MultiByteToWideChar(CP_ACP,
		0,
		TempRemoteAddress,
		-1,
		remoteAddress,
		32);

	remotePort = ntohs(Remote->sin_port);

	return TRUE;
}

SOCKET CNetworkSession::GetSocket(VOID)
{
	CThreadSync Sync;

	return mSocket;
}