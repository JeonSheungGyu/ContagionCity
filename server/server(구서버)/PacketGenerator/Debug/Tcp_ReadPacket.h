#pragma once

inline VOID READ_PT_CHANNEL_ENTER(BYTE *buffer, S_PT_CHANNEL_ENTER &parameter)
{
	CStreamSP Stream;
	Stream->SetBuffer(buffer);

	Stream->ReadWCHARs(parameter.USER_ID, 32);
	Stream->ReadWCHARs(parameter.VIRTUAL_ADDRESS, 32);
	Stream->ReadUSHORT(&parameter.VIRTUAL_PORT);
}

inline VOID READ_PT_CHANNEL_ENTER_SUCC_U(BYTE *buffer, S_PT_CHANNEL_ENTER_SUCC_U &parameter)
{
	CStreamSP Stream;
	Stream->SetBuffer(buffer);

	Stream->ReadBytes((BYTE*) parameter.SESSION_ID, sizeof(DOWRD_PTR) * 0);
	Stream->ReadWCHARs(parameter.VIRTUAL_ADDRESS, 32);
	Stream->ReadUSHORT(&parameter.VIRTUAL_PORT);
	Stream->ReadWCHARs(parameter.REAL_ADDRESS, 32);
	Stream->ReadUSHORT(&parameter.REAL_PORT);
}

inline VOID READ_PT_CHANNEL_ENTER_FAIL_U(BYTE *buffer, S_PT_CHANNEL_ENTER_FAIL_U &parameter)
{
	CStreamSP Stream;
	Stream->SetBuffer(buffer);

	Stream->ReadDWORD(&parameter.ERRROR_CODE);
}

inline VOID READ_PT_ROOM_ENTER_M(BYTE *buffer, S_PT_ROOM_ENTER_M &parameter)
{
	CStreamSP Stream;
	Stream->SetBuffer(buffer);

	Stream->ReadDWORD(&parameter.SLOT_POSITION);
	Stream->ReadWCHARs(parameter.USER_ID, 32);
	Stream->ReadDWORD_PTR(&parameter.SESSION_ID);
	Stream->ReadDWORD(&parameter.VIRTUAL_ADDRESS);
	Stream->ReadUSHORT(&parameter.VIRTUAL_PORT);
	Stream->ReadWCHARs(parameter.REAL_ADDRESS, 32);
	Stream->ReadUSHORT(&parameter.REAL_PORT);
}

inline VOID READ_PT_ROOM_REQ_INFO(BYTE *buffer, S_PT_ROOM_REQ_INFO &parameter)
{
	CStreamSP Stream;
	Stream->SetBuffer(buffer);

}

inline VOID READ_PT_ROOM_REQ_INFO_SUCC_U(BYTE *buffer, S_PT_ROOM_REQ_INFO_SUCC_U &parameter)
{
	CStreamSP Stream;
	Stream->SetBuffer(buffer);

	Stream->ReadDWORD(&parameter.ROOM_INDEX);
	Stream->ReadWCHARs(parameter.TITLE, 32);
	Stream->ReadDWORD(&parameter.MAP_INDEX);
	Stream->ReadUSHORT(&parameter.CURRENT_USER_COUNT);
	Stream->ReadDWORD_PTR(&parameter.ROOT_USER_SESSION_ID);
	Stream->ReadBytes((BYTE*) parameter.DATA, sizeof(SLOT_USER_DATA) * 8);
}

inline VOID READ_PT_ROOM_REQ_INFO_FAIL_U(BYTE *buffer, S_PT_ROOM_REQ_INFO_FAIL_U &parameter)
{
	CStreamSP Stream;
	Stream->SetBuffer(buffer);

	Stream->ReadDWORD(&parameter.ERROR_CODE);
}

inline VOID READ_PT_ROOM_LEAVE(BYTE *buffer, S_PT_ROOM_LEAVE &parameter)
{
	CStreamSP Stream;
	Stream->SetBuffer(buffer);

}

inline VOID READ_PT_ROOM_LEAVE_SUCC_U(BYTE *buffer, S_PT_ROOM_LEAVE_SUCC_U &parameter)
{
	CStreamSP Stream;
	Stream->SetBuffer(buffer);

}

