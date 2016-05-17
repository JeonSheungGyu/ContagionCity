#pragma once
#include "Scene.h"
#include "Texture2D.h"

class CLoginScene : public CScene
{
	enum { LOGIN_BUTTON = 4, EDIT_IP, EDIT_ID, EDIT_PW };
	TCHAR* m_pFileName;
	CTexture2DShader *m_pBackgroundImage;
	CTexture2DShader *m_pLoginButton;
	CTexture2DShader *m_pPW;
	CTexture2DShader *m_pID;
	CTexture2DShader *m_pIP;
	
	XMFLOAT3 m_editPos;
	int m_choosenEditBox;
	POINT m_ptCursorPos;

	std::string m_ip;
	std::string m_id;
	std::string m_pw;

public:
	CLoginScene( );
	~CLoginScene( );

	virtual void BuildObjects( ID3D11Device* pd3dDevice );

	void RenderText( int textID, std::string text );
	bool SetInputData( WPARAM wParam );
	bool CatchLoginButton( HWND hWnd );
	bool SetCursorPosition( HWND hWnd );
	bool ConnetByEnter( HWND hWnd );

	virtual void Render( ID3D11DeviceContext *pd3dDeviceContext, CCamera *pCamera );
	virtual bool ProcessInput( HWND hWnd, CGameTimer timer );
	virtual bool OnProcessingMouseMessage( HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam );
	virtual bool OnProcessingKeyboardMessage( HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam );


	bool LoginPermit();
};

