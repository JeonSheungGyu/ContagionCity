#include "stdafx.h"
#include "GameFramework.h"

CGameFramework::CGameFramework( )
{
	m_pCamera = NULL;
	m_pd3dDevice = NULL;
	m_pDXGISwapChain = NULL;
	m_pd3dRenderTargetView = NULL;
	m_pd3dDeviceContext = NULL;

	m_nWndClientWidth = FRAME_BUFFER_WIDTH;
	m_nWndClientHeight = FRAME_BUFFER_HEIGHT;

	m_pScene = NULL;
	_tcscpy_s( m_pszBuffer, _T( "Contain City " ) );
	m_pPlayer = NULL;
	m_pd3dDepthStencilBuffer = NULL;
	m_pd3dDepthStencilView = NULL;
	m_pPlayerShader = NULL;
}


CGameFramework::~CGameFramework( )
{
}

bool CGameFramework::OnCreate( HINSTANCE hInstance, HWND hMainWnd )
{
	m_hInstance = hInstance;
	m_hWnd = hMainWnd;

	// Direct3D 디바이스, 디바이스 컨텍스트, 스왑 체인 등을 생성하는 함수를 호출한다.
	if (!CreateDirect3DDisplay( )) return false;

	// 렌더링할 객체를 생성
	BuildObjects( );

	return true;
}

bool CGameFramework::CreateRenderTargetDepthStencilView( )
{
	HRESULT hResult = S_OK;

	ID3D11Texture2D *pd3dBackBuffer;
	if (FAILED( hResult = m_pDXGISwapChain->GetBuffer( 0, __uuidof( ID3D11Texture2D ), (LPVOID *)&pd3dBackBuffer ) )) return false;
	if (FAILED( hResult = m_pd3dDevice->CreateRenderTargetView( pd3dBackBuffer, NULL, &m_pd3dRenderTargetView ) )) return false;
	if (pd3dBackBuffer) pd3dBackBuffer->Release( );

	// 렌더 타깃과 같은 크기의 깊이 버퍼를 생성
	D3D11_TEXTURE2D_DESC d3dDepthStencilBufferDesc;
	::ZeroMemory( &d3dDepthStencilBufferDesc, sizeof( D3D11_TEXTURE2D_DESC ) );
	d3dDepthStencilBufferDesc.Width = m_nWndClientWidth;
	d3dDepthStencilBufferDesc.Height = m_nWndClientHeight;
	d3dDepthStencilBufferDesc.MipLevels = 1;
	d3dDepthStencilBufferDesc.ArraySize = 1;
	d3dDepthStencilBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	d3dDepthStencilBufferDesc.SampleDesc.Count = 1;
	d3dDepthStencilBufferDesc.SampleDesc.Quality = 0;
	d3dDepthStencilBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	d3dDepthStencilBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	d3dDepthStencilBufferDesc.CPUAccessFlags = 0;
	d3dDepthStencilBufferDesc.MiscFlags = 0;
	if (FAILED( hResult = m_pd3dDevice->CreateTexture2D( &d3dDepthStencilBufferDesc, NULL, &m_pd3dDepthStencilBuffer ) )) return false;

	// 생성한 깊이 버퍼에 대한 뷰를 생성
	D3D11_DEPTH_STENCIL_VIEW_DESC d3dViewDesc;
	::ZeroMemory( &d3dViewDesc, sizeof( D3D11_DEPTH_STENCIL_VIEW_DESC ) );
	d3dViewDesc.Format = d3dDepthStencilBufferDesc.Format;
	d3dViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	d3dViewDesc.Texture2D.MipSlice = 0;
	if (FAILED( hResult = m_pd3dDevice->CreateDepthStencilView( m_pd3dDepthStencilBuffer, &d3dViewDesc, &m_pd3dDepthStencilView ) )) return false;

	m_pd3dDeviceContext->OMSetRenderTargets( 1, &m_pd3dRenderTargetView, m_pd3dDepthStencilView );

	return true;
}

