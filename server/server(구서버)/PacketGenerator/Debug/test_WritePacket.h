#pragma once

inline DWORD WRITE_PT_CHANNEL_NICKNAME(BYTE *buffer, S_PT_CHANNEL_NICKNAME &parameter)
{
	CStreamSP Stream;
	Stream->SetBuffer(buffer);

	Stream->WriteWCHARs(parameter.USER_ID, 32);
	Stream->WriteWCHARs(parameter.NICK_NAME, 32);

	return Stream->GetLength();
}

inline DWORD WRITE_PT_CHANNEL_NICKNAME(BYTE *buffer, WCHAR *user_id, WCHAR *nick_name)
{
	CStreamSP Stream;
	Stream->SetBuffer(buffer);

	WCHAR _user_id[32] = {0,};
	_tcsncpy(_user_id, user_id, 32);
	Stream->WriteWCHARs(_user_id, 32);
	WCHAR _nick_name[32] = {0,};
	_tcsncpy(_nick_name, nick_name, 32);
	Stream->WriteWCHARs(_nick_name, 32);

	return Stream->GetLength();
}

