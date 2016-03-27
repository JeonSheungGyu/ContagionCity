#pragma once

inline DWORD WRITE_PT_CHANNEL_ENTER(BYTE *buffer, S_PT_CHANNEL_ENTER &parameter)
{
	CStreamSP Stream;
	Stream->SetBuffer(buffer);

	Stream->WriteWCHARs(parameter.USER_ID, 32);
	Stream->WriteWCHARs(parameter.VIRTUAL_ADDRESS, 32);
	Stream->WriteUSHORT(parameter.VIRTUAL_PORT);

	return Stream->GetLength();
}

inline DWORD WRITE_PT_CHANNEL_ENTER_SUCC_U(BYTE *buffer, S_PT_CHANNEL_ENTER_SUCC_U &parameter)
{
	CStreamSP Stream;
	Stream->SetBuffer(buffer);

	Stream->WriteBytes((BYTE*) parameter.SESSION_ID, sizeof(DOWRD_PTR) * 0);
	Stream->WriteWCHARs(parameter.VIRTUAL_ADDRESS, 32);
	Stream->WriteUSHORT(parameter.VIRTUAL_PORT);
	Stream->WriteWCHARs(parameter.REAL_ADDRESS, 32);
	Stream->WriteUSHORT(parameter.REAL_PORT);

	return Stream->GetLength();
}

inline DWORD WRITE_PT_CHANNEL_ENTER_FAIL_U(BYTE *buffer, S_PT_CHANNEL_ENTER_FAIL_U &parameter)
{
	CStreamSP Stream;
	Stream->SetBuffer(buffer);

	Stream->WriteDWORD(parameter.ERRROR_CODE);

	return Stream->GetLength();
}

inline DWORD WRITE_PT_ROOM_ENTER_M(BYTE *buffer, S_PT_ROOM_ENTER_M &parameter)
{
	CStreamSP Stream;
	Stream->SetBuffer(buffer);

	Stream->WriteDWORD(parameter.SLOT_POSITION);
	Stream->WriteWCHARs(parameter.USER_ID, 32);
	Stream->WriteDWORD_PTR(parameter.SESSION_ID);
	Stream->WriteDWORD(parameter.VIRTUAL_ADDRESS);
	Stream->WriteUSHORT(parameter.VIRTUAL_PORT);
	Stream->WriteWCHARs(parameter.REAL_ADDRESS, 32);
	Stream->WriteUSHORT(parameter.REAL_PORT);

	return Stream->GetLength();
}

inline DWORD WRITE_PT_ROOM_REQ_INFO(BYTE *buffer, S_PT_ROOM_REQ_INFO &parameter)
{
	CStreamSP Stream;
	Stream->SetBuffer(buffer);


	return Stream->GetLength();
}

inline DWORD WRITE_PT_ROOM_REQ_INFO_SUCC_U(BYTE *buffer, S_PT_ROOM_REQ_INFO_SUCC_U &parameter)
{
	CStreamSP Stream;
	Stream->SetBuffer(buffer);

	Stream->WriteDWORD(parameter.ROOM_INDEX);
	Stream->WriteWCHARs(parameter.TITLE, 32);
	Stream->WriteDWORD(parameter.MAP_INDEX);
	Stream->WriteUSHORT(parameter.CURRENT_USER_COUNT);
	Stream->WriteDWORD_PTR(parameter.ROOT_USER_SESSION_ID);
	Stream->WriteBytes((BYTE*) parameter.DATA, sizeof(SLOT_USER_DATA) * 8);

	return Stream->GetLength();
}

inline DWORD WRITE_PT_ROOM_REQ_INFO_FAIL_U(BYTE *buffer, S_PT_ROOM_REQ_INFO_FAIL_U &parameter)
{
	CStreamSP Stream;
	Stream->SetBuffer(buffer);

	Stream->WriteDWORD(parameter.ERROR_CODE);

	return Stream->GetLength();
}

inline DWORD WRITE_PT_ROOM_LEAVE(BYTE *buffer, S_PT_ROOM_LEAVE &parameter)
{
	CStreamSP Stream;
	Stream->SetBuffer(buffer);


	return Stream->GetLength();
}

inline DWORD WRITE_PT_ROOM_LEAVE_SUCC_U(BYTE *buffer, S_PT_ROOM_LEAVE_SUCC_U &parameter)
{
	CStreamSP Stream;
	Stream->SetBuffer(buffer);


	return Stream->GetLength();
}

