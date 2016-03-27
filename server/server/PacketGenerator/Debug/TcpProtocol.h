#pragma once

typedef struct _SLOT_USER_DATA{
	WCHAR UserID[32]; //����� ���̵�
	DWORD_PTR SessionID; //����� ������ȣ
	WCHAR VirtualAddress[32]; //������� �缳 ������
	USHORT VirtualPort;
	WCHAR RealAddress[32];
	USHORT RealPort;
	BOOL IsReady; //  ������� FLAG
}SLOT_USER_DATA;

//���� ��������

typedef enum TCP_PROTOCOL{

	//�������� ����
	//������ ũ�� �ٲ� ��� ���ڸ� �������� �ش�.
	PT_VERSION = 0x1000000,
	PT_CHANNEL_ENTER,
	/*
		WCHAR : USER_ID[32]
		WCHAR : VIRTUAL_ADDRESS[32]
		USHORT : VIRTUAL_PORT
	*/
	PT_CHANNEL_ENTER_SUCC_U,
	/*
		DOWRD_PTR : SESSION_ID
		WCHAR : VIRTUAL_ADDRESS[32]
		USHORT : VIRTUAL_PORT
		WCHAR : REAL_ADDRESS[32]
		USHORT : REAL_PORT

	*/
	PT_CHANNEL_ENTER_FAIL_U,
	/*
		DWORD : ERRROR_CODE
	*/
	
	//�濡 ������ ����� ����
	//��� ������� �Ѹ��� ��������

	PT_ROOM_ENTER_M,
	/*
		DWORD : SLOT_POSITION
		WCHAR : USER_ID[32]
		DWORD_PTR : SESSION_ID
		DWORD : VIRTUAL_ADDRESS[32]
		USHORT : VIRTUAL_PORT 
		WCHAR : REAL_ADDRESS[32]
		USHORT : REAL_PORT
	*/
	PT_ROOM_REQ_INFO,
	/*
	*/
	PT_ROOM_REQ_INFO_SUCC_U,
	/*
		DWORD : ROOM_INDEX 
		WCHAR : TITLE[32] 
		DWORD : MAP_INDEX 
		USHORT : CURRENT_USER_COUNT 
		DWORD_PTR : ROOT_USER_SESSION_ID 
		SLOT_USER_DATA : DATA[8] 
	*/
	PT_ROOM_REQ_INFO_FAIL_U,
	/*
		DWORD : ERROR_CODE
	*/
	//�� ������
	PT_ROOM_LEAVE,
	/*
	*/
	PT_ROOM_LEAVE_SUCC_U,
	/*
	*/
	PT_ROOM_LEAVE_FAIL_U,
	/*
		DWORD : ERROR_CODE
	*/
	//�� ���� ����� �˸�
	PT_ROOM_LEAVE_M,
	/*
		DWORD_PTR : SESSION_ID 
		DWORD_PTR : ROOT_USER_SESSION_ID 
	*/

	//�� ����
	//������ ������ ���� �����Ѵ�.
	PT_ROOM_MAP_CHANGE,
	/*
		DWORD : MAP_INDEX 
	*/
	PT_ROOM_MAP_SUCC_U,
	/*
	*/
	PT_ROOM_MAP_FAIL_U,
	/*
		DWORD : ERROR_CODE 
	*/
	//�� ���� �˸�
	PT_ROOM_MAP_CHANGE_M,
	/*
		DWORD : MAP_INDEX
	*/
	PT_ROOM_CHATTING,
	/*
		WCHAR : CHAT[256] 
	*/
	PT_ROOM_CHATTING_SUCC_U,
	/*
	*/
	PT_ROOM_CHATTING_FAIL_U,
	/*
		DWORD : ERROR_CODE
	*/
	//ä�� ���� �˸�
	PT_ROOM_CHATTING_M,
	/*
		DWORD_PTR : SESSION_ID 
		WCHAR : CHAT[256]
	*/
	//����� ���� 
	PT_ROOM_READY,
	/*
		BOOL : READY 
	*/
	PT_ROOM_READY_SUCC_U,
	/*
	*/
	PT_ROOM_READY_FAIL_U,
	/*
		DWORD : ERROR_CODE 
	*/
	//���� �˸�
	PT_ROOM_READY_M,
	/*
		DWORD_PTR : SESSION_ID 
		BOOL : READY 
	*/
	//���ӽ���
	PT_ROOM_START,
	/*
	*/
	PT_ROOM_START_SUCC_U,
	/*
	*/
	PT_ROOM_START_FAIL_U,
	/*
		DWORD : ERROR_CODE
	*/
	//���ӽ��� �˸�
	PT_ROOM_START_M,
	/*
	*/
	//�ε��Ϸ�
	PT_GAME_LOAD_COMPLETE,
	/*
	*/
	PT_GAME_LOAD_COMPLETE_SUCC_U,
	/*
	*/
	PT_GAME_LOAD_COMPLETE_FAIL_U,
	/*
		DWORD : ERROR_CODE
	*/
	//�ε��Ϸ�˸�
	PT_GAME_LOAD_COMPLETE_M,
	/*
		DWORD_PTR : SESSION_ID 
	*/
	//��� ����ڰ� �ε��� �Ϸ��
	PT_GAME_ALL_LOAD_COMPLETE_M,
	/*
	*/
	//�ʵ� �Ұ� 
	PT_GAME_INTRO_COMPLETE,
	/*
	*/
	PT_GAME_INTRO_COMPLETE_SUCC_U,
	/*
	*/
	PT_GAME_INTRO_COMPLETE_FAIL_U,
	/*
		DWORD : ERROR_CODE
	*/
	//�ʵ� �Ұ� �˸�
	PT_GAME_INTRO_COMPLETE_M,
	/*
		DWORD_PTR : SESSION_ID 
	*/
	PT_GAME_ALL_INTRO_COMPLETE_M,
	/*
	*/

	//�������ӽ���
	PT_GAME_START_M,
	/*
	*/
	//��������
	PT_GAME_END_M,
	/*
	*/
};