#pragma once
#include <vector>
#include "Shader.h"
#include "FbxManager.h"

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
};

class CScene
{
private:
	// ���� ���̴����� ����Ʈ�̴�
	CShader **m_ppShaders;
	int m_nShaders;
	CCamera *m_pCamera;

	// �� �߰�
	LIGHTS *m_pLights;
	ID3D11Buffer *m_pd3dcbLights;

	int m_nFbxCount;
public:
	CScene( );
	~CScene( );

	void CreateShaderVariables( ID3D11Device *pd3dDevice );
	void UpdateShaderVariable( ID3D11DeviceContext *pd3dDeviceContext, LIGHTS *pLights );
	void ReleaseShaderVariables( );

	bool OnProcessingMouseMessage( HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam );
	bool OnProcessingKeyboardMessage( HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam );

	void BuildObjects( ID3D11Device *pd3dDevice );
	void LoadFBXs( );
	void ReleaseObjects( );

	bool ProcessInput( );
	void AnimateObjects( float fTimeElapsed );

	void Render( ID3D11DeviceContext *pd3dDeviceContext, CCamera *pCamera );

	void SetCamera( CCamera *pCamera ) { m_pCamera = pCamera; }
};