inline DWORD WRITE_PT_ROOM_LEAVE_FAIL_U(BYTE *buffer, S_PT_ROOM_LEAVE_FAIL_U &parameter)
{
	CStreamSP Stream;
	Stream->SetBuffer(buffer);

	Stream->WriteDWORD(parameter.ERROR_CODE);

	return Stream->GetLength();
}

inline DWORD WRITE_PT_ROOM_LEAVE_M(BYTE *buffer, S_PT_ROOM_LEAVE_M &parameter)
{
	CStreamSP Stream;
	Stream->SetBuffer(buffer);

	Stream->WriteDWORD_PTR(parameter.SESSION_ID);
	Stream->WriteDWORD_PTR(parameter.ROOT_USER_SESSION_ID);

	return Stream->GetLength();
}

inline DWORD WRITE_PT_ROOM_MAP_CHANGE(BYTE *buffer, S_PT_ROOM_MAP_CHANGE &parameter)
{
	CStreamSP Stream;
	Stream->SetBuffer(buffer);

	Stream->WriteDWORD(parameter.MAP_INDEX);

	return Stream->GetLength();
}

inline DWORD WRITE_PT_ROOM_MAP_SUCC_U(BYTE *buffer, S_PT_ROOM_MAP_SUCC_U &parameter)
{
	CStreamSP Stream;
	Stream->SetBuffer(buffer);


	return Stream->GetLength();
}

inline DWORD WRITE_PT_ROOM_MAP_FAIL_U(BYTE *buffer, S_PT_ROOM_MAP_FAIL_U &parameter)
{
	CStreamSP Stream;
	Stream->SetBuffer(buffer);

	Stream->WriteDWORD(parameter.ERROR_CODE);

	return Stream->GetLength();
}

inline DWORD WRITE_PT_ROOM_MAP_CHANGE_M(BYTE *buffer, S_PT_ROOM_MAP_CHANGE_M &parameter)
{
	CStreamSP Stream;
	Stream->SetBuffer(buffer);

	Stream->WriteDWORD(parameter.MAP_INDEX);

	return Stream->GetLength();
}

inline DWORD WRITE_PT_ROOM_CHATTING(BYTE *buffer, S_PT_ROOM_CHATTING &parameter)
{
	CStreamSP Stream;
	Stream->SetBuffer(buffer);

	Stream->WriteWCHARs(parameter.CHAT, 256);

	return Stream->GetLength();
}

inline DWORD WRITE_PT_ROOM_CHATTING_SUCC_U(BYTE *buffer, S_PT_ROOM_CHATTING_SUCC_U &parameter)
{
	CStreamSP Stream;
	Stream->SetBuffer(buffer);


	return Stream->GetLength();
}

inline DWORD WRITE_PT_ROOM_CHATTING_FAIL_U(BYTE *buffer, S_PT_ROOM_CHATTING_FAIL_U &parameter)
{
	CStreamSP Stream;
	Stream->SetBuffer(buffer);

	Stream->WriteDWORD(parameter.ERROR_CODE);

	return Stream->GetLength();
}

inline DWORD WRITE_PT_ROOM_CHATTING_M(BYTE *buffer, S_PT_ROOM_CHATTING_M &parameter)
{
	CStreamSP Stream;
	Stream->SetBuffer(buffer);

	Stream->WriteDWORD_PTR(parameter.SESSION_ID);
	Stream->WriteWCHARs(parameter.CHAT, 256);

	return Stream->GetLength();
}

inline DWORD WRITE_PT_ROOM_READY(BYTE *buffer, S_PT_ROOM_READY &parameter)
{
	CStreamSP Stream;
	Stream->SetBuffer(buffer);

	Stream->WriteBOOL(parameter.READY);

	return Stream->GetLength();
}

inline DWORD WRITE_PT_ROOM_READY_SUCC_U(BYTE *buffer, S_PT_ROOM_READY_SUCC_U &parameter)
{
	CStreamSP Stream;
	Stream->SetBuffer(buffer);


	return Stream->GetLength();
}

inline DWORD WRITE_PT_ROOM_READY_FAIL_U(BYTE *buffer, S_PT_ROOM_READY_FAIL_U &parameter)
{
	CStreamSP Stream;
	Stream->SetBuffer(buffer);

	Stream->WriteDWORD(parameter.ERROR_CODE);

	return Stream->GetLength();
}

