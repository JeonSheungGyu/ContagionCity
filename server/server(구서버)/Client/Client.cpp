// Client.cpp : �ܼ� ���� ���α׷��� ���� �������� �����մϴ�.
//

#include "stdafx.h"

#define READ_PACKET(PROTOCOL)\
	S_##PROTOCOL Data;\
	READ_##PROTOCOL(Packet, Data);

// Ŭ���̾�Ʈ ����� ����� CTestClientSession ��ü
// CClientSession�� ��ӹ޾Ƽ� ����մϴ�.
class CTestClientSession : public CClientSession
{
protected:
	// Ŭ���̾�Ʈ�� ���� �����Ǿ����� ȣ��Ǵ� �����Լ�
	VOID OnIoConnected(VOID)
	{
		//_tprintf(_T("OnIoConnected\n"));
	}

	// Ŭ���̾�Ʈ�� ���� ����Ǿ����� ȣ��Ǵ� �����Լ�
	VOID OnIoDisconnected(VOID)
	{
		//_tprintf(_T("OnIoDisconnected\n"));
	}
};

int _tmain(int argc, _TCHAR* argv[])
{
	// Winsock�� ����ϱ� ���� DLL �ε�
	WSADATA WsaData;
	WSAStartup(MAKEWORD(2, 2), &WsaData);	// 2.2 ������ �ε��մϴ�.

	// ��� ��ü�� �����մϴ�.
	CTestClientSession *pClientSession = new CTestClientSession();

	// TCP�� ����Ұ��� �����ϰ� ������ IP, Port�� �����մϴ�.
	// �������� ��� getchar�� ���� ��� ���� ���°� �˴ϴ�.
	if (pClientSession->BeginTcp("127.0.0.1", DEFAULT_PORT))
	{
		// �Ѱ��� ��ɾ ��� ó���ϱ� ���� while���Դϴ�.
		while (TRUE)
		{
			// ��Ŷ�� ������ ����ϴ� ����
			BYTE WriteBuffer[MAX_BUFFER_LENGTH] = {0,};
			// ��Ŷ�� ������ ����ϴ� ��������, ����, ��Ŷ ����
			DWORD dwProtocol = 0, dwPacketLength = 0;
			BYTE Packet[MAX_BUFFER_LENGTH] = {0,};

			// ȭ���� �ѹ� �����ݴϴ�.
			system("cls");

			WCHAR UserID[32] = { 0, };
			WCHAR Address[32] = { 0, };
			USHORT Port = 0;

			// USER_ID�� �Է¹޽��ϴ�.
			ZeroMemory(UserID, sizeof(UserID));
			_tcscpy(UserID, _T("ssyauu159"));
			ZeroMemory(Address, sizeof(Address));
			_tcscpy(Address, _T("127.0.0.1"));
			Port = DEFAULT_PORT;


			// ��Ŷ�� �����մϴ�.
			pClientSession->WritePacket(PT_CHANNEL_ENTER,
				WriteBuffer,
				WRITE_PT_CHANNEL_ENTER(WriteBuffer, // WRITE �Լ�
				UserID,
				Address,
				Port));
			
			// Ű����� �Է��� ������ �����ϴ� ����
			// ������ ��� Ű����� �Է��� ������ �̰��� ����˴ϴ�.
			TCHAR szCommand[32] = {0,};
			// USER, COMPUTER, PROGRAM�� ������ �޽��ϴ�.
			_tprintf(_T("Enter command (ROOM) : "));
			ZeroMemory(szCommand, sizeof(szCommand));
			_getts(szCommand);
			
			// Ű����� �Է¹��� ������ �ҹ��ڷ� �ٲپ� �񱳸� �մϴ�.
			if (!_tcscmp(_tcslwr(szCommand), _T("room")))
			{
				_tprintf(_T("=== ROOM ===\n"));
				// REG���� QUERY������ �����մϴ�.
				_tprintf(_T("Enter command (enter/leave) : "));
				ZeroMemory(szCommand, sizeof(szCommand));
				_getts(szCommand);

				// REG�� ���
				if (!_tcscmp(_tcslwr(szCommand), _T("enter")))
				{

					// ��Ŷ�� �����մϴ�.
					pClientSession->WritePacket(PT_ROOM_QUICK_JOIN, 
						WriteBuffer, 
						WRITE_PT_ROOM_QUICK_JOIN(WriteBuffer));

					_tprintf(_T("Press any key..."));
					getchar();
				}
				// QUERY�� ���
				else if (!_tcscmp(_tcslwr(szCommand), _T("leave")))
				{
					//WCHAR szUserID[32] = {0,};

					//// �˻��� USER_ID�� �Է� �޽��ϴ�.
					//_tprintf(_T("Enter USER_ID : "));
					//ZeroMemory(szCommand, sizeof(szCommand));
					//_getts(szCommand);
					//_tcscpy(szUserID, szCommand);

					//// �˻� ��Ŷ�� �����մϴ�.
					//pClientSession->WritePacket(PT_QUERY_USER, 
					//	WriteBuffer, 
					//	WRITE_PT_QUERY_USER(WriteBuffer, 
					//	szUserID));

					//// ������ ��Ŷ�� ����� �� ������ ��� ��Ŷ�� �н��ϴ�.
					//// ��� �д� ������ Server�� �߰��ߴ� KeepAlive ��Ŷ �����Դϴ�.
					//while (TRUE) if (!pClientSession->ReadPacket(dwProtocol, Packet, dwPacketLength)) if (dwProtocol == PT_QUERY_USER_RESULT) break;

					//// ��Ŷ�� �н��ϴ�.
					//READ_PACKET(PT_QUERY_USER_RESULT);

					//// ���� ��Ŷ�� ����մϴ�.
					//_tprintf(_T("QUERY : USER_ID(%s), USER_NAME(%s), AGE(%d), SEX(%d), ADDRESS(%s)\n"), Data.USER_ID, Data.USER_NAME, Data.AGE, Data.SEX, Data.ADDRESS);

					//_tprintf(_T("Press any key..."));
					//getchar();
				}
			}
			
		}
	}

	// �ƹ�Ű�� ������ ��� ����������� ó���˴ϴ�.
	pClientSession->End();

	delete pClientSession;

	// Winsock�� Unload�մϴ�.
	WSACleanup();

	return 0;
}

