#pragma once

#include "Protocol.h"

struct OVERLAPPED_EX{
	WSAOVERLAPPED overlapped;
	bool is_send;
	bool Login_Permision;
	WSABUF wsabuf;
	short command;
	char packetBuf[MAX_PACKET_SIZE];	// Sned�ÿ��� ���. Recv ������� ���
};