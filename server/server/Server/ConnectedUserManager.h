#pragma once


//ConnectedUser를 관리하는 Manager 클래스
class CConnectedUserManager : public CMultiThreadSync<CConnectedUserManager>
{
public:
	CConnectedUserManager(VOID);
	virtual ~CConnectedUserManager(VOID);

private:
	std::vector<CConnectedUser*>	mConnectedUserVector;
	DWORD							mMaxUserCount;

	DWORD							mCurrentUserCount;

	SOCKET							mListenSocket;

public:
	BOOL	Begin(DWORD maxUserCount, SOCKET listenSocket);
	BOOL	End(VOID);

	//모든 CConnectedUSer 개체를 Accept 대기 상태로 만듭니다.
	BOOL	AcceptAll(VOID);
	//접속한 모든 사용자에게 패킷을 보낸다.
	BOOL	WriteAll(DWORD protocol, BYTE *data, DWORD dataLength);
	//접속중인지 확인
	BOOL	IsAlreadyLogined(LPTSTR userID);
	//현재 사용자수를 Interlocked을사용하여 컨트롤한다.
	inline VOID	IncreaseConnectedUserCount(VOID){ InterlockedIncrement((LONG*)&mCurrentUserCount); }
	inline VOID	DecreaseConnectedUserCount(VOID){ InterlockedDecrement((LONG*)&mCurrentUserCount); }
};
