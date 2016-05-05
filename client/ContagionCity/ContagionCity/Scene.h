#pragma once
#include <vector>
#include "Shader.h"
#include "FbxManager.h"
#include "Timer.h"

using namespace std;

#define MAX_LIGHTS		4
#define POINT_LIGHT		1.0f
#define SPOT_LIGHT		2.0f
#define DIRECTIONAL_LIGHT	3.0f

// 하나의 조명 구조체
struct LIGHT
{
	D3DXCOLOR m_cAmbient;
	D3DXCOLOR m_cDiffuse;
	D3DXCOLOR m_cSpecular;
	XMFLOAT3 m_vPosition;
	float m_fRange;
	XMFLOAT3 m_vDirection;
	float m_nType;
	XMFLOAT3 m_vAttenuation;
	float m_fFalloff;
	float m_fTheta;
	float m_fPi;
	float m_bEnable;
	float padding;
};

// 상수버퍼는 크기가 반드시 16바이트의 배수여야 한다.
struct LIGHTS
{	
	LIGHT m_pLights[MAX_LIGHTS];
	D3DXCOLOR m_cGlobalAmbient;
	XMFLOAT4 m_vCameraPosition;

	double m_FogStart;
	double m_FogRange;
	D3DXCOLOR m_FogColor;
};

class CScene
{
protected:
	// 씬은 셰이더들의 리스트이다
	CShader **m_ppShaders;
	int m_nShaders;

	// 빛 추가
	LIGHTS *m_pLights;
	ID3D11Buffer *m_pd3dcbLights;

	int m_nFbxCount;
	float m_fogStart;
	float m_fogRange;
	D3DXCOLOR m_fogColor;

	// 게임 내 연산에 필요한 것들
	CCamera *m_pCamera;
	CPlayer *m_pPlayer;

public:
	CScene( );
	~CScene( );

	virtual void CreateShaderVariables( ID3D11Device *pd3dDevice );
	virtual void UpdateShaderVariable( ID3D11DeviceContext *pd3dDeviceContext, LIGHTS *pLights );
	virtual void ReleaseShaderVariables( );

	virtual bool OnProcessingMouseMessage( HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam );
	virtual bool OnProcessingKeyboardMessage( HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam );

	virtual void BuildObjects( ID3D11Device *pd3dDevice );
	virtual void LoadFBXs( );
	virtual void ReleaseObjects( );

	virtual bool ProcessInput( HWND hWnd, CGameTimer timer );
	virtual void AnimateObjects( float fTimeElapsed );

	void Render( ID3D11DeviceContext *pd3dDeviceContext, CCamera *pCamera );

	void SetCamera( CCamera *pCamera ) { m_pCamera = pCamera; }
	void SetPlayer( CPlayer *pPlayer ) { m_pPlayer = pPlayer; }
	int getShaderCount( ){ return m_nShaders; }
	CShader** getShaders( ){ return m_ppShaders; }

	// 피킹
	bool Picking( int x, int y );	// 피킹된 물체가 있으면 true 반환
	// 충돌체크
	bool CollisionCheck( );
	bool CollisionCheck( CGameObject *pObj1, CGameObject *pObj2 );
	// 사운드 
	virtual bool LoadingSoundResource( );

	// 마우스 버튼 클릭할 때의 마우스 커서 위치
	POINT m_ptOldCursorPos;
	// 피킹된 오브젝트와 피킹위치
	CGameObject *pPickedObject;
	XMFLOAT3 vPickPos;
};

