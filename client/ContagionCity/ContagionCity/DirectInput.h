#pragma once

#define DIRECTINPUT_VERSION 0x0800
#pragma comment(lib, "dinput8.lib")

#include <dinput.h>

class DirectInput
{
	// direct input 8 ����̽���
	IDirectInput8* m_directInput;
	IDirectInputDevice8* m_keyboard;
	IDirectInputDevice8* m_mouse;

	int m_screenWidth, m_screenHeight;
	
	// ���콺�� ����
	DIMOUSESTATE m_mouseState;
	// Ű���� ���콺 ������ġ
	unsigned char m_keyboardState[256];
	int m_mouseX, m_mouseY;

	DirectInput( );
public:
	static DirectInput* instance;
	static DirectInput* GetInstance( ) { if (instance == NULL) instance = new DirectInput; return instance; }

	~DirectInput( );

	bool Init( HINSTANCE hInstance, HWND hWnd, int screenWidth, int screenHeight );
	void Release( );
	bool Frame( );

	bool IsEscapePressed( int KeyValue );
	void GetMouseLocation( int& x, int& y );	

private:
	bool ReadKeyboard( );
	bool ReadMouse( );
	void ProcessInput( );
};

