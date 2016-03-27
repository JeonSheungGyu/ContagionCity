#include "stdafx.h"

#include "../Packet/TcpProtocol.h"
#include "../Packet/Tcp_Structure.h"
#include "../Packet/Tcp_ReadPacket.h"
#include "../Packet/Tcp_WritePacket.h"


CRoom::CRoom(VOID)
{
	mIndex = 0;
	mIsRoomStarted = FALSE;
	mMapIndex = 0;
	mCurrentUserCount = 0;
	mRootUser = NULL;
	mIsGameStarted = FALSE;

	mGameTime = DEFAULT_GAME_TIME;

	ZeroMemory(mTitle, sizeof(mTitle));
	ZeroMemory(mRoomUser, sizeof(mRoomUser));
}

CRoom::~CRoom(VOID)
{
}

BOOL CRoom::Begin(DWORD index)
{
	CThreadSync Sync;

	mIndex = index;
	mIsRoomStarted = FALSE;
	mMapIndex = 0;
	mCurrentUserCount = 0;
	mRootUser = NULL;
	mIsGameStarted = FALSE;

	mGameTime = DEFAULT_GAME_TIME;


	ZeroMemory(mTitle, sizeof(mTitle));
	ZeroMemory(mRoomUser, sizeof(mRoomUser));

	return TRUE;
}

BOOL CRoom::End(VOID)
{
	CThreadSync Sync;

	mIndex = 0;
	mIsRoomStarted = FALSE;
	mMapIndex = 0;
	mCurrentUserCount = 0;
	mRootUser = NULL;
	mIsGameStarted = FALSE;

	mGameTime = DEFAULT_GAME_TIME;


	ZeroMemory(mTitle, sizeof(mTitle));
	ZeroMemory(mRoomUser, sizeof(mRoomUser));

	return TRUE;
}

BOOL CRoom::JoinUser(CConnectedUser *connectedUser, USHORT &slotIndex)
{
	CThreadSync Sync;

	if (!connectedUser)
		return FALSE;

	// ���� �´��� Ȯ���Ѵ�.
	USHORT	RedTeamCount = 0;
	USHORT	BlueTeamCount = 0;

	for (USHORT i = 0; i<8; i++)
	{
		// �ش� ����ڸ� ã�´�.
		if (mRoomUser[i] && i < 4)
			RedTeamCount++;
		else if (mRoomUser[i] && i >= 4)
			BlueTeamCount++;
	}

	// FULL
	if (RedTeamCount + BlueTeamCount == 8)
		return FALSE;

	USHORT StartPos = RedTeamCount <= BlueTeamCount ? 0 : 4;

	for (USHORT i = StartPos; i<8; i++)
	{
		// �� ���� ��ġ ������ �ʿ��ϴ�.
		// ���ڸ��� ã���ش�.

		if (mRoomUser[i] == NULL)
		{
			mRoomUser[i] = connectedUser;
			connectedUser->SetEnteredRoom(this);

			mCurrentUserCount = min(SHORT(mCurrentUserCount++), 8);

			slotIndex = i;

			// �� ó�� ����
			if (mCurrentUserCount == 1)
			{
				mRootUser = connectedUser;
				mMapIndex = 0;
			}

			return TRUE;
		}
	}

	return FALSE;
}

