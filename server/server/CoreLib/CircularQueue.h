template<class T>

class CCircularQueue{

private:
	T mQueue[MAX_QUEUE_LENTH];
	DWORD mQueueHead;
	DWORD mQueueTail;

public:
	CCircularQueue(VOID){
		ZeroMemory(mQueue, sizeof(mQueue));
		mQueueHead = mQueueTail = 0;
	}
	~CCircularQueue(VOID){}

	BOOL Begin(VOID){
		ZeroMemory(mQueue, sizeof(mQueue));
		mQueueHead = mQueueTail = 0;
		return true;
	}
	BOOL End(VOID){ return true; }
	BOOL Push(T data){
		DWORD TempTail = (mQueueTail + 1) % MAX_QUEUE_LENTH;
		if (TempTail == mQueueHead)
			return FALSE;
		
		CopyMemory(&mQueue[TempTail], &data, sizeof(T));
		mQueueTail = TempTail;

		return TRUE;
	}

	BOOL Pop(T& data){
		if (mQueueHead == mQueueTail)
			return FALSE;

		DWORD TempHead = (mQueueHead + 1) % MAX_QUEUE_LENTH;

		CopyMemory(&data, &mQueue[TempHead], sizeof(T));

		mQueueHead = TempHead;

		return TRUE;
	}

	BOOL IsEmpty(VOID){
		if (mQueueHead == mQueueTail) return TRUE;
		return FALSE;
	}
};