inline DWORD WRITE_PT_ROOM_READY_M(BYTE *buffer, S_PT_ROOM_READY_M &parameter)
{
	CStreamSP Stream;
	Stream->SetBuffer(buffer);

	Stream->WriteDWORD_PTR(parameter.SESSION_ID);
	Stream->WriteBOOL(parameter.READY);

	return Stream->GetLength();
}

inline DWORD WRITE_PT_ROOM_START(BYTE *buffer, S_PT_ROOM_START &parameter)
{
	CStreamSP Stream;
	Stream->SetBuffer(buffer);


	return Stream->GetLength();
}

inline DWORD WRITE_PT_ROOM_START_SUCC_U(BYTE *buffer, S_PT_ROOM_START_SUCC_U &parameter)
{
	CStreamSP Stream;
	Stream->SetBuffer(buffer);


	return Stream->GetLength();
}

inline DWORD WRITE_PT_ROOM_START_FAIL_U(BYTE *buffer, S_PT_ROOM_START_FAIL_U &parameter)
{
	CStreamSP Stream;
	Stream->SetBuffer(buffer);

	Stream->WriteDWORD(parameter.ERROR_CODE);

	return Stream->GetLength();
}

inline DWORD WRITE_PT_ROOM_START_M(BYTE *buffer, S_PT_ROOM_START_M &parameter)
{
	CStreamSP Stream;
	Stream->SetBuffer(buffer);


	return Stream->GetLength();
}

inline DWORD WRITE_PT_GAME_LOAD_COMPLETE(BYTE *buffer, S_PT_GAME_LOAD_COMPLETE &parameter)
{
	CStreamSP Stream;
	Stream->SetBuffer(buffer);


	return Stream->GetLength();
}

inline DWORD WRITE_PT_GAME_LOAD_COMPLETE_SUCC_U(BYTE *buffer, S_PT_GAME_LOAD_COMPLETE_SUCC_U &parameter)
{
	CStreamSP Stream;
	Stream->SetBuffer(buffer);


	return Stream->GetLength();
}

inline DWORD WRITE_PT_GAME_LOAD_COMPLETE_FAIL_U(BYTE *buffer, S_PT_GAME_LOAD_COMPLETE_FAIL_U &parameter)
{
	CStreamSP Stream;
	Stream->SetBuffer(buffer);

	Stream->WriteDWORD(parameter.ERROR_CODE);

	return Stream->GetLength();
}

inline DWORD WRITE_PT_GAME_LOAD_COMPLETE_M(BYTE *buffer, S_PT_GAME_LOAD_COMPLETE_M &parameter)
{
	CStreamSP Stream;
	Stream->SetBuffer(buffer);

	Stream->WriteDWORD_PTR(parameter.SESSION_ID);

	return Stream->GetLength();
}

inline DWORD WRITE_PT_GAME_ALL_LOAD_COMPLETE_M(BYTE *buffer, S_PT_GAME_ALL_LOAD_COMPLETE_M &parameter)
{
	CStreamSP Stream;
	Stream->SetBuffer(buffer);


	return Stream->GetLength();
}

inline DWORD WRITE_PT_GAME_INTRO_COMPLETE(BYTE *buffer, S_PT_GAME_INTRO_COMPLETE &parameter)
{
	CStreamSP Stream;
	Stream->SetBuffer(buffer);


	return Stream->GetLength();
}

inline DWORD WRITE_PT_GAME_INTRO_COMPLETE_SUCC_U(BYTE *buffer, S_PT_GAME_INTRO_COMPLETE_SUCC_U &parameter)
{
	CStreamSP Stream;
	Stream->SetBuffer(buffer);


	return Stream->GetLength();
}

inline DWORD WRITE_PT_GAME_INTRO_COMPLETE_FAIL_U(BYTE *buffer, S_PT_GAME_INTRO_COMPLETE_FAIL_U &parameter)
{
	CStreamSP Stream;
	Stream->SetBuffer(buffer);

	Stream->WriteDWORD(parameter.ERROR_CODE);

	return Stream->GetLength();
}

inline DWORD WRITE_PT_GAME_INTRO_COMPLETE_M(BYTE *buffer, S_PT_GAME_INTRO_COMPLETE_M &parameter)
{
	CStreamSP Stream;
	Stream->SetBuffer(buffer);

	Stream->WriteDWORD_PTR(parameter.SESSION_ID);

	return Stream->GetLength();
}

