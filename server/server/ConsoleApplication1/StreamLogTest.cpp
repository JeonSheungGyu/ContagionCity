#include "stdafx.h"
#include "../CoreLib/Stream.h"
#include "../CoreLib/Log.h"

int main(void){

	
	//��Ʈ�� �׽�Ʈ
	/*CStreamSP wtStream;
	BYTE buffer[100];
	
	wtStream->SetBuffer(buffer);
	wtStream->WriteBOOL(true);

	
	CStreamSP rdStream;
	BOOL data;
	rdStream->SetBuffer(buffer);
	rdStream->ReadBOOL(&data);
	printf("%d\n", data);*/

	//�α� �׽�Ʈ
	CLog::WriteLog(_T("���Ͽ����߻�"));
	
	return 0;
}