//Direct3D 디바이스 생성하면서 스왑체인, 디바이스 컨텍스트, 렌더 타겟 뷰를 생성하고 설정한다.
bool CGameFramework::CreateDirect3DDisplay( )
{
	RECT rcClient;
	::GetClientRect( m_hWnd, &rcClient );
	m_nWndClientWidth = rcClient.right - rcClient.left;
	m_nWndClientHeight = rcClient.bottom - rcClient.top;

	UINT dwCreateDeviceFlags = 0;
#ifdef _DEBUG
	dwCreateDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	// 디바이스를 생성하기 위해 시도할 드라이버 순서
	D3D_DRIVER_TYPE d3dDriverTypes[ ] =
	{
		D3D_DRIVER_TYPE_HARDWARE,
		D3D_DRIVER_TYPE_WARP,
		D3D_DRIVER_TYPE_REFERENCE
	};
	UINT nDriverTypes = sizeof( d3dDriverTypes ) / sizeof( D3D10_DRIVER_TYPE );

	// 디바이스를 생성하기 위해 시도할 특성 레벨의 순서
	D3D_FEATURE_LEVEL d3dFeatureLevels[]= 
	{
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0
	};
	UINT nFeatureLevels = sizeof( d3dFeatureLevels ) / sizeof( D3D_FEATURE_LEVEL );

	// 생성할 스왑 체인을 서술하는 구조체
	DXGI_SWAP_CHAIN_DESC dxgiSwapChainDesc;
	::ZeroMemory( &dxgiSwapChainDesc, sizeof( dxgiSwapChainDesc ) );
	dxgiSwapChainDesc.BufferCount = 1;
	dxgiSwapChainDesc.BufferDesc.Width = m_nWndClientWidth;
	dxgiSwapChainDesc.BufferDesc.Height = m_nWndClientHeight;
	dxgiSwapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	dxgiSwapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
	dxgiSwapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	dxgiSwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	dxgiSwapChainDesc.OutputWindow = m_hWnd;
	dxgiSwapChainDesc.SampleDesc.Count = 1;
	dxgiSwapChainDesc.SampleDesc.Quality = 0;
	dxgiSwapChainDesc.Windowed = TRUE;

	D3D_DRIVER_TYPE nd3dDriverType = D3D_DRIVER_TYPE_NULL;
	D3D_FEATURE_LEVEL nd3dFeatureLevel = D3D_FEATURE_LEVEL_11_0;

	HRESULT hResult = S_OK;
	// 디바이스의 드라이버 유형과 특성 레벨을 지원하는 디바이스와 스왑체인을 생성
	// 고수준의 디바이스 생성을 시도하고 실패하면 하위 수준의 디바이스 생성을 시도
	for (UINT i = 0; i < nDriverTypes; i++)
	{
		nd3dDriverType = d3dDriverTypes[i];
		if (SUCCEEDED( hResult = D3D11CreateDeviceAndSwapChain( NULL, nd3dDriverType, NULL,
			dwCreateDeviceFlags, d3dFeatureLevels, nFeatureLevels, D3D11_SDK_VERSION,
			&dxgiSwapChainDesc, &m_pDXGISwapChain, &m_pd3dDevice, &nd3dFeatureLevel, &m_pd3dDeviceContext ) ))
			break;
	}
	if (!m_pDXGISwapChain || !m_pd3dDevice || !m_pd3dDeviceContext) return false;

	// 렌더 타깃 뷰를 생성하는 함수를 호출
	if (!CreateRenderTargetDepthStencilView( )) return false;

	return true;
}

