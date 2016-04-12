#include "stdafx.h"

#include "Character.h"
#include "ConnectedUser.h"

CConnectedUser::CConnectedUser(VOID)
{
	ZeroMemory(mUserID, sizeof(mUserID));
	ZeroMemory(mVirtualAddress, sizeof(mVirtualAddress));
	ZeroMemory(mRealAddress, sizeof(mRealAddress));
	ZeroMemory(mCharacters, sizeof(mCharacters));

	mVirtualPort = 0;
	mRealPort = 0;

	mStatus = US_NONE;

	mEnteredRoom = NULL;

	mIsConnected = FALSE;


	mSelectedCharacterSlot = 0;

	mCharacters[0].Initialize();
	mCharacters[1].Initialize();
	mCharacters[2].Initialize();
}

CConnectedUser::~CConnectedUser(VOID)
{
}

BOOL CConnectedUser::Begin(VOID)
{
	CThreadSync Sync;

	ZeroMemory(mUserID, sizeof(mUserID));
	ZeroMemory(mVirtualAddress, sizeof(mVirtualAddress));
	ZeroMemory(mRealAddress, sizeof(mRealAddress));
	ZeroMemory(mCharacters, sizeof(mCharacters));

	mVirtualPort = 0;
	mRealPort = 0;

	mStatus = US_NONE;

	mEnteredRoom = NULL;

	mIsConnected = FALSE;
	


	mSelectedCharacterSlot = 0;

	mCharacters[0].Initialize();
	mCharacters[1].Initialize();
	mCharacters[2].Initialize();

	return CPacketSession::Begin();
}

BOOL CConnectedUser::End(VOID)
{
	CThreadSync Sync;

	ZeroMemory(mUserID, sizeof(mUserID));
	ZeroMemory(mVirtualAddress, sizeof(mVirtualAddress));
	ZeroMemory(mRealAddress, sizeof(mRealAddress));
	ZeroMemory(mCharacters, sizeof(mCharacters));

	mVirtualPort = 0;
	mRealPort = 0;

	mStatus = US_NONE;

	mEnteredRoom = NULL;

	mIsConnected = FALSE;
	

	

	mSelectedCharacterSlot = 0;

	mCharacters[0].Initialize();
	mCharacters[1].Initialize();
	mCharacters[2].Initialize();

	return CPacketSession::End();
}

BOOL CConnectedUser::Reload(SOCKET listenSocket)
{
	CThreadSync Sync;

	End();

	if (!Begin())
		return FALSE;

	if (!CNetworkSession::Accept(listenSocket))
		return FALSE;

	return TRUE;
}