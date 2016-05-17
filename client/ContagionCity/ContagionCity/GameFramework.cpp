#include "stdafx.h"
#include "GameFramework.h"
#include "Stage1Scene.h"
#include "VilligeScene.h"
#include "LoginScene.h"

CGameFramework::CGameFramework( )
{
	m_pCamera = NULL;
	m_pd3dDevice = NULL;
	m_pDXGISwapChain = NULL;
	m_pd3dRenderTargetView = NULL;
	m_pd3dDeviceContext = NULL;

	m_pDWriteFactory = NULL;
	m_pTextFormat = NULL;

	m_pD2DFactory = NULL;
	m_pRT = NULL;
	m_pBrush = NULL;

	m_nWndClientWidth = FRAME_BUFFER_WIDTH;
	m_nWndClientHeight = FRAME_BUFFER_HEIGHT;

	m_pScene = NULL;
	_tcscpy_s( m_pszBuffer, _T( "Contain City " ) );
	m_pPlayer = NULL;
	m_pd3dDepthStencilBuffer = NULL;
	m_pd3dDepthStencilView = NULL;
	m_pPlayerShader = NULL;
	m_iStageState = STAGE_LOGIN;
}


CGameFramework::~CGameFramework( )
{
}

bool CGameFramework::OnCreate( HINSTANCE hInstance, HWND hMainWnd )
{
	m_hInstance = hInstance;
	m_hWnd = hMainWnd;

	// FMOD 사운드 시스템 초기화 및 로딩
	CreateSoundResources( );

	// Direct3D 디바이스, 디바이스 컨텍스트, 스왑 체인 등을 생성하는 함수를 호출한다.
	if (!CreateDirect3DDisplay( )) return false;

	// DirectInput8 객체 생성
	RECT rt;
	GetClientRect( hMainWnd, &rt);
	DirectInput::GetInstance( )->Init( hInstance, hMainWnd, rt.left - rt.right, rt.bottom - rt.top );

	CAppManager::GetInstance( )->m_pFrameWork = this;

	// 렌더링할 객체를 생성
	BuildObjects( );

	return true;
}

bool CGameFramework::CreateSoundResources( )
{
	SoundManager::GetInstance( )->Init( );

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
	dwCreateDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG ;
#endif
	dwCreateDeviceFlags |= D3D11_CREATE_DEVICE_BGRA_SUPPORT;

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
	dxgiSwapChainDesc.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
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

	if (!CreateDirect2D( )) return false;
	return true;
}

bool CGameFramework::CreateDirect2D( )
{
	// d2dFactory 생성
	HRESULT hr = D2D1CreateFactory( D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_pD2DFactory );

	// IDWriteFactory 생성
	if (SUCCEEDED( hr ))
	{
		hr = DWriteCreateFactory( DWRITE_FACTORY_TYPE_SHARED, __uuidof( IDWriteFactory ), reinterpret_cast<IUnknown**>( &m_pDWriteFactory ) );
	}

	// 글자 포맷 생성
	if (SUCCEEDED( hr ))
	{
		hr = m_pDWriteFactory->CreateTextFormat(
			L"돌림",                // Font family name.
			NULL,                       // Font collection (NULL sets it to use the system font collection).
			DWRITE_FONT_WEIGHT_REGULAR,
			DWRITE_FONT_STYLE_NORMAL,
			DWRITE_FONT_STRETCH_NORMAL,
			15.0f,
			L"ko-KR",
			&m_pTextFormat
			);
	}
	if (SUCCEEDED( hr ))
	{
		FLOAT dpiX;
		FLOAT dpiY;
		m_pD2DFactory->GetDesktopDpi( &dpiX, &dpiY );

		D2D1_RENDER_TARGET_PROPERTIES props =
			D2D1::RenderTargetProperties(
			D2D1_RENDER_TARGET_TYPE_DEFAULT,
			D2D1::PixelFormat( DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED ),
			dpiX,
			dpiY
			);

		IDXGISurface *pBackBuffer;
		m_pDXGISwapChain->GetBuffer( 0, IID_PPV_ARGS( &pBackBuffer ) );
		// 렌더타깃 생성
		hr = m_pD2DFactory->CreateDxgiSurfaceRenderTarget(
			pBackBuffer,
			(const D2D1_RENDER_TARGET_PROPERTIES *)&props,
			&m_pBackBufferRT
			);
		if (SUCCEEDED( hr ))
		{
			m_pBackBufferRT->CreateSolidColorBrush( D2D1::ColorF( D2D1::ColorF::White ), &m_pBrush );
		}
	}
	if (SUCCEEDED( hr ))
		return true;
	else
		return false;
}