inline DWORD WRITE_PT_GAME_ALL_INTRO_COMPLETE_M(BYTE *buffer, S_PT_GAME_ALL_INTRO_COMPLETE_M &parameter)
{
	CStreamSP Stream;
	Stream->SetBuffer(buffer);


	return Stream->GetLength();
}

inline DWORD WRITE_PT_GAME_START_M(BYTE *buffer, S_PT_GAME_START_M &parameter)
{
	CStreamSP Stream;
	Stream->SetBuffer(buffer);


	return Stream->GetLength();
}

inline DWORD WRITE_PT_GAME_END_M(BYTE *buffer, S_PT_GAME_END_M &parameter)
{
	CStreamSP Stream;
	Stream->SetBuffer(buffer);


	return Stream->GetLength();
}

inline DWORD WRITE_PT_CHANNEL_ENTER(BYTE *buffer, WCHAR *user_id, WCHAR *virtual_address, USHORT virtual_port)
{
	CStreamSP Stream;
	Stream->SetBuffer(buffer);

	WCHAR _user_id[32] = {0,};
	_tcsncpy(_user_id, user_id, 32);
	Stream->WriteWCHARs(_user_id, 32);
	WCHAR _virtual_address[32] = {0,};
	_tcsncpy(_virtual_address, virtual_address, 32);
	Stream->WriteWCHARs(_virtual_address, 32);
	Stream->WriteUSHORT(virtual_port);

	return Stream->GetLength();
}

inline DWORD WRITE_PT_CHANNEL_ENTER_SUCC_U(BYTE *buffer, DOWRD_PTR session_id, WCHAR *virtual_address, USHORT virtual_port, WCHAR *real_address, USHORT real_port)
{
	CStreamSP Stream;
	Stream->SetBuffer(buffer);

	Stream->WriteBytes((BYTE*) session_id, sizeof(DOWRD_PTR) * 0);
	WCHAR _virtual_address[32] = {0,};
	_tcsncpy(_virtual_address, virtual_address, 32);
	Stream->WriteWCHARs(_virtual_address, 32);
	Stream->WriteUSHORT(virtual_port);
	WCHAR _real_address[32] = {0,};
	_tcsncpy(_real_address, real_address, 32);
	Stream->WriteWCHARs(_real_address, 32);
	Stream->WriteUSHORT(real_port);

	return Stream->GetLength();
}

inline DWORD WRITE_PT_CHANNEL_ENTER_FAIL_U(BYTE *buffer, DWORD errror_code)
{
	CStreamSP Stream;
	Stream->SetBuffer(buffer);

	Stream->WriteDWORD(errror_code);

	return Stream->GetLength();
}

inline DWORD WRITE_PT_ROOM_ENTER_M(BYTE *buffer, DWORD slot_position, WCHAR *user_id, DWORD_PTR session_id, DWORD *virtual_address, USHORT virtual_port, WCHAR *real_address, USHORT real_port)
{
	CStreamSP Stream;
	Stream->SetBuffer(buffer);

	Stream->WriteDWORD(slot_position);
	WCHAR _user_id[32] = {0,};
	_tcsncpy(_user_id, user_id, 32);
	Stream->WriteWCHARs(_user_id, 32);
	Stream->WriteDWORD_PTR(session_id);
	Stream->WriteDWORD(virtual_address);
	Stream->WriteUSHORT(virtual_port);
	WCHAR _real_address[32] = {0,};
	_tcsncpy(_real_address, real_address, 32);
	Stream->WriteWCHARs(_real_address, 32);
	Stream->WriteUSHORT(real_port);

	return Stream->GetLength();
}

inline DWORD WRITE_PT_ROOM_REQ_INFO(BYTE *buffer)
{
	CStreamSP Stream;
	Stream->SetBuffer(buffer);


	return Stream->GetLength();
}

inline DWORD WRITE_PT_ROOM_REQ_INFO_SUCC_U(BYTE *buffer, DWORD room_index, WCHAR *title, DWORD map_index, USHORT current_user_count, DWORD_PTR root_user_session_id, SLOT_USER_DATA *data)
{
	CStreamSP Stream;
	Stream->SetBuffer(buffer);

	Stream->WriteDWORD(room_index);
	WCHAR _title[32] = {0,};
	_tcsncpy(_title, title, 32);
	Stream->WriteWCHARs(_title, 32);
	Stream->WriteDWORD(map_index);
	Stream->WriteUSHORT(current_user_count);
	Stream->WriteDWORD_PTR(root_user_session_id);
	Stream->WriteBytes((BYTE*) data, sizeof(SLOT_USER_DATA) * 8);

	return Stream->GetLength();
}

