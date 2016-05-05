#include "stdafx.h"
#include "../CoreLib/MiniDump.h"

class CDataMP{
public:
	BOOL mTest;
};
int main(void){

	CMiniDump::Begin();
	
	CDataMP *pData = new CDataMP();
	pData = NULL;
	pData->mTest = false;

	CMiniDump::End();
	return 0;
}