void CGameFramework::RenderText( RECT renderRect, int color, std::string text )
{
	D2D1_SIZE_F targetSize = m_pBackBufferRT->GetSize( );

	m_pBackBufferRT->BeginDraw( );

	m_pBrush->SetTransform(
		D2D1::Matrix3x2F::Scale( targetSize )
		);

	m_pBrush->SetColor( D2D1::ColorF( color ) );

	D2D1_RECT_F rect = D2D1::RectF(
		renderRect.left,
		renderRect.top,
		renderRect.right,
		renderRect.bottom
		);

	wchar_t* szText = StringTowchar_t( text );

	m_pBackBufferRT->DrawTextW(
		szText,
		wcslen( szText ),
		m_pTextFormat,
		rect,
		m_pBrush );

	m_pBackBufferRT->EndDraw( );
}

wchar_t* CGameFramework::StringTowchar_t( std::string& s )
{
	const char* all = s.c_str( );
	int len = 1 + strlen( all );
	wchar_t* t = new wchar_t[len];
	if (NULL == t) throw std::bad_alloc( );
	mbstowcs( t, all, len );
	return t;
}

void CGameFramework::OnProcessingMouseMessage( HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam )
{
	m_pScene->OnProcessingMouseMessage( hWnd, nMessageID, wParam, lParam );
}

void CGameFramework::OnProcessingKeyboardMessage( HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam )
{
	m_pScene->OnProcessingKeyboardMessage( hWnd, nMessageID, wParam, lParam );
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
	m_pLoadingTexture = new CTexture2DShader( m_pd3dDevice, _T( "res/Loading.jpg" ), m_nWndClientWidth/2,m_nWndClientHeight/2 );

	m_pScene = new CLoginScene( );
	m_pScene->BuildObjects( m_pd3dDevice );

	MakePlayer( m_pd3dDevice );

	m_pCamera = m_pPlayer->GetCamera( );
	m_pCamera->SetViewport( m_pd3dDeviceContext, 0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 0.0f, 1.0f );
	m_pCamera->GenerateViewMatrix( );
	m_pCamera->SetOffset( XMFLOAT3(0.0f, 0.0f, -10.0f) );

	m_pScene->SetCamera( m_pCamera );
	m_pScene->SetPlayer( m_pPlayer );
}

void CGameFramework::MakePlayer( ID3D11Device* pd3dDevice )
{
	FBXManager::GetInstance( )->LoadFBX( "res/hero/Hero_sword_animation2.FBX", LAYER_PLAYER, PLAYER_MAN, 2
		, _T( "res/hero/hero_texture.dds" ), _T( "res/hero/hero_NormalsMap.dds" ) );
//	FBXManager::GetInstance( )->LoadFBX( "res/hero/Sword.FBX", LAYER_PLAYER, PLAYER_WEAPON , 1 , _T( "res/hero/sword_texture.dds" ) );

	//FBXManager::GetInstance( )->LoadFBX( "res/hero/Girl_Npc_Animation_mod.FBX", LAYER_PLAYER, PLAYER_MAN, 2
	//	, _T( "res/hero/hero_texture.dds" ), _T("res/hero/hero_NormalsMap.dds"));

	std::vector<CFbxMesh> tempMesh = FBXManager::GetInstance( )->m_pMeshes;
	FBXManager::GetInstance( )->ClearMeshes( );

	m_pPlayerShader = new CPlayerShader( );
	m_pPlayerShader->CreateShader( m_pd3dDevice );
	m_pPlayerShader->BuildObjects( m_pd3dDevice, tempMesh );
	m_pPlayer = m_pPlayerShader->GetPlayer( );
	m_pPlayer->SetPosition( m_pScene->getStartPos( ) );
}

void CGameFramework::ReleaseObjects( )
{
	CShader::ReleaseShaderVariables( );
	CIlluminatedShader::ReleaseShaderVariables( );

	if (m_pScene) m_pScene->ReleaseObjects( );
	if (m_pScene) delete m_pScene;

	if (m_pPlayerShader) m_pPlayerShader->ReleaseObject( );
	if (m_pPlayerShader) delete m_pPlayerShader;

	if (m_pBrush)
		m_pBrush->Release( );
	if (m_pTextFormat)
		m_pTextFormat->Release( );
	if (m_pD2DFactory)
		m_pD2DFactory->Release( );
	if (m_pDWriteFactory)
		m_pDWriteFactory->Release( );
}

void CGameFramework::ProcessInput( )
{
	if (m_pScene)
		m_pScene->ProcessInput( m_hWnd, m_GameTimer );
}

void CGameFramework::AnimateObjects( )
{
	if (m_pScene)
		m_pScene->AnimateObjects( m_GameTimer.GetTimeElapsed( ) );

	if (m_iStageState != STAGE_LOGIN)
		if (m_pPlayer)
			m_pPlayer->Animate( m_GameTimer.GetTimeElapsed( ) );
}

void CGameFramework::FrameAdvance( )
{
	// 시간변화
	m_GameTimer.Tick( );
	// 입력 체크
	DirectInput::GetInstance( )->Frame( );

	// 입력 처리
	ProcessInput( );

	// 씬변화
	if (m_pScene->iChangeScene != -1) ChangeScene( m_pd3dDevice, m_pScene->iChangeScene );

	// 객체 애니메이트
	AnimateObjects( );

	// 후면 버퍼 초기화
	float fClearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
	if (m_pd3dRenderTargetView) m_pd3dDeviceContext->ClearRenderTargetView( m_pd3dRenderTargetView, fClearColor );
	if (m_pd3dDepthStencilView) m_pd3dDeviceContext->ClearDepthStencilView( m_pd3dDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0 );

	// 렌더링을 위한 준비
	if (m_pPlayer) m_pPlayer->UpdateShaderVariables( m_pd3dDeviceContext );

	CCamera *pCamera = ( m_pPlayer ) ? m_pPlayer->GetCamera( ) : NULL;

	// 씬 그리기
	if (m_pScene) 
		if(m_iStageState == STAGE_LOGIN)
			((CLoginScene*)m_pScene)->Render( m_pd3dDeviceContext, pCamera );
		else
			m_pScene->Render( m_pd3dDeviceContext, pCamera );

	// 플레이어 그리기
	if (m_iStageState != STAGE_LOGIN)
		if (m_pPlayerShader) m_pPlayerShader->Render( m_pd3dDeviceContext, pCamera );

	// 플리핑
	m_pDXGISwapChain->Present( 0, 0 );

	// 프레임 저장 후 출력
	m_GameTimer.GetFrameRate( m_pszBuffer + 13, 50 );
	::SetWindowText( m_hWnd, m_pszBuffer );
}

void CGameFramework::ChangeScene( ID3D11Device* pd3dDevice, int iState )
{
	// 후면 버퍼 초기화
	float fClearColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	if (m_pd3dRenderTargetView) m_pd3dDeviceContext->ClearRenderTargetView( m_pd3dRenderTargetView, fClearColor );
	if (m_pd3dDepthStencilView) m_pd3dDeviceContext->ClearDepthStencilView( m_pd3dDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0 );

	// 카메라 변화 및 로딩텍스처 그리기
	m_iStageState = STAGE_LOGIN;
	m_pCamera->SetOffset( XMFLOAT3( 0.0f, 0.0f, -1.0f ) );
	if (m_pPlayer) m_pPlayer->UpdateShaderVariables( m_pd3dDeviceContext );

	XMFLOAT3 LoadingTexturePos = MathHelper::GetInstance( )->Float3MulFloat( m_pCamera->GetLookVector( ), 5 );
	m_pLoadingTexture->MakeTransformToCamera( m_pCamera, m_pPlayer->GetPosition() );
	m_pLoadingTexture->Render( m_pd3dDeviceContext, m_pCamera );
	
	// 플리핑
	m_pDXGISwapChain->Present( 0, 0 );

	// 새로운 씬 생성 및 초기화
	CScene *pScene = NULL;

	switch (iState)
	{
		case STAGE_LOGIN:
			pScene = new CLoginScene( );
			m_iStageState = STAGE_LOGIN;
			m_pCamera->SetOffset( XMFLOAT3( 0.0f, 0.0f, -1.0f ) );
			break;
		case STAGE_VILLIGE:
			pScene = new VilligeScene( );
			m_iStageState = STAGE_VILLIGE;
			m_pCamera->SetOffset( XMFLOAT3( 0.0f, 200.0f, -510.0f ) );
			break;
		case STAGE_1:
			pScene = new CStage1Scene( );
			m_iStageState = STAGE_1;
			m_pCamera->SetOffset( XMFLOAT3( 0.0f, 200.0f, -510.0f ) );
			break;
	}

	pScene->BuildObjects( pd3dDevice );
	pScene->SetPlayer( m_pPlayer );
	pScene->SetCamera( m_pCamera );

	// 기존 씬의 객체들 제거
	if (m_pScene)
		m_pScene->ReleaseObjects( );
	delete m_pScene;

	// 씬 변화
	m_pScene = pScene;
	// 플레이어 위치를 해당 씬에서의 초기 위치로 세팅
	XMFLOAT3 startPos = m_pScene->getStartPos( );
	m_pPlayer->SetPosition( startPos );
}