void CGameFramework::OnProcessingMouseMessage( HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam )
{
	switch (nMessageID)
	{
		case WM_LBUTTONDOWN:
		case WM_RBUTTONDOWN:
			SetCapture( hWnd );
			GetCursorPos( &m_ptOldCursorPos );
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
}

void CGameFramework::OnProcessingKeyboardMessage( HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam )
{
	switch (nMessageID)
	{
		case WM_KEYUP:
			switch (wParam)
			{
				case VK_ESCAPE:
					break;
				default:
					break;
			}
			break;
		default:
			break;
	}
}

LRESULT CALLBACK CGameFramework::OnProcessingWindowMessage( HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam )
{
	switch (nMessageID)
	{
		case WM_SIZE:
		{
			m_nWndClientWidth = LOWORD( lParam );
			m_nWndClientHeight = HIWORD( lParam );

			m_pd3dDeviceContext->OMSetRenderTargets( 0, NULL, NULL );

			if (m_pd3dRenderTargetView) m_pd3dRenderTargetView->Release( );
			if (m_pd3dDepthStencilBuffer) m_pd3dDepthStencilBuffer->Release( );
			if (m_pd3dDepthStencilView) m_pd3dDepthStencilView->Release( );

			m_pDXGISwapChain->ResizeBuffers( 1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0 );

			CreateRenderTargetDepthStencilView( );
			CCamera *pCamera = m_pPlayer->GetCamera( );
			if (pCamera)
				pCamera->SetViewport( m_pd3dDeviceContext, 0, 0, m_nWndClientWidth, m_nWndClientHeight, 0.0f, 1.0f );
			break;
		}

		case WM_LBUTTONDOWN:
		case WM_RBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_RBUTTONUP:
		case WM_MOUSEMOVE:
			OnProcessingMouseMessage( hWnd, nMessageID, wParam, lParam );
			break;
		case WM_KEYDOWN:
		case WM_KEYUP:
			OnProcessingKeyboardMessage( hWnd, nMessageID, wParam, lParam );
			break;
	}

	return 0;
}

void CGameFramework::OnDestroy( )
{
	ReleaseObjects( );

	if (m_pd3dDeviceContext) m_pd3dDeviceContext->ClearState( );
	if (m_pd3dRenderTargetView)m_pd3dRenderTargetView->Release( );
	if (m_pd3dDepthStencilBuffer) m_pd3dDepthStencilBuffer->Release( );
	if (m_pd3dDepthStencilView) m_pd3dDepthStencilView->Release( );
	if (m_pDXGISwapChain) m_pDXGISwapChain->Release( );
	if (m_pd3dDeviceContext) m_pd3dDeviceContext->Release( );
	if (m_pd3dDevice) m_pd3dDevice->Release( );
}

void CGameFramework::BuildObjects( )
{
	CShader::CreateShaderVariables( m_pd3dDevice );
	CIlluminatedShader::CreateShaderVariables( m_pd3dDevice );

	m_pScene = new CScene( );
	m_pScene->BuildObjects( m_pd3dDevice );

	MakePlayer( m_pd3dDevice );

	m_pCamera = m_pPlayer->GetCamera( );
	m_pCamera->SetViewport( m_pd3dDeviceContext, 0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 0.0f, 1.0f );
	m_pCamera->GenerateViewMatrix( );

	m_pScene->SetCamera( m_pCamera );
}

void CGameFramework::MakePlayer( ID3D11Device* pd3dDevice )
{
	FBXManager::GetInstance( )->LoadFBX( "res/hero_low.FBX", LAYER_PLAYER, PLAYER_MAN );
	std::vector<CFbxMesh> tempMesh = FBXManager::GetInstance( )->m_pMeshes;
	FBXManager::GetInstance( )->ClearMeshes( );

	m_pPlayerShader = new CPlayerShader( );
	m_pPlayerShader->CreateShader( m_pd3dDevice );
	m_pPlayerShader->BuildObjects( m_pd3dDevice, tempMesh );
	m_pPlayer = m_pPlayerShader->GetPlayer( );
}

void CGameFramework::ReleaseObjects( )
{
	CShader::ReleaseShaderVariables( );
	CIlluminatedShader::ReleaseShaderVariables( );

	if (m_pScene) m_pScene->ReleaseObjects( );
	if (m_pScene) delete m_pScene;

	if (m_pPlayerShader) m_pPlayerShader->ReleaseObject( );
	if (m_pPlayerShader) delete m_pPlayerShader;
}

void CGameFramework::ProcessInput( )
{
	bool bProcessdByScene = false;
	if (m_pScene) bProcessdByScene = m_pScene->ProcessInput( );
	if (!bProcessdByScene)
	{
		static UCHAR pKeyBuffer[256];
		DWORD dwDirection = 0;
		if (GetKeyboardState( pKeyBuffer ))
		{
			if (pKeyBuffer[VK_UP] & 0xF0) 	dwDirection |= DIR_FORWARD;
			if (pKeyBuffer[VK_DOWN] & 0xF0) dwDirection |= DIR_BACKWARD;
			if (pKeyBuffer[VK_LEFT] & 0xF0) dwDirection |= DIR_LEFT;
			if (pKeyBuffer[VK_RIGHT] & 0xF0) dwDirection |= DIR_RIGHT;
		}
		float cxDelta = 0.0f, cyDelta = 0.0f;
		POINT ptCursorPos;
		/*마우스를 캡쳐했으면 마우스가 얼마만큼 이동하였는 가를 계산한다.
		마우스 왼쪽 또는 오른쪽 버튼이 눌러질 때의 메시지(WM_LBUTTONDOWN, WM_RBUTTONDOWN)를 처리할 때 마우스를 캡쳐하였다.
		그러므로 마우스가 캡쳐된 것은 마우스 버튼이 눌려진 상태를 의미한다.
		마우스를 좌우 또는 상하로 움직이면 플레이어를 x-축 또는 y-축으로 회전한다.*/
		if (GetCapture( ) == m_hWnd)
		{
			SetCursor( NULL );
			GetCursorPos( &ptCursorPos );
			cxDelta = (float)( ptCursorPos.x - m_ptOldCursorPos.x ) / 3.0f;
			cyDelta = (float)( ptCursorPos.y - m_ptOldCursorPos.y ) / 3.0f;
			SetCursorPos( m_ptOldCursorPos.x, m_ptOldCursorPos.y );
		}
		if (dwDirection != 0 || cxDelta != 0.0f || cyDelta != 0.0f)
		{
			if (cxDelta || cyDelta)
			{
				/*cxDelta는 y-축의 회전을 나타내고 cyDelta는 x-축의 회전을 나타낸다.
				오른쪽 마우스 버튼이 눌려진 경우 cxDelta는 z-축의 회전을 나타낸다.*/
				if (pKeyBuffer[VK_RBUTTON] & 0xF0)
					m_pPlayer->Rotate( cyDelta, 0.0f, -cxDelta );
				else
					m_pPlayer->Rotate( cyDelta, cxDelta, 0.0f );
			}
			/*플레이어를 dwDirection 방향으로 이동한다(실제로는 속도 벡터를 변경한다).
			이동 거리는 시간에 비례하도록 한다. 플레이어의 이동 속력은 (50/초)로 가정한다.
			만약 플레이어의 이동 속력이 있다면 그 값을 사용한다.*/
			if (dwDirection)
				m_pPlayer->Move( dwDirection, 50.0f * m_GameTimer.GetTimeElapsed( ), false );
		}
	}
	m_pPlayer->Update( m_GameTimer.GetTimeElapsed( ) );
}

void CGameFramework::AnimateObjects( )
{
	if (m_pScene)
		m_pScene->AnimateObjects( m_GameTimer.GetTimeElapsed( ) );
}

void CGameFramework::FrameAdvance( )
{
	m_GameTimer.Tick( );

	ProcessInput( );

	AnimateObjects( );

	float fClearColor[4] = { 0.0f, 0.125f, 0.3f, 1.0f };
	if (m_pd3dRenderTargetView) m_pd3dDeviceContext->ClearRenderTargetView( m_pd3dRenderTargetView, fClearColor );
	if (m_pd3dDepthStencilView) m_pd3dDeviceContext->ClearDepthStencilView( m_pd3dDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0 );

	if (m_pPlayer) m_pPlayer->UpdateShaderVariables( m_pd3dDeviceContext );

	CCamera *pCamera = ( m_pPlayer ) ? m_pPlayer->GetCamera( ) : NULL;
	if (m_pScene) m_pScene->Render( m_pd3dDeviceContext, pCamera );

	if (m_pPlayerShader) m_pPlayerShader->Render( m_pd3dDeviceContext, pCamera );

	m_pDXGISwapChain->Present( 0, 0 );

	m_GameTimer.GetFrameRate( m_pszBuffer + 13, 50 );
	::SetWindowText( m_hWnd, m_pszBuffer );
}