BOOL CRoom::LeaveUser(BOOL isDisconnected, CGameIocp *iocp, CConnectedUser *connectedUser)
{
	CThreadSync Sync;

	BYTE	WriteBuffer[MAX_BUFFER_LENGTH] = { 0, };

	if (!connectedUser)
		return FALSE;

	for (USHORT i = 0; i<8; i++)
	{
		// �ش� ����ڸ� ã�´�.
		if (mRoomUser[i] == connectedUser)
		{
			mRoomUser[i] = NULL;
			connectedUser->SetEnteredRoom(NULL);

			mCurrentUserCount = max(SHORT(mCurrentUserCount--), 0);

			if (connectedUser == mRootUser)
			{
				mRootUser = NULL;

				for (USHORT j = 0; j<8; j++)
				{
					if (mRoomUser[j])
					{
						mRootUser = mRoomUser[j];

						break;
					}
				}
			}

			if (!isDisconnected)
			{
				// �� ������ ������ ������
				connectedUser->WritePacket(PT_ROOM_LEAVE_SUCC_U,
					WriteBuffer,
					WRITE_PT_ROOM_LEAVE_SUCC_U(WriteBuffer));

				CLog::WriteLog(_T("# Write packet : PT_ROOM_LEAVE_SUCC_U\n"));
			}

			WriteAll(PT_ROOM_LEAVE_M,
				WriteBuffer,
				WRITE_PT_ROOM_LEAVE_M(WriteBuffer,
				(DWORD_PTR)connectedUser,
				(DWORD_PTR)mRootUser));

			CLog::WriteLog(_T("# WriteAll packet : PT_ROOM_LEAVE_M 0x%x/0x%x\n"),
				(DWORD_PTR)connectedUser,
				(DWORD_PTR)mRootUser);

			if (mIsRoomStarted)
			{
				// Game End�� �����ش�.
				GameEnd(iocp);

				// ��� ��� ����ڿ��� Game End�� ������. (���� ����ڵ��� ������ ���ư� ���� �ȵ� ���·� ���)
				WriteAll(PT_GAME_END_M,
					WriteBuffer,
					WRITE_PT_GAME_END_M(WriteBuffer));

				CLog::WriteLog(_T("# WriteAll packet : PT_GAME_END_M\n"));
			}

			return TRUE;
		}
	}

	return FALSE;
}

BOOL CRoom::WriteAll(DWORD protocol, BYTE *packet, DWORD packetLength)
{
	CThreadSync Sync;

	if (protocol <= 0 || !packet)
		return FALSE;

	for (USHORT i = 0; i<8; i++)
	{
		// �ش� ����ڸ� ã�´�.
		if (mRoomUser[i])
			mRoomUser[i]->WritePacket(protocol, packet, packetLength);
	}

	return TRUE;
}

DWORD CRoom::RoomStart(VOID)
{
	CThreadSync Sync;
#ifdef _CHECK_MODULE
	// ������� ���� Ready���� Ȯ���Ѵ�.
	for (USHORT i = 0; i<8; i++)
	{
		// �ش� ����ڸ� ã�´�.
		if (mRoomUser[i])
		{
			if (!mRoomUser[i]->GetIsReady())
				return EC_ROOM_START_FAIL_ALL_READY;
		}
	}

	// ����ڰ� ¦������ Ȯ���Ѵ�.
	if (mCurrentUserCount % 2 != 0)
		return EC_ROOM_START_FAIL_TEAM_INCORRECT;

	// ���� �´��� Ȯ���Ѵ�.
	USHORT	RedTeamCount = 0;
	USHORT	BlueTeamCount = 0;

	for (USHORT i = 0; i<8; i++)
	{
		// �ش� ����ڸ� ã�´�.
		if (mRoomUser[i] && i < 4)
			RedTeamCount++;
		else if (mRoomUser[i] && i >= 4)
			BlueTeamCount++;
	}

	if (RedTeamCount != BlueTeamCount)
		return EC_ROOM_START_FAIL_TEAM_INCORRECT;
#endif
	mIsRoomStarted = TRUE;

	// ��� ����� ���¸� ��ȭ�� �ش�.
	for (USHORT i = 0; i<8; i++)
	{
		if (mRoomUser[i])
		{
			mRoomUser[i]->SetStatus(US_GAME_STARTING);
			mRoomUser[i]->SetIsReady(FALSE);
		}
	}

	return 0;
}

