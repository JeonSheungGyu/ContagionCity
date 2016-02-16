#include<Windows.h>
#include<iostream>

using namespace std;

class CStream{
public:
	BYTE *mBufferPointer;
	DWORD mLength;
};

class CStreamSP{
public:
	CStreamSP(VOID){
		Stream = new CStream();
	}

	~CStreamSP(VOID){
		delete Stream;
	}
	
	CStream* operator ->(VOID){
		return Stream;
	}
	
	operator CStream*(VOID){
		return Stream;
	}

private:
	CStream *Stream;
};

int main(VOID){

	CStreamSP stream;
	cout << stream->mLength << endl;
	return 0;
}