inline DWORD WRITE_PT_ROOM_REQ_INFO_FAIL_U(BYTE *buffer, DWORD error_code)
{
	CStreamSP Stream;
	Stream->SetBuffer(buffer);

	Stream->WriteDWORD(error_code);

	return Stream->GetLength();
}

inline DWORD WRITE_PT_ROOM_LEAVE(BYTE *buffer)
{
	CStreamSP Stream;
	Stream->SetBuffer(buffer);


	return Stream->GetLength();
}

inline DWORD WRITE_PT_ROOM_LEAVE_SUCC_U(BYTE *buffer)
{
	CStreamSP Stream;
	Stream->SetBuffer(buffer);


	return Stream->GetLength();
}

inline DWORD WRITE_PT_ROOM_LEAVE_FAIL_U(BYTE *buffer, DWORD error_code)
{
	CStreamSP Stream;
	Stream->SetBuffer(buffer);

	Stream->WriteDWORD(error_code);

	return Stream->GetLength();
}

inline DWORD WRITE_PT_ROOM_LEAVE_M(BYTE *buffer, DWORD_PTR session_id, DWORD_PTR root_user_session_id)
{
	CStreamSP Stream;
	Stream->SetBuffer(buffer);

	Stream->WriteDWORD_PTR(session_id);
	Stream->WriteDWORD_PTR(root_user_session_id);

	return Stream->GetLength();
}

inline DWORD WRITE_PT_ROOM_MAP_CHANGE(BYTE *buffer, DWORD map_index)
{
	CStreamSP Stream;
	Stream->SetBuffer(buffer);

	Stream->WriteDWORD(map_index);

	return Stream->GetLength();
}

inline DWORD WRITE_PT_ROOM_MAP_SUCC_U(BYTE *buffer)
{
	CStreamSP Stream;
	Stream->SetBuffer(buffer);


	return Stream->GetLength();
}

inline DWORD WRITE_PT_ROOM_MAP_FAIL_U(BYTE *buffer, DWORD error_code)
{
	CStreamSP Stream;
	Stream->SetBuffer(buffer);

	Stream->WriteDWORD(error_code);

	return Stream->GetLength();
}

inline DWORD WRITE_PT_ROOM_MAP_CHANGE_M(BYTE *buffer, DWORD map_index)
{
	CStreamSP Stream;
	Stream->SetBuffer(buffer);

	Stream->WriteDWORD(map_index);

	return Stream->GetLength();
}

inline DWORD WRITE_PT_ROOM_CHATTING(BYTE *buffer, WCHAR *chat)
{
	CStreamSP Stream;
	Stream->SetBuffer(buffer);

	WCHAR _chat[256] = {0,};
	_tcsncpy(_chat, chat, 256);
	Stream->WriteWCHARs(_chat, 256);

	return Stream->GetLength();
}

inline DWORD WRITE_PT_ROOM_CHATTING_SUCC_U(BYTE *buffer)
{
	CStreamSP Stream;
	Stream->SetBuffer(buffer);


	return Stream->GetLength();
}

inline DWORD WRITE_PT_ROOM_CHATTING_FAIL_U(BYTE *buffer, DWORD error_code)
{
	CStreamSP Stream;
	Stream->SetBuffer(buffer);

	Stream->WriteDWORD(error_code);

	return Stream->GetLength();
}

inline DWORD WRITE_PT_ROOM_CHATTING_M(BYTE *buffer, DWORD_PTR session_id, WCHAR *chat)
{
	CStreamSP Stream;
	Stream->SetBuffer(buffer);

	Stream->WriteDWORD_PTR(session_id);
	WCHAR _chat[256] = {0,};
	_tcsncpy(_chat, chat, 256);
	Stream->WriteWCHARs(_chat, 256);

	return Stream->GetLength();
}

inline DWORD WRITE_PT_ROOM_READY(BYTE *buffer, BOOL ready)
{
	CStreamSP Stream;
	Stream->SetBuffer(buffer);

	Stream->WriteBOOL(ready);

	return Stream->GetLength();
}

inline DWORD WRITE_PT_ROOM_READY_SUCC_U(BYTE *buffer)
{
	CStreamSP Stream;
	Stream->SetBuffer(buffer);


	return Stream->GetLength();
}

