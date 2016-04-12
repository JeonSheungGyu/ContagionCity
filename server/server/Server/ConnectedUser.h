#pragma once
#include "Character.h"
class CRoom;




class CConnectedUser : public CPacketSession
{
public:
	CConnectedUser(VOID);
	virtual ~CConnectedUser(VOID);

private:
	//�������� ( Virtual( ���ξ����� ), Real ( �缳������ )
	TCHAR				mVirtualAddress[32];
	TCHAR				mRealAddress[32];

	USHORT				mVirtualPort;
	USHORT				mRealPort;

	//���� �������ͽ�, ĳ���� �������ͽ��� ����
	USER_STATUS			mStatus;

	//���̵�
	TCHAR				mUserID[32];

	//������ ��
	CRoom				*mEnteredRoom;

	//���ӿ���
	BOOL				mIsConnected;

	//ĳ���� ����
	CCharacter			mCharacters[3];

	//������ ĳ���� ����
	USHORT				mSelectedCharacterSlot;
	//////////////////////////////////////////////////////////////////////////
public:
	BOOL				Begin(VOID);
	BOOL				End(VOID);

	BOOL				Reload(SOCKET listenSocket);

	inline BOOL			SetIsConnected(BOOL isConnected){ CThreadSync Sync; mIsConnected = isConnected; return TRUE; }
	inline BOOL			GetIsConnected(VOID){ CThreadSync Sync; return mIsConnected; }


	inline BOOL			SetVirtualInformation(LPTSTR virtualAddress, USHORT virtualPort){ CThreadSync Sync; if (!virtualAddress || virtualPort <= 0)return FALSE; _tcsncpy(mVirtualAddress, virtualAddress, 32); mVirtualPort = virtualPort; return TRUE; }
	inline BOOL			SetRealInformation(LPTSTR realAddress, USHORT realPort){ CThreadSync Sync; if (!realAddress || realPort <= 0)return FALSE; _tcsncpy(mRealAddress, realAddress, 32); mRealPort = realPort; return TRUE; }
	inline BOOL			SetUserID(LPTSTR userID){ CThreadSync Sync; if (!userID)return FALSE; _tcsncpy(mUserID, userID, 32); return TRUE; }
	inline LPTSTR		GetUserID(VOID){ CThreadSync Sync; return mUserID; }
	inline LPTSTR		GetRealAddress(VOID){ CThreadSync Sync; return mRealAddress; }
	inline LPTSTR		GetVirtualAddress(VOID){ CThreadSync Sync; return mVirtualAddress; }
	inline USHORT		GetRealPort(VOID){ CThreadSync Sync; return mRealPort; }
	inline USHORT		GetVirtualPort(VOID){ CThreadSync Sync; return mVirtualPort; }
	
	inline BOOL			SetStatus(USER_STATUS status){ CThreadSync Sync; mStatus = status; return TRUE; }
	inline USER_STATUS	GetStatus(VOID){ CThreadSync Sync; return mStatus; }

	inline BOOL			SetEnteredRoom(CRoom *room){ CThreadSync Sync; mEnteredRoom = room; return TRUE; }
	inline CRoom*		GetEnteredRoom(VOID){ CThreadSync Sync; return mEnteredRoom; }
	

	inline CCharacter*	GetCharacterObject(USHORT slotIndex){ return &mCharacters[slotIndex]; }

	inline USHORT		GetSelectedCharacterSlot(VOID){ CThreadSync Sync; return mSelectedCharacterSlot; }
	inline VOID			SetSelectedCharacterSlot(USHORT slot){ CThreadSync Sync; mSelectedCharacterSlot = slot; }

	//////////////////////////////////////////////////////////////////////////
};
