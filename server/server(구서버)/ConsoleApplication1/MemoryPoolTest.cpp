#include "stdafx.h"
#include "../CoreLib/MemoryPool.h"

class CDataMP : public CMemoryPool<CDataMP> {
private:
	BYTE a[1024];
	BYTE b[1024];
};
int main(void){
	clock_t before;
	double result;
	before = clock();

	for (INT i = 0; i < 10000000; i++){
		CDataMP *pData = new CDataMP();
		delete pData;
	}

	result = (double)(clock() - before) / CLOCKS_PER_SEC;

	printf("걸린시간은 %5.2f 입니다.\n", result);


	return 0;
}