#pragma once

template <class T, int ALLOC_BLOCK_SIZE = 50>
class CMemoryPool 
{
public:
	static VOID* operator new(size_t allocLength)
	{
		 

		assert(sizeof(T) == allocLength);
		assert(sizeof(T) >= sizeof(UCHAR*));

		//�Ҵ�
		//���� �޸𸮰� �� ���� mFreePointer�� NULL�̵ǰ� ���ο� �޸𸮸� �ٽ� �Ҵ��ϰ� �ȴ�.
		//static �޸𸮿� �޸� ������ �Ҵ緮��ŭ�� ����ϴ� ���� �ƴϴ�.
		if (!mFreePointer)
			allocBlock();

		UCHAR *ReturnPointer = mFreePointer;
		//���� ��ġ�� ������ �ٲ�
		mFreePointer = *reinterpret_cast<UCHAR**>(ReturnPointer);

		return ReturnPointer;
	}

	static VOID	operator delete(VOID* deletePointer)
	{
		
		//�����Ǵ� ������ ���� ������ �ּҸ� ���´�.
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
			//���� �ּҸ� �Է��Ѵ�.
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
