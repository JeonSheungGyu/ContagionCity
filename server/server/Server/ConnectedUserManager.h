#pragma once


//ConnectedUser�� �����ϴ� Manager Ŭ����
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

	//��� CConnectedUSer ��ü�� Accept ��� ���·� ����ϴ�.
	BOOL	AcceptAll(VOID);
	//������ ��� ����ڿ��� ��Ŷ�� ������.
	BOOL	WriteAll(DWORD protocol, BYTE *data, DWORD dataLength);
	//���������� Ȯ��
	BOOL	IsAlreadyLogined(LPTSTR userID);
	//���� ����ڼ��� Interlocked������Ͽ� ��Ʈ���Ѵ�.
	inline VOID	IncreaseConnectedUserCount(VOID){ InterlockedIncrement((LONG*)&mCurrentUserCount); }
	inline VOID	DecreaseConnectedUserCount(VOID){ InterlockedDecrement((LONG*)&mCurrentUserCount); }
};