BOOL CRoom::GameStart(VOID)
{
	CThreadSync Sync;

	// ��� IntroComplete���� Ȯ���Ѵ�.
	for (USHORT i = 0; i<8; i++)
	{
		if (mRoomUser[i])
		{
			if (!mRoomUser[i]->GetIsIntroComplete())
				return FALSE;
		}
	}

	mIsGameStarted = TRUE;
	mGameTime = DEFAULT_GAME_TIME;

	// ����� ��� TRUE�� ���� �� �ְ� LoadComplete �ʱ�ȭ
	// ��� ����� ���ӻ��·� ����
	for (USHORT i = 0; i<8; i++)
	{
		if (mRoomUser[i])
		{
			mRoomUser[i]->SetStatus(US_GAME_STARTED);
			mRoomUser[i]->SetIsLoadComplete(FALSE);
			mRoomUser[i]->SetIsIntroComplete(FALSE);
			mRoomUser[i]->InitializeForGameStart();
		}
	}



	return TRUE;
}

BOOL CRoom::GameEnd(CGameIocp *iocp)
{
	CThreadSync Sync;

	// ���� ���� ó��

	// ��� �뿡 ������� HP�� ���ؼ� �������� �̱�Ŵ�
	// Ȥ�ó� ������ ���º�
	DWORD	Team0TotalHP = 0;
	DWORD	Team1TotalHP = 0;
	USHORT	WinTeam = 0;

	for (USHORT i = 0; i<4; i++)
	{
		if (mRoomUser[i])
			Team0TotalHP += mRoomUser[i]->GetHP();
	}

	for (USHORT i = 4; i<8; i++)
	{
		if (mRoomUser[i])
			Team1TotalHP += mRoomUser[i]->GetHP();
	}

	if (Team0TotalHP < Team1TotalHP)
		WinTeam = 1;

	// ���º� �϶�
	if (Team0TotalHP == Team1TotalHP)
		WinTeam = 2;

	//#ifdef _INCLUDE_DATABASE
	//	// ���� �Ӵ�, ����ġ ������Ʈ
	//	for (USHORT i=0;i<8;i++)
	//	{
	//		if (mRoomUser[i])
	//		{
	//			mRoomUser[i]->SetGameMoney(mRoomUser[i]->GetGameMoney() + DEFAULT_GAMEMONEY);
	//			mRoomUser[i]->SetExp(mRoomUser[i]->GetExp() + DEFAULT_EXP);
	//
	//			iocp->db_SetGameMoneyAndExp(mRoomUser[i]->GetUserID(), mRoomUser[i]->GetCharacterObject(mRoomUser[i]->GetSelectedCharacterSlot())->GetIndex(), mRoomUser[i]->GetGameMoney(), mRoomUser[i]->GetExp());
	//		}
	//	}
	//#endif

	BYTE WriteBuffer[MAX_BUFFER_LENGTH] = { 0, };
	/*
	WriteAll(PT_GAME_RESULT_M,
		WriteBuffer,
		WRITE_PT_GAME_RESULT_M(WriteBuffer,
		WinTeam,
		Team0TotalHP,
		Team1TotalHP, 0));
	*/
	CLog::WriteLog(_T("# WriteAll packet : PT_GAME_RESULT_M %d/%d/%d/%d\n"),
		WinTeam,
		Team0TotalHP,
		Team1TotalHP,
		0/*Game Money*/);

	// READY, LOADCOMPLETE, IsStarted, USER_STATS �� �ʱ�ȭ
	for (USHORT i = 0; i<8; i++)
	{
		if (mRoomUser[i])
		{
			mRoomUser[i]->SetStatus(US_ROOM_ENTERED);
			mRoomUser[i]->SetIsLoadComplete(FALSE);
			mRoomUser[i]->SetIsIntroComplete(FALSE);
			mRoomUser[i]->SetIsReady(FALSE);
			mRoomUser[i]->InitializeForGameStart();
		}
	}

	mIsRoomStarted = FALSE;
	mIsGameStarted = FALSE;

	return TRUE;
}
