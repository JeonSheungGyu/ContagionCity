#include "stdafx.h"
#include "SyncObject.h"

using namespace std;

SyncObject::SyncObject()
{
	InitializeCriticalSection(&cs);
}


void SyncObject::enter()
{
	EnterCriticalSection(&cs);
}


void SyncObject::leave()
{
	LeaveCriticalSection(&cs);
}


SyncObject::~SyncObject()
{
	DeleteCriticalSection(&cs);
}