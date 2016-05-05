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

// �ϳ��� ���� ����ü
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

// ������۴� ũ�Ⱑ �ݵ�� 16����Ʈ�� ������� �Ѵ�.
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
	// ���� ���̴����� ����Ʈ�̴�
	CShader **m_ppShaders;
	int m_nShaders;

	// �� �߰�
	LIGHTS *m_pLights;
	ID3D11Buffer *m_pd3dcbLights;

	int m_nFbxCount;
	float m_fogStart;
	float m_fogRange;
	D3DXCOLOR m_fogColor;

	// ���� �� ���꿡 �ʿ��� �͵�
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

	// ��ŷ
	bool Picking( int x, int y );	// ��ŷ�� ��ü�� ������ true ��ȯ
	// �浹üũ
	bool CollisionCheck( );
	bool CollisionCheck( CGameObject *pObj1, CGameObject *pObj2 );
	// ���� 
	virtual bool LoadingSoundResource( );

	// ���콺 ��ư Ŭ���� ���� ���콺 Ŀ�� ��ġ
	POINT m_ptOldCursorPos;
	// ��ŷ�� ������Ʈ�� ��ŷ��ġ
	CGameObject *pPickedObject;
	XMFLOAT3 vPickPos;
};