inline DWORD WRITE_PT_ROOM_READY_FAIL_U(BYTE *buffer, DWORD error_code)
{
	CStreamSP Stream;
	Stream->SetBuffer(buffer);

	Stream->WriteDWORD(error_code);

	return Stream->GetLength();
}

inline DWORD WRITE_PT_ROOM_READY_M(BYTE *buffer, DWORD_PTR session_id, BOOL ready)
{
	CStreamSP Stream;
	Stream->SetBuffer(buffer);

	Stream->WriteDWORD_PTR(session_id);
	Stream->WriteBOOL(ready);

	return Stream->GetLength();
}

inline DWORD WRITE_PT_ROOM_START(BYTE *buffer)
{
	CStreamSP Stream;
	Stream->SetBuffer(buffer);


	return Stream->GetLength();
}

inline DWORD WRITE_PT_ROOM_START_SUCC_U(BYTE *buffer)
{
	CStreamSP Stream;
	Stream->SetBuffer(buffer);


	return Stream->GetLength();
}

inline DWORD WRITE_PT_ROOM_START_FAIL_U(BYTE *buffer, DWORD error_code)
{
	CStreamSP Stream;
	Stream->SetBuffer(buffer);

	Stream->WriteDWORD(error_code);

	return Stream->GetLength();
}

inline DWORD WRITE_PT_ROOM_START_M(BYTE *buffer)
{
	CStreamSP Stream;
	Stream->SetBuffer(buffer);


	return Stream->GetLength();
}

inline DWORD WRITE_PT_GAME_LOAD_COMPLETE(BYTE *buffer)
{
	CStreamSP Stream;
	Stream->SetBuffer(buffer);


	return Stream->GetLength();
}

inline DWORD WRITE_PT_GAME_LOAD_COMPLETE_SUCC_U(BYTE *buffer)
{
	CStreamSP Stream;
	Stream->SetBuffer(buffer);


	return Stream->GetLength();
}

inline DWORD WRITE_PT_GAME_LOAD_COMPLETE_FAIL_U(BYTE *buffer, DWORD error_code)
{
	CStreamSP Stream;
	Stream->SetBuffer(buffer);

	Stream->WriteDWORD(error_code);

	return Stream->GetLength();
}

inline DWORD WRITE_PT_GAME_LOAD_COMPLETE_M(BYTE *buffer, DWORD_PTR session_id)
{
	CStreamSP Stream;
	Stream->SetBuffer(buffer);

	Stream->WriteDWORD_PTR(session_id);

	return Stream->GetLength();
}

inline DWORD WRITE_PT_GAME_ALL_LOAD_COMPLETE_M(BYTE *buffer)
{
	CStreamSP Stream;
	Stream->SetBuffer(buffer);


	return Stream->GetLength();
}

inline DWORD WRITE_PT_GAME_INTRO_COMPLETE(BYTE *buffer)
{
	CStreamSP Stream;
	Stream->SetBuffer(buffer);


	return Stream->GetLength();
}

inline DWORD WRITE_PT_GAME_INTRO_COMPLETE_SUCC_U(BYTE *buffer)
{
	CStreamSP Stream;
	Stream->SetBuffer(buffer);


	return Stream->GetLength();
}

inline DWORD WRITE_PT_GAME_INTRO_COMPLETE_FAIL_U(BYTE *buffer, DWORD error_code)
{
	CStreamSP Stream;
	Stream->SetBuffer(buffer);

	Stream->WriteDWORD(error_code);

	return Stream->GetLength();
}

inline DWORD WRITE_PT_GAME_INTRO_COMPLETE_M(BYTE *buffer, DWORD_PTR session_id)
{
	CStreamSP Stream;
	Stream->SetBuffer(buffer);

	Stream->WriteDWORD_PTR(session_id);

	return Stream->GetLength();
}

inline DWORD WRITE_PT_GAME_ALL_INTRO_COMPLETE_M(BYTE *buffer)
{
	CStreamSP Stream;
	Stream->SetBuffer(buffer);


	return Stream->GetLength();
}

inline DWORD WRITE_PT_GAME_START_M(BYTE *buffer)
{
	CStreamSP Stream;
	Stream->SetBuffer(buffer);


	return Stream->GetLength();
}

inline DWORD WRITE_PT_GAME_END_M(BYTE *buffer)
{
	CStreamSP Stream;
	Stream->SetBuffer(buffer);


	return Stream->GetLength();
}

