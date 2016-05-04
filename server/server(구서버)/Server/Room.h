#pragma once

class CConnectedUser;


class CRoom : public CMultiThreadSync<CRoom>
{
public:
	CRoom(VOID);
	virtual ~CRoom(VOID);

private:
	DWORD					mIndex;
	TCHAR					mTitle[32];

	BOOL					mIsRoomStarted;
	BOOL					mIsGameStarted;

	DWORD					mMapIndex;

	CConnectedUser*			mRoomUser[8];
	USHORT					mCurrentUserCount;

	CConnectedUser			*mRootUser;

	DWORD					mGameTime;

	//////////////////////////////////////////////////////////////////////////


	//////////////////////////////////////////////////////////////////////////

public:
	BOOL					Begin(DWORD index);
	BOOL					End(VOID);

	BOOL					JoinUser(CConnectedUser *connectedUser, USHORT &slotIndex);
	BOOL					LeaveUser(BOOL isDisconnected, CGameIocp *iocp, CConnectedUser *connectedUser);

	BOOL					WriteAll(DWORD protocol, BYTE *packet, DWORD packetLength);
	BOOL					WriteAllInitHPAP(VOID);


	DWORD					RoomStart(VOID);
	BOOL					GameStart(VOID);
	BOOL					GameEnd(CGameIocp *iocp);

	inline BOOL				IncreaseGameTime(VOID){ CThreadSync Sync; mGameTime++; return TRUE; }
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

	//////////////////////////////////////////////////////////////////////////
};
