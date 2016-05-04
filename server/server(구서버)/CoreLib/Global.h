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
입출력 타입을 정의
OVERLAPPED_EX 에서 사용한다.
*/
enum IO_TYPE
{
	IO_ACCEPT,
	IO_READ,
	IO_WRITE
};

/*
기존의 Overlapped를 가지고 있
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

