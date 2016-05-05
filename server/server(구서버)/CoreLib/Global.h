#pragma once

#include <winsock2.h>
#include <ws2tcpip.h>
#include <mswsock.h>
#include <windows.h>
#include <dbghelp.h>
#include <assert.h>
#include <stdio.h>
#include <tchar.h>
#include <time.h>
#include <DbgHelp.h>
#include <vector>
#include <map>

/*
����� Ÿ���� ����
OVERLAPPED_EX ���� ����Ѵ�.
*/
enum IO_TYPE
{
	IO_ACCEPT,
	IO_READ,
	IO_WRITE
};

/*
������ Overlapped�� ������ ��
*/
typedef struct _OVERLAPPED_EX
{
	OVERLAPPED	Overlapped;
	IO_TYPE		IoType;
	VOID		*Object;
} OVERLAPPED_EX;




#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#define MAX_QUEUE_LENGTH 500
#define MAX_BUFFER_LENGTH	4096
#pragma warning(disable:4996)

