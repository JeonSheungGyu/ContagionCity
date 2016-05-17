#pragma comment(lib, "ws2_32")
#include "stdafx.h"
#include "LoginScene.h"
#include "Protocol.h"
#include <winsock2.h>

#define BUFSIZE 1024

void ErrorHandling(char *msg)
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	MessageBox(NULL, (LPCTSTR)lpMsgBuf, (LPCWSTR)msg, MB_ICONERROR);
	LocalFree(lpMsgBuf);
//	exit(1);

}

// 사용자 정의 데이터 수신 함수
int recvn(SOCKET s, char *buf, int len, int flags) {
	int received;
	char *ptr = buf;
	int left = len;

	while (left > 0) {
		received = recv(s, ptr, left, flags);
		if (received == SOCKET_ERROR)
			return SOCKET_ERROR;
		else if (received == 0)
			break;
		left -= received;
		ptr += received;
	}

	return (len - left);
}

bool CLoginScene::LoginPermit() {
	WSADATA wsaData;
	SOCKADDR_IN recvAddr;
	SOCKET hSocket;
	char buff[BUFSIZE];

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) /* Load Winsock 2.2 DLL */
		ErrorHandling("WSAStartup() error!");

	hSocket = WSASocket(PF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (hSocket == INVALID_SOCKET)
		ErrorHandling("socket() error");

	const char *myIP = m_ip.data();
	memset(&recvAddr, 0, sizeof(recvAddr));
	recvAddr.sin_family = AF_INET;
	recvAddr.sin_addr.s_addr = inet_addr( myIP );
	recvAddr.sin_port = htons(LOGIN_PORT);

	if (connect(hSocket, (SOCKADDR*)&recvAddr, sizeof(recvAddr)) == SOCKET_ERROR)
		ErrorHandling("connect() error!");

	//PacketSender::instance().requestLogin(id, password);

	cl_packet_request_login login_packet;

	const char *myID = m_id.data( );
	const char *myPW = m_pw.data( );
	login_packet.type = CL_REQUEST_LOGIN;
	login_packet.size = sizeof(login_packet);
	strncpy( login_packet.id, myID, ID_LEN );
	strncpy( login_packet.password, myPW, ID_LEN );

	send(hSocket, reinterpret_cast<char*>(&login_packet), login_packet.size, 0);


	//응답패킷 받아오기
	recvn(hSocket, buff, sizeof(BYTE) + sizeof(BYTE), 0);
	//나머지 데이터 받아오기
	recvn(hSocket, buff + sizeof(BYTE) + sizeof(BYTE), buff[0] - (sizeof(BYTE) + sizeof(BYTE)), 0);

	lc_packet_permit_login permit_packet;
	memcpy(reinterpret_cast<char*>(&permit_packet), buff, *buff);

	closesocket(hSocket);
	WSACleanup();

	return permit_packet.permit_check;
}

CLoginScene::CLoginScene( )
{
	m_vPlayerStartPos = XMFLOAT3( 0.0f, 0.0f, 0.0f );
	m_editPos = XMFLOAT3( 0.0f, 0.0f, 0.0f );
	m_choosenEditBox = LOGIN_BUTTON;
	m_ptCursorPos.x = 0.0f;
	m_ptCursorPos.y = 0.0f;
}

CLoginScene::~CLoginScene( )
{
	if (m_ppEffectShaders)
		for (int i = 0; i < m_nEffects; i++)
		{
			if (m_ppEffectShaders[i]){
				m_ppEffectShaders[i]->ReleaseObject( );
				delete m_ppEffectShaders[i];
			}
		}
}

