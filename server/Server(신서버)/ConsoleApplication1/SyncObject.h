#pragma once
class SyncObject
{
public:
	CRITICAL_SECTION cs;

	SyncObject& operator=(const SyncObject& other);

public:
	SyncObject(const SyncObject& other)
	{
		InitializeCriticalSection(&cs);
	}

	SyncObject();

	void enter();

	void leave();

	virtual ~SyncObject();
};


class Lock
{
	SyncObject* syncObj;
	Lock& operator=(const Lock& other);
	Lock(const Lock& other);

public:
	Lock(SyncObject* syncObj) : syncObj(syncObj) { syncObj->enter(); }

	virtual ~Lock() { syncObj->leave(); }
};