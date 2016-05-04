#pragma once
#include "Timer.h"
#include "Player.h"
#include "Scene.h"

class CGameFramework
{
private:
	HINSTANCE m_hInstance;
	HWND m_hWnd;

	int m_nWndClientWidth;
	int m_nWndClientHeight;

	ID3D11Device *m_pd3dDevice;
	IDXGISwapChain *m_pDXGISwapChain;
	ID3D11DeviceContext *m_pd3dDeviceContext;
	ID3D11RenderTargetView *m_pd3dRenderTargetView;

	CGameTimer m_GameTimer;

	CScene *m_pScene;
	CCamera *m_pCamera;

	_TCHAR m_pszBuffer[100];
	
	CPlayer *m_pPlayer;

	ID3D11Buffer *m_pd3dcbColor;

	ID3D11Texture2D *m_pd3dDepthStencilBuffer;
	ID3D11DepthStencilView *m_pd3dDepthStencilView;

public:
	CGameFramework( );
	~CGameFramework( );

	bool OnCreate( HINSTANCE hInstance, HWND hMainWnd );
	void OnDestroy( );
	
	// 디바이스, 스왑체인, 디바이스 컨텍스트, 디바이스와 관련된 뷰를 생성하는 함수
	bool CreateRenderTargetDepthStencilView( );
	bool CreateDirect3DDisplay( );

	// 렌더링할 메시, 객체를 생성하고 소멸하는 함수
	void BuildObjects( );
	void ReleaseObjects( );
	void MakePlayer( ID3D11Device* pd3dDevice );

	// 프레임워크의 핵심을 구성하는 함수
	void ProcessInput( );
	void AnimateObjects( );
	void FrameAdvance( );
	void CheckCollision( );

	// 윈도우 메세지 처리 함수
	void OnProcessingMouseMessage( HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam );
	void OnProcessingKeyboardMessage( HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam );
	LRESULT CALLBACK OnProcessingWindowMessage( HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam );

	bool CreateSoundResources( );
public:
	CPlayerShader *m_pPlayerShader;
};