void CLoginScene::BuildObjects( ID3D11Device* pd3dDevice )
{
	LoadingSoundResource( );
	SoundManager::GetInstance( )->Play( SOUND_BGM );

	float screenHalfWidth = CAppManager::GetInstance( )->m_pFrameWork->m_nWndClientWidth / 2;
	float screenHalfHeight = CAppManager::GetInstance( )->m_pFrameWork->m_nWndClientHeight / 2;

	m_pBackgroundImage = new CTexture2DShader( pd3dDevice, _T("res/login/loginBackground.jpg"), screenHalfWidth, screenHalfHeight );

	float editBoxY = CAppManager::GetInstance( )->m_pFrameWork->m_nWndClientHeight / 3 - screenHalfHeight - 40;

	m_pIP = new CTexture2DShader( pd3dDevice, _T( "res/login/IP.png" ), 50, 25 );
	m_pIP->getObjects( )[0]->SetPosition( -250, editBoxY, -1 );
	m_pID = new CTexture2DShader( pd3dDevice, _T( "res/login/ID.png" ), 50, 25 );
	m_pID->getObjects( )[0]->SetPosition( -250, editBoxY-70, -1 );
	m_pPW = new CTexture2DShader( pd3dDevice, _T( "res/login/PW.png" ), 50, 25 );
	m_pPW->getObjects( )[0]->SetPosition( -250, editBoxY-140, -1 );
	m_pLoginButton = new CTexture2DShader( pd3dDevice, _T( "res/login/login.png" ), 50, 25 );
	m_pLoginButton->getObjects( )[0]->SetPosition( 150, editBoxY - 140, -1 );

	CTexture2DShader *temp = new CTexture2DShader( pd3dDevice, _T( "res/login/edit.png" ), 100, 25 );
	temp->getObjects( )[0]->SetPosition( -70, editBoxY, -1 );
	CTexture2DShader *temp2 = new CTexture2DShader( pd3dDevice, _T( "res/login/edit.png" ), 100, 25 );
	temp2->getObjects( )[0]->SetPosition( -70, editBoxY -70, -1 );
	CTexture2DShader *temp3 = new CTexture2DShader( pd3dDevice, _T( "res/login/edit.png" ), 100, 25 );
	temp3->getObjects( )[0]->SetPosition( -70, editBoxY-140, -1 );

	m_nEffects = 1 + 4 + 3; // 배경 1장, 버튼 4개, 텍스트공간 3개
	m_ppEffectShaders = new CShader*[m_nEffects];
	m_ppEffectShaders[0] = m_pBackgroundImage;
	m_ppEffectShaders[1] = m_pID;
	m_ppEffectShaders[2] = m_pPW;
	m_ppEffectShaders[3] = m_pIP;
	m_ppEffectShaders[4] = m_pLoginButton;
	m_ppEffectShaders[5] = temp;
	m_ppEffectShaders[6] = temp2;
	m_ppEffectShaders[7] = temp3;
}

void CLoginScene::Render( ID3D11DeviceContext *pd3dDeviceContext, CCamera *pCamera )
{
	if (m_pLights && m_pd3dcbLights)
		UpdateShaderVariable( pd3dDeviceContext, m_pLights );

	for (int i = 0; i < m_nShaders; i++)
	{
		m_ppShaders[i]->Render( pd3dDeviceContext, pCamera );
	}
	( (CTexture2DShader*)m_ppEffectShaders[0] )->MakeTransformToCamera( m_pCamera, XMFLOAT3( 0.0f, 0.0f, 0.0f ) );
	for (int i = 0; i < m_nEffects; i++)
	{
		m_ppEffectShaders[i]->Render( pd3dDeviceContext, pCamera );
	}

	if (m_ip.size( ) != 0)
		RenderText( EDIT_IP, m_ip );
	if (m_id.size( ) != 0)
		RenderText( EDIT_ID, m_id );
	if (m_pw.size( ) != 0)
	{
		std::string pw;
		for (int i = 0; i < m_pw.size( ); i++)
			pw += '*';
		RenderText( EDIT_PW, pw );
	}
}

void CLoginScene::RenderText( int textID, std::string text )
{
	XMFLOAT3 uiPosition = m_ppEffectShaders[textID]->getObjects( )[0]->GetPosition( );
	float uiWidth = ( (CTexture2D*)( m_ppEffectShaders[textID]->getObjects( )[0]->GetMesh( 0 ) ) )->m_x;
	float uiHeight = ( (CTexture2D*)( m_ppEffectShaders[textID]->getObjects( )[0]->GetMesh( 0 ) ) )->m_y;

	RECT renderRect;
	// RECT의 위치는 (0,0)의 위치가 Direct3D에 맞는 좌표계임 따라서 윈도우의 좌상단(0,0)에 맞게 해야함
	RECT rt = CAppManager::GetInstance( )->m_ClientRect;

	float ClientHeight = rt.bottom - rt.top;
	float ClientWidth = rt.right - rt.left;

	uiPosition.y *= -1;
	uiPosition.x += ClientWidth / 2;
	uiPosition.y += ClientHeight / 2;

	renderRect.left = uiPosition.x - uiWidth;
	renderRect.right = uiPosition.x + uiWidth;
	renderRect.top = uiPosition.y - uiHeight;
	renderRect.bottom = uiPosition.y + uiHeight;

	CAppManager::GetInstance( )->m_pFrameWork->RenderText( renderRect, D2D1::ColorF::White, text );
}

bool CLoginScene::SetInputData( WPARAM wParam )
{
	// 백스페이스가 눌리지 않았으면 모든 키값 검사해서 확인
	TCHAR key;
	static UCHAR pKeyBuffer[256];
	if (GetKeyboardState( pKeyBuffer ))
	{
		for (int i = 0; i < 256; i++)
		{
			if (pKeyBuffer[i] & 0xF0)
				key = i;
		}
		if (key == '\b')
			return false;
		if (key == 190)
			key = '.';

		switch (m_choosenEditBox)
		{
			case EDIT_IP:
				if (m_ip.size( ) < 16)
					m_ip += key;
				break;
			case EDIT_ID:
				if (m_id.size( ) < 10)
					m_id += key;
				break;
			case EDIT_PW:
				if (m_pw.size( ) < 12)
					m_pw += key;
				break;
		}
	}
	return false;
}

bool CLoginScene::SetCursorPosition( HWND hWnd )
{
	POINT ptCursorPos = m_ptCursorPos;

	if (GetCapture( ) == hWnd)
	{
		SetCursor( NULL );
		// 마우스 좌표는 좌상단이 0,0 임 따라서 가운데가 0,0이 되도록 변환해야함
		RECT rt;
		GetClientRect( hWnd, &rt );
		CAppManager::GetInstance( )->m_ClientRect = rt;

		float ClientHeight = rt.bottom - rt.top;
		float ClientWidth = rt.right - rt.left;

		ptCursorPos.x -= ClientWidth/2;
		ptCursorPos.y -= ClientHeight/2;
		ptCursorPos.y *= -1;
	}

	for (int i = 5; i < m_nEffects; i++)
	{
		XMFLOAT3 uiPosition = m_ppEffectShaders[i]->getObjects( )[0]->GetPosition( );
		float uiWidth = ( (CTexture2D*)( m_ppEffectShaders[i]->getObjects( )[0]->GetMesh( 0 ) ) )->m_x;
		float uiHeight = ( (CTexture2D*)( m_ppEffectShaders[i]->getObjects( )[0]->GetMesh( 0 ) ) )->m_y;

		if (ptCursorPos.x > uiPosition.x - uiWidth && ptCursorPos.x < uiPosition.x + uiWidth)
			if (ptCursorPos.y > uiPosition.y - uiHeight && ptCursorPos.y < uiPosition.y + uiWidth)
			{
				m_editPos = XMFLOAT3( uiPosition.x - uiWidth + 10, uiPosition.y, -5 );
				m_choosenEditBox = i;
				return true;
			}
	}

	return false;
}

