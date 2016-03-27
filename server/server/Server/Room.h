#pragma once

class CRoom : public CMultiThreadSync<CRoom>
{
public:
	CRoom(VOID);
	virtual ~CRoom(VOID);

private:
	DWORD					mIndex;	// �� �ε���
	TCHAR					mTitle[32]; // �� ����

	BOOL					mIsRoomStarted; // �� ���۹�ư ���������� Ȯ���ϴ� FLAG
	BOOL					mIsGameStarted; // ���������� Ȯ���ϴ� FLAG

	DWORD					mMapIndex; // �� �ε���

	CConnectedUser*			mRoomUser[8]; // User
	USHORT					mCurrentUserCount; // UserCount

	CConnectedUser			*mRootUser; // Root User

	DWORD					mGameTime; // ���� ���� �ð�

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

public:
	BOOL					Begin(DWORD index);
	BOOL					End(VOID);

	BOOL					JoinUser(CConnectedUser *connectedUser, USHORT &slotIndex);
	BOOL					LeaveUser(BOOL isDisconnected, CGameIocp *iocp, CConnectedUser *connectedUser);

	BOOL					WriteAll(DWORD protocol, BYTE *packet, DWORD packetLength);
	BOOL					WriteAllInitHPAP(VOID);

	BOOL					IsAllLoadComplete(VOID);
	BOOL					IsAllIntroComplete(VOID);

	DWORD					RoomStart(VOID);
	BOOL					GameStart(VOID);
	BOOL					GameEnd(CGameIocp *iocp);

	inline BOOL				IncreaseGameTime(VOID){ CThreadSync Sync; mGameTime++; return true; }
	inline BOOL				GetIsEmpty(VOID){ CThreadSync Sync; return mCurrentUserCount ? FALSE : TRUE; }
	inline BOOL				GetIsFull(VOID){ CThreadSync Sync; return mCurrentUserCount == 8 ? TRUE : FALSE; }
	inline BOOL				GetIsRoomStarted(VOID){ CThreadSync Sync; return mIsRoomStarted; }
	inline BOOL				GetIsGameStarted(VOID){ CThreadSync Synd; return mIsGameStarted; }
	inline CConnectedUser** GetRoomUser(VOID){ CThreadSync Sync; return mRoomUser; }
	inline DWORD			GetIndex(VOID){ CThreadSync Sync; return mIndex; }
	inline LPTSTR			GetTitle(VOID){ CThreadSync Sync; return mTitle; }
	inline DWORD			GetMapIndex(VOID){ CThreadSync Sync; return mMapIndex; }
	inline DWORD			SetMapIndex(DWORD mapIndex){ CThreadSync Sync; mMapIndex = mapIndex; return TRUE; }
	inline USHORT			GetCurrentUserCount(VOID){ CThreadSync Sync; return mCurrentUserCount; }
	inline CConnectedUser*	GetRootUser(VOID){ CThreadSync Sync; return mRootUser; }

	DWORD					GetTeam(CConnectedUser *player);
};
