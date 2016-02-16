#include "stdafx.h"
#include "../CoreLib/Stream.h"


int main(void){
	CStreamSP wtStream;
	BYTE buffer[100];
	
	wtStream->SetBuffer(buffer);
	wtStream->WriteBOOL(true);

	CStreamSP rdStream;
	BOOL data;
	rdStream->SetBuffer(buffer);
	rdStream->ReadBOOL(&data);
	printf("%d\n", data);
	return 0;
}