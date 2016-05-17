#include "stdafx.h"
#include "DirectInput.h"

DirectInput::DirectInput( )
{
	m_directInput = 0;
	m_keyboard = 0;
	m_mouse = 0;

	m_mouseX = 0;
	m_mouseY = 0;
}

DirectInput::~DirectInput( )
{
}

bool DirectInput::Init( HINSTANCE hInstance, HWND hWnd, int screenWidth, int screenHeight )
{
	HRESULT result;

	// 마우스 커서 위치를 위한 스크린 크기 저장
	m_screenWidth = screenWidth;
	m_screenHeight = screenHeight;

	// DirectInput8 생성
	if (FAILED( result = DirectInput8Create( hInstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&m_directInput, NULL ) ))
		return false;

	// 키보드
	// 키보드를 입력받기 위한 인터페이스 디바이스 생성
	if (FAILED( result = m_directInput->CreateDevice( GUID_SysKeyboard, &m_keyboard, NULL ) ))
		return false;
	// 포멧 저장 In this case since it is a keyboard we can use the predefined data format.
	if (FAILED( result = m_keyboard->SetDataFormat( &c_dfDIKeyboard ) ))
		return false;
	// 협력레벨 지정
	// DISCL_NONEXCLUSIVE를 사용하면 다른 프로그램에서 이 인풋시스템을 사용할 수 있음,( 프린트스크린키 사용 가능), 포커스를 계속 확인해주어야함
	// DISCL_EXCLUSIVE를 사용하면 다른 프로그램에서 이 인풋시스템을 사용할 수 없음, (프린트스크린키를 막으며 전체화면에서 유용)
	if (FAILED( result = m_keyboard->SetCooperativeLevel( hWnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE ) ))
		return false;
	// 키보드에 대한 접근을 받아옴
	if (FAILED( result = m_keyboard->Acquire( ) ))
		return false;

	// 마우스
	// 마우스를 입력받기 위한 인터페이스 디바이스 생성
	if (FAILED( result = m_directInput->CreateDevice( GUID_SysMouse, &m_mouse, NULL ) ))
		return false;
	// 포멧 저장 for the mouse using the pre-defined mouse data format.
	if (FAILED( result = m_mouse->SetDataFormat( &c_dfDIMouse ) ))
		return false;
	// 협력레벨 지정
	// 마우스의 협력레벨이 DISCL_NONEXCLUSIVE이면 포커스를 항상 검사해야한다.
	if (FAILED( result = m_mouse->SetCooperativeLevel( hWnd, DISCL_FOREGROUND | DISCL_EXCLUSIVE ) ))
		return false;
	// 마우스에 대한 접근을 받아옴
	if (FAILED( result = m_mouse->Acquire( ) ))
		return false;
}

void DirectInput::Release( )
{
	// 디바이스들 해제
	// Release the mouse.
	if (m_mouse)
	{
		m_mouse->Unacquire( );
		m_mouse->Release( );
		m_mouse = 0;
	}

	// Release the keyboard.
	if (m_keyboard)
	{
		m_keyboard->Unacquire( );
		m_keyboard->Release( );
		m_keyboard = 0;
	}

	// Release the main interface to direct input.
	if (m_directInput)
	{
		m_directInput->Release( );
		m_directInput = 0;
	}
}

// 각 장치의 현재 상태를 읽어 버퍼에 저장
bool DirectInput::Frame( )
{
	bool result;

	// 현재 키보드 상태를 읽음
	if (result = ReadKeyboard( ))
		return false;

	// 현재 마우스 상태를 읽음
	if (result = ReadMouse( ))
		return false;

	return true;
}

// 특정 키가 눌렸는지 검사하는 함수
bool DirectInput::IsEscapePressed( int KeyValue )
{
	if (m_keyboardState[KeyValue] & 0x80)
	{
		return true;
	}

	return false;
}

// 현재 마우스 위치를 반환하는 함수
void DirectInput::GetMouseLocation( int& x, int& y )
{
	x = m_mouseX;
	y = m_mouseY;
	return;
}

// 키보드 상태를 읽어 버퍼에 저장하는 함수, 이 함수는 모든 키에 대해서 검사한다.
bool DirectInput::ReadKeyboard( )
{
	HRESULT result;


	// 키보드 읽기
	result = m_keyboard->GetDeviceState( sizeof( m_keyboardState ), (LPVOID)&m_keyboardState );
	if (FAILED( result ))
	{
		// 키보드가 포커스를 읽었거나 접근 권한 취득이 안되어있을 때
		if (( result == DIERR_INPUTLOST ) || ( result == DIERR_NOTACQUIRED ))	
			m_keyboard->Acquire( );
		// 다른 경우는 그냥 실패
		else
			return false;
	}

	return true;
}

// 마우스 상태를 읽어 버퍼에 저장하는 함수, 이 함수는 이전 위치에서 최근 위치로 이동한 변위만 저장한다.
// 따라서 현재의 위치는 내가 찾아야한다.
bool DirectInput::ReadMouse( )
{
	HRESULT result;

	// 마우스 읽기
	result = m_mouse->GetDeviceState( sizeof( DIMOUSESTATE ), (LPVOID)&m_mouseState );
	if (FAILED( result ))
	{
		// 마우스가 포커스를 읽었거나 접근 권한 취득이 안되어있을 때
		if (( result == DIERR_INPUTLOST ) || ( result == DIERR_NOTACQUIRED ))
			m_mouse->Acquire( );
		else
			return false;
	}

	// 마우스 변화의 변위를 저장하여 현재 마우스 위치를 저장
	m_mouseX += m_mouseState.lX;
	m_mouseY += m_mouseState.lY;

	// 마우스의 좌표가 화면 밖으로 나가지 않도록 함
	if (m_mouseX < 0)  { m_mouseX = 0; }
	if (m_mouseY < 0)  { m_mouseY = 0; }

	if (m_mouseX > m_screenWidth)  { m_mouseX = m_screenWidth; }
	if (m_mouseY > m_screenHeight) { m_mouseY = m_screenHeight; }

	return true;
}