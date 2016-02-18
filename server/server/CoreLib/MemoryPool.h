#pragma once

template <class T, int ALLOC_BLOCK_SIZE = 50>
class CMemoryPool 
{
public:
	static VOID* operator new(size_t allocLength)
	{
		 

		assert(sizeof(T) == allocLength);
		assert(sizeof(T) >= sizeof(UCHAR*));

		//할당
		//만약 메모리가 꽉 차면 mFreePointer는 NULL이되고 새로운 메모리를 다시 할당하게 된다.
		//static 메모리와 달리 정해진 할당량만큼만 사용하는 것이 아니다.
		if (!mFreePointer)
			allocBlock();

		UCHAR *ReturnPointer = mFreePointer;
		//다음 위치로 포인터 바꿈
		mFreePointer = *reinterpret_cast<UCHAR**>(ReturnPointer);

		return ReturnPointer;
	}

	static VOID	operator delete(VOID* deletePointer)
	{
		
		//삭제되는 영역에 다음 영역을 주소를 적는다.
		*reinterpret_cast<UCHAR**>(deletePointer) = mFreePointer;
		mFreePointer = static_cast<UCHAR*>(deletePointer);
	}

private:
	static VOID	allocBlock()
	{
		mFreePointer = new UCHAR[sizeof(T) * ALLOC_BLOCK_SIZE];

		UCHAR **Current = reinterpret_cast<UCHAR **>(mFreePointer);
		UCHAR *Next = mFreePointer;

		for (INT i = 0; i<ALLOC_BLOCK_SIZE - 1; ++i)
		{
			Next += sizeof(T);
			//다음 주소를 입력한다.
			*Current = Next;
			Current = reinterpret_cast<UCHAR**>(Next);
		}

		*Current = 0;
	}

private:
	static UCHAR	*mFreePointer;

protected:
	~CMemoryPool()
	{
	}
};

template <class T, int ALLOC_BLOCK_SIZE>
UCHAR* CMemoryPool<T, ALLOC_BLOCK_SIZE>::mFreePointer;