inline VOID READ_PT_ROOM_LEAVE_FAIL_U(BYTE *buffer, S_PT_ROOM_LEAVE_FAIL_U &parameter)
{
	CStreamSP Stream;
	Stream->SetBuffer(buffer);

	Stream->ReadDWORD(&parameter.ERROR_CODE);
}

inline VOID READ_PT_ROOM_LEAVE_M(BYTE *buffer, S_PT_ROOM_LEAVE_M &parameter)
{
	CStreamSP Stream;
	Stream->SetBuffer(buffer);

	Stream->ReadDWORD_PTR(&parameter.SESSION_ID);
	Stream->ReadDWORD_PTR(&parameter.ROOT_USER_SESSION_ID);
}

inline VOID READ_PT_ROOM_MAP_CHANGE(BYTE *buffer, S_PT_ROOM_MAP_CHANGE &parameter)
{
	CStreamSP Stream;
	Stream->SetBuffer(buffer);

	Stream->ReadDWORD(&parameter.MAP_INDEX);
}

inline VOID READ_PT_ROOM_MAP_SUCC_U(BYTE *buffer, S_PT_ROOM_MAP_SUCC_U &parameter)
{
	CStreamSP Stream;
	Stream->SetBuffer(buffer);

}

inline VOID READ_PT_ROOM_MAP_FAIL_U(BYTE *buffer, S_PT_ROOM_MAP_FAIL_U &parameter)
{
	CStreamSP Stream;
	Stream->SetBuffer(buffer);

	Stream->ReadDWORD(&parameter.ERROR_CODE);
}

inline VOID READ_PT_ROOM_MAP_CHANGE_M(BYTE *buffer, S_PT_ROOM_MAP_CHANGE_M &parameter)
{
	CStreamSP Stream;
	Stream->SetBuffer(buffer);

	Stream->ReadDWORD(&parameter.MAP_INDEX);
}

inline VOID READ_PT_ROOM_CHATTING(BYTE *buffer, S_PT_ROOM_CHATTING &parameter)
{
	CStreamSP Stream;
	Stream->SetBuffer(buffer);

	Stream->ReadWCHARs(parameter.CHAT, 256);
}

inline VOID READ_PT_ROOM_CHATTING_SUCC_U(BYTE *buffer, S_PT_ROOM_CHATTING_SUCC_U &parameter)
{
	CStreamSP Stream;
	Stream->SetBuffer(buffer);

}

inline VOID READ_PT_ROOM_CHATTING_FAIL_U(BYTE *buffer, S_PT_ROOM_CHATTING_FAIL_U &parameter)
{
	CStreamSP Stream;
	Stream->SetBuffer(buffer);

	Stream->ReadDWORD(&parameter.ERROR_CODE);
}

inline VOID READ_PT_ROOM_CHATTING_M(BYTE *buffer, S_PT_ROOM_CHATTING_M &parameter)
{
	CStreamSP Stream;
	Stream->SetBuffer(buffer);

	Stream->ReadDWORD_PTR(&parameter.SESSION_ID);
	Stream->ReadWCHARs(parameter.CHAT, 256);
}

inline VOID READ_PT_ROOM_READY(BYTE *buffer, S_PT_ROOM_READY &parameter)
{
	CStreamSP Stream;
	Stream->SetBuffer(buffer);

	Stream->ReadBOOL(&parameter.READY);
}

inline VOID READ_PT_ROOM_READY_SUCC_U(BYTE *buffer, S_PT_ROOM_READY_SUCC_U &parameter)
{
	CStreamSP Stream;
	Stream->SetBuffer(buffer);

}

inline VOID READ_PT_ROOM_READY_FAIL_U(BYTE *buffer, S_PT_ROOM_READY_FAIL_U &parameter)
{
	CStreamSP Stream;
	Stream->SetBuffer(buffer);

	Stream->ReadDWORD(&parameter.ERROR_CODE);
}

inline VOID READ_PT_ROOM_READY_M(BYTE *buffer, S_PT_ROOM_READY_M &parameter)
{
	CStreamSP Stream;
	Stream->SetBuffer(buffer);

	Stream->ReadDWORD_PTR(&parameter.SESSION_ID);
	Stream->ReadBOOL(&parameter.READY);
}

