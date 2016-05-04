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
	
	// ����̽�, ����ü��, ����̽� ���ؽ�Ʈ, ����̽��� ���õ� �並 �����ϴ� �Լ�
	bool CreateRenderTargetDepthStencilView( );
	bool CreateDirect3DDisplay( );

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
public:
	CPlayerShader *m_pPlayerShader;
};