bool CLoginScene::CatchLoginButton( HWND hWnd )
{
	POINT ptCursorPos = m_ptCursorPos;

	if (GetCapture( ) == hWnd)
	{
		SetCursor( NULL );
		RECT rt;
		GetClientRect( hWnd, &rt );

		float ClientHeight = rt.bottom - rt.top;
		float ClientWidth = rt.right - rt.left;
		ptCursorPos.x -= ClientWidth / 2;
		ptCursorPos.y -= ClientHeight / 2;
		ptCursorPos.y *= -1;
	}

	XMFLOAT3 uiPosition = m_ppEffectShaders[LOGIN_BUTTON]->getObjects( )[0]->GetPosition( );
	float uiWidth = ( (CTexture2D*)( m_ppEffectShaders[LOGIN_BUTTON]->getObjects( )[0]->GetMesh( 0 ) ) )->m_x;
	float uiHeight = ( (CTexture2D*)( m_ppEffectShaders[LOGIN_BUTTON]->getObjects( )[0]->GetMesh( 0 ) ) )->m_y;

	if (ptCursorPos.x > uiPosition.x - uiWidth && ptCursorPos.x < uiPosition.x + uiWidth)
		if (ptCursorPos.y > uiPosition.y - uiHeight && ptCursorPos.y < uiPosition.y + uiWidth)
		{
			if (LoginPermit()){
				CAppManager::GetInstance()->m_pFrameWork->ChangeScene(CAppManager::GetInstance()->m_pFrameWork->m_pd3dDevice, STAGE_VILLIGE);
			}
			else{
				ErrorHandling("로그인실패");
				return false;
			}

		}
	return false;
}

bool CLoginScene::ConnetByEnter( HWND hWnd )
{
	// m_ip, m_id, m_pw 의 값을 이용하여 서버에 전송하고 서버에 연결
	//if (LoginPermit( )){
		CAppManager::GetInstance( )->m_pFrameWork->ChangeScene( CAppManager::GetInstance( )->m_pFrameWork->m_pd3dDevice, STAGE_VILLIGE );
	//}
	//else{
	//	ErrorHandling( "로그인실패" );
	//	return false;
	//}
	return false;
}

bool CLoginScene::OnProcessingMouseMessage( HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam )
{
	switch (nMessageID)
	{
		case WM_LBUTTONDOWN:
			SetCapture( hWnd );
			m_ptCursorPos.x = LOWORD( lParam );
			m_ptCursorPos.y = HIWORD( lParam );
			CatchLoginButton( hWnd );
			SetCursorPosition( hWnd );
			break;
		case WM_RBUTTONDOWN:
			break;
		case WM_LBUTTONUP:
		case WM_RBUTTONUP:
			ReleaseCapture( );
			break;
		case WM_MOUSEMOVE:
			break;
		default:
			break;
	}

	return false;
}

bool CLoginScene::OnProcessingKeyboardMessage( HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam )
{
	switch (nMessageID)
	{
		case WM_KEYUP:
			switch (wParam)
			{
				case VK_ESCAPE:
					break;
				case VK_RETURN:	
					ConnetByEnter(hWnd);
					break;
				case VK_BACK:
					switch (m_choosenEditBox)
					{
						case EDIT_IP:
							if (m_ip.size( ) > 0)
								m_ip.erase( m_ip.end( ) - 1 );
							break;
						case EDIT_ID:
							if (m_id.size( ) > 0)
								m_id.erase( m_id.end( ) - 1 );
							break;
						case EDIT_PW:
							if (m_pw.size( ) > 0)
								m_pw.erase( m_pw.end( ) - 1 );
							break;
					}
					break;
				default:
					break;
			}	
			break;
		case WM_KEYDOWN:
		case WM_CHAR:
			SetInputData( wParam );
			break;

		default:
			break;
	}

	return false;
}

bool CLoginScene::ProcessInput( HWND hWnd, CGameTimer timer )
{
	return false;
}