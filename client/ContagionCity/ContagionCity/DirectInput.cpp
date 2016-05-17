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

	// ���콺 Ŀ�� ��ġ�� ���� ��ũ�� ũ�� ����
	m_screenWidth = screenWidth;
	m_screenHeight = screenHeight;

	// DirectInput8 ����
	if (FAILED( result = DirectInput8Create( hInstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&m_directInput, NULL ) ))
		return false;

	// Ű����
	// Ű���带 �Է¹ޱ� ���� �������̽� ����̽� ����
	if (FAILED( result = m_directInput->CreateDevice( GUID_SysKeyboard, &m_keyboard, NULL ) ))
		return false;
	// ���� ���� In this case since it is a keyboard we can use the predefined data format.
	if (FAILED( result = m_keyboard->SetDataFormat( &c_dfDIKeyboard ) ))
		return false;
	// ���·��� ����
	// DISCL_NONEXCLUSIVE�� ����ϸ� �ٸ� ���α׷����� �� ��ǲ�ý����� ����� �� ����,( ����Ʈ��ũ��Ű ��� ����), ��Ŀ���� ��� Ȯ�����־����
	// DISCL_EXCLUSIVE�� ����ϸ� �ٸ� ���α׷����� �� ��ǲ�ý����� ����� �� ����, (����Ʈ��ũ��Ű�� ������ ��üȭ�鿡�� ����)
	if (FAILED( result = m_keyboard->SetCooperativeLevel( hWnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE ) ))
		return false;
	// Ű���忡 ���� ������ �޾ƿ�
	if (FAILED( result = m_keyboard->Acquire( ) ))
		return false;

	// ���콺
	// ���콺�� �Է¹ޱ� ���� �������̽� ����̽� ����
	if (FAILED( result = m_directInput->CreateDevice( GUID_SysMouse, &m_mouse, NULL ) ))
		return false;
	// ���� ���� for the mouse using the pre-defined mouse data format.
	if (FAILED( result = m_mouse->SetDataFormat( &c_dfDIMouse ) ))
		return false;
	// ���·��� ����
	// ���콺�� ���·����� DISCL_NONEXCLUSIVE�̸� ��Ŀ���� �׻� �˻��ؾ��Ѵ�.
	if (FAILED( result = m_mouse->SetCooperativeLevel( hWnd, DISCL_FOREGROUND | DISCL_EXCLUSIVE ) ))
		return false;
	// ���콺�� ���� ������ �޾ƿ�
	if (FAILED( result = m_mouse->Acquire( ) ))
		return false;
}

void DirectInput::Release( )
{
	// ����̽��� ����
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

// �� ��ġ�� ���� ���¸� �о� ���ۿ� ����
bool DirectInput::Frame( )
{
	bool result;

	// ���� Ű���� ���¸� ����
	if (result = ReadKeyboard( ))
		return false;

	// ���� ���콺 ���¸� ����
	if (result = ReadMouse( ))
		return false;

	return true;
}

// Ư�� Ű�� ���ȴ��� �˻��ϴ� �Լ�
bool DirectInput::IsEscapePressed( int KeyValue )
{
	if (m_keyboardState[KeyValue] & 0x80)
	{
		return true;
	}

	return false;
}

// ���� ���콺 ��ġ�� ��ȯ�ϴ� �Լ�
void DirectInput::GetMouseLocation( int& x, int& y )
{
	x = m_mouseX;
	y = m_mouseY;
	return;
}

// Ű���� ���¸� �о� ���ۿ� �����ϴ� �Լ�, �� �Լ��� ��� Ű�� ���ؼ� �˻��Ѵ�.
bool DirectInput::ReadKeyboard( )
{
	HRESULT result;


	// Ű���� �б�
	result = m_keyboard->GetDeviceState( sizeof( m_keyboardState ), (LPVOID)&m_keyboardState );
	if (FAILED( result ))
	{
		// Ű���尡 ��Ŀ���� �о��ų� ���� ���� ����� �ȵǾ����� ��
		if (( result == DIERR_INPUTLOST ) || ( result == DIERR_NOTACQUIRED ))	
			m_keyboard->Acquire( );
		// �ٸ� ���� �׳� ����
		else
			return false;
	}

	return true;
}

// ���콺 ���¸� �о� ���ۿ� �����ϴ� �Լ�, �� �Լ��� ���� ��ġ���� �ֱ� ��ġ�� �̵��� ������ �����Ѵ�.
// ���� ������ ��ġ�� ���� ã�ƾ��Ѵ�.
bool DirectInput::ReadMouse( )
{
	HRESULT result;

	// ���콺 �б�
	result = m_mouse->GetDeviceState( sizeof( DIMOUSESTATE ), (LPVOID)&m_mouseState );
	if (FAILED( result ))
	{
		// ���콺�� ��Ŀ���� �о��ų� ���� ���� ����� �ȵǾ����� ��
		if (( result == DIERR_INPUTLOST ) || ( result == DIERR_NOTACQUIRED ))
			m_mouse->Acquire( );
		else
			return false;
	}

	// ���콺 ��ȭ�� ������ �����Ͽ� ���� ���콺 ��ġ�� ����
	m_mouseX += m_mouseState.lX;
	m_mouseY += m_mouseState.lY;

	// ���콺�� ��ǥ�� ȭ�� ������ ������ �ʵ��� ��
	if (m_mouseX < 0)  { m_mouseX = 0; }
	if (m_mouseY < 0)  { m_mouseY = 0; }

	if (m_mouseX > m_screenWidth)  { m_mouseX = m_screenWidth; }
	if (m_mouseY > m_screenHeight) { m_mouseY = m_screenHeight; }

	return true;
}