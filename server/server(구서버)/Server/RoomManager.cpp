#include "stdafx.h"


#include "Character.h"
#include "Room.h"
#include "RoomManager.h"
#include "ConnectedUser.h"
#include "ConnectedUserManager.h"
#include "GameIocp.h"

CRoomManager::CRoomManager(VOID)
{
}

CRoomManager::~CRoomManager(VOID)
{
}

BOOL CRoomManager::Begin(DWORD maxRoomCount)
{
	if (maxRoomCount <= 0)
		return FALSE;

	mMaxRoomCount = maxRoomCount;

	for (DWORD i = 0; i<mMaxRoomCount; i++)
	{
		CRoom *Room = new CRoom();

		if (Room->Begin(i))

			mRoomVector.push_back(Room);
		else
		{
			End();

			return FALSE;
		}
	}

	return TRUE;
}

BOOL CRoomManager::End(VOID)
{
	CThreadSync Sync;

	for (DWORD i = 0; i<mRoomVector.size(); i++)
	{
		CRoom *Room = mRoomVector[i];

		Room->End();

		delete Room;
	}

	mRoomVector.clear();

	return TRUE;
}

CRoom* CRoomManager::QuickJoin(CConnectedUser *connectedUser, USHORT &slotIndex)
{
	CThreadSync Sync;

	if (!connectedUser)
		return NULL;

	for (DWORD i = 0; i<mRoomVector.size(); i++)
	{
		CRoom *Room = mRoomVector[i];

		if (!Room->GetIsFull() && !Room->GetIsEmpty() && !Room->GetIsRoomStarted())
		{
			// ����� �ְ� ������ ����������~ ���۵� ���� �ʾҰ�..
			if (!Room->JoinUser(connectedUser, slotIndex))
				continue;
			else
				return Room;
		}
	}

	// ����ִ� ���� ���ų�,,, ��� �� ã�� ����.
	// �켱 ����� ã�´�.
	for (DWORD i = 0; i<mRoomVector.size(); i++)
	{
		CRoom *Room = mRoomVector[i];

		if (Room->GetIsEmpty())
		{
			if (!Room->JoinUser(connectedUser, slotIndex))
				continue;
			else
				return Room;
		}
	}

	// ��¿�� ����... �� á��
	return NULL;
}

BOOL CRoomManager::CheckGameTime(CGameIocp *iocp)
{
	CThreadSync Sync;

	//BYTE	WriteBuffer[MAX_BUFFER_LENGTH] = { 0, };

	//for (DWORD i = 0; i<mRoomVector.size(); i++)
	//{
	//	CRoom *Room = mRoomVector[i];

	//	// ���� ������ �������� ���
	//	if (Room->GetIsGameStarted())
	//	{
	//		// ���� ����
	//		if (!Room->IncreaseGameTime())
	//		{
	//			Room->GameEnd(iocp);

	//			Room->WriteAll(PT_GAME_END_M, WriteBuffer, WRITE_PT_GAME_END_M(WriteBuffer));

	//			CLog::WriteLog(_T("# WriteAll packet : PT_GAME_END_M\n"));
	//		}
	//		else{
	//			//Room->CheckExplosion(iocp);
	//		}

	//	}
	//}

	return TRUE;
}