inline VOID READ_PT_ROOM_START(BYTE *buffer, S_PT_ROOM_START &parameter)
{
	CStreamSP Stream;
	Stream->SetBuffer(buffer);

}

inline VOID READ_PT_ROOM_START_SUCC_U(BYTE *buffer, S_PT_ROOM_START_SUCC_U &parameter)
{
	CStreamSP Stream;
	Stream->SetBuffer(buffer);

}

inline VOID READ_PT_ROOM_START_FAIL_U(BYTE *buffer, S_PT_ROOM_START_FAIL_U &parameter)
{
	CStreamSP Stream;
	Stream->SetBuffer(buffer);

	Stream->ReadDWORD(&parameter.ERROR_CODE);
}

inline VOID READ_PT_ROOM_START_M(BYTE *buffer, S_PT_ROOM_START_M &parameter)
{
	CStreamSP Stream;
	Stream->SetBuffer(buffer);

}

inline VOID READ_PT_GAME_LOAD_COMPLETE(BYTE *buffer, S_PT_GAME_LOAD_COMPLETE &parameter)
{
	CStreamSP Stream;
	Stream->SetBuffer(buffer);

}

inline VOID READ_PT_GAME_LOAD_COMPLETE_SUCC_U(BYTE *buffer, S_PT_GAME_LOAD_COMPLETE_SUCC_U &parameter)
{
	CStreamSP Stream;
	Stream->SetBuffer(buffer);

}

inline VOID READ_PT_GAME_LOAD_COMPLETE_FAIL_U(BYTE *buffer, S_PT_GAME_LOAD_COMPLETE_FAIL_U &parameter)
{
	CStreamSP Stream;
	Stream->SetBuffer(buffer);

	Stream->ReadDWORD(&parameter.ERROR_CODE);
}

inline VOID READ_PT_GAME_LOAD_COMPLETE_M(BYTE *buffer, S_PT_GAME_LOAD_COMPLETE_M &parameter)
{
	CStreamSP Stream;
	Stream->SetBuffer(buffer);

	Stream->ReadDWORD_PTR(&parameter.SESSION_ID);
}

inline VOID READ_PT_GAME_ALL_LOAD_COMPLETE_M(BYTE *buffer, S_PT_GAME_ALL_LOAD_COMPLETE_M &parameter)
{
	CStreamSP Stream;
	Stream->SetBuffer(buffer);

}

inline VOID READ_PT_GAME_INTRO_COMPLETE(BYTE *buffer, S_PT_GAME_INTRO_COMPLETE &parameter)
{
	CStreamSP Stream;
	Stream->SetBuffer(buffer);

}

inline VOID READ_PT_GAME_INTRO_COMPLETE_SUCC_U(BYTE *buffer, S_PT_GAME_INTRO_COMPLETE_SUCC_U &parameter)
{
	CStreamSP Stream;
	Stream->SetBuffer(buffer);

}

inline VOID READ_PT_GAME_INTRO_COMPLETE_FAIL_U(BYTE *buffer, S_PT_GAME_INTRO_COMPLETE_FAIL_U &parameter)
{
	CStreamSP Stream;
	Stream->SetBuffer(buffer);

	Stream->ReadDWORD(&parameter.ERROR_CODE);
}

inline VOID READ_PT_GAME_INTRO_COMPLETE_M(BYTE *buffer, S_PT_GAME_INTRO_COMPLETE_M &parameter)
{
	CStreamSP Stream;
	Stream->SetBuffer(buffer);

	Stream->ReadDWORD_PTR(&parameter.SESSION_ID);
}

inline VOID READ_PT_GAME_ALL_INTRO_COMPLETE_M(BYTE *buffer, S_PT_GAME_ALL_INTRO_COMPLETE_M &parameter)
{
	CStreamSP Stream;
	Stream->SetBuffer(buffer);

}

inline VOID READ_PT_GAME_START_M(BYTE *buffer, S_PT_GAME_START_M &parameter)
{
	CStreamSP Stream;
	Stream->SetBuffer(buffer);

}

inline VOID READ_PT_GAME_END_M(BYTE *buffer, S_PT_GAME_END_M &parameter)
{
	CStreamSP Stream;
	Stream->SetBuffer(buffer);

}

