#include "stdafx.h"
#include "../CoreLib/Stream.h"
#include "../CoreLib/Log.h"

int main(void){

	
	//스트림 테스트
	/*CStreamSP wtStream;
	BYTE buffer[100];
	
	wtStream->SetBuffer(buffer);
	wtStream->WriteBOOL(true);

	
	CStreamSP rdStream;
	BOOL data;
	rdStream->SetBuffer(buffer);
	rdStream->ReadBOOL(&data);
	printf("%d\n", data);*/

	//로그 테스트
	CLog::WriteLog(_T("소켓오류발생"));
	
	return 0;
}