#pragma once
#include "Timer.h"
#include "Player.h"
#include "Scene.h"
#include "Texture2D.h"
#include <D2D1.h>
#include <DWrite.h>

class CGameFramework
{
private:
	//direct3D
	IDXGISwapChain *m_pDXGISwapChain;
	ID3D11DeviceContext *m_pd3dDeviceContext;
	ID3D11RenderTargetView *m_pd3dRenderTargetView;

	// direct2D
	IDWriteFactory* m_pDWriteFactory;
	IDWriteTextFormat* m_pTextFormat;

	ID2D1Factory* m_pD2DFactory;
	ID2D1HwndRenderTarget* m_pRT;
	ID2D1RenderTarget* m_pBackBufferRT;
	
	ID2D1SolidColorBrush* m_pBrush;

	////////
	CGameTimer m_GameTimer;

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

	// ����̽�, ����ü��, ����̽� ���ؽ�Ʈ, ����̽��� ���õ� �並 �����ϴ� �Լ�
	bool CreateRenderTargetDepthStencilView( );
	bool CreateDirect3DDisplay( );

	// Direct2D ���� �Լ�
	bool CreateDirect2D( );

	// �������� �޽�, ��ü�� �����ϰ� �Ҹ��ϴ� �Լ�
	void BuildObjects( );
	void ReleaseObjects( );
	void MakePlayer( ID3D11Device* pd3dDevice );

	// �����ӿ�ũ�� �ٽ��� �����ϴ� �Լ�
	void ProcessInput( );
	void AnimateObjects( );
	void FrameAdvance( );
	void CheckCollision( );

	// ������ �޼��� ó�� �Լ�
	void OnProcessingMouseMessage( HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam );
	void OnProcessingKeyboardMessage( HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam );
	LRESULT CALLBACK OnProcessingWindowMessage( HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam );

	bool CreateSoundResources( );
	void ChangeScene( ID3D11Device* pd3dDevice, int iState );

	wchar_t* StringTowchar_t( std::string& s );
public:
	// for other class
	HINSTANCE m_hInstance;
	HWND m_hWnd;

	int m_nWndClientWidth;
	int m_nWndClientHeight;

	CPlayerShader *m_pPlayerShader;
	CTexture2DShader *m_pLoadingTexture;
	CScene *m_pScene;

	int m_iStageState;
	ID3D11Device *m_pd3dDevice;
	
	void RenderText( RECT renderRect, int color, std